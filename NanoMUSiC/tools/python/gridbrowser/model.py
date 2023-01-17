import os
import stat
import datetime

from PyQt4.QtCore import QAbstractItemModel, pyqtSignal, pyqtSlot, QObject, \
                         QRunnable, QMutex, QModelIndex, QThreadPool, Qt, \
                         QVariant

from PyQt4.QtGui import QIcon

from .util import natsort_key, format_file_size

class _WorkerSignals(QObject):
    started = pyqtSignal()
    finished = pyqtSignal()
    progressUpdate = pyqtSignal(int, int)

    beginInsertRows = pyqtSignal(object, int, int)
    endInsertRows = pyqtSignal()
    dataChanged = pyqtSignal(object, object)


class PopulateModelWorker(QRunnable):
    def __init__(self, item, storage_element, index=None):
        QRunnable.__init__(self)

        self.signals = _WorkerSignals()

        self.item = item
        self.storage_element = storage_element
        self.index = index

    def run(self):
        success = self.item.mutex.tryLock()
        if not success:
            return

        self.signals.started.emit()
        self.item.clear_children()

        children = self.storage_element.ls(self.item.path, stat=True)
        children.sort(key=lambda tuple: natsort_key(tuple.name))

        self.signals.beginInsertRows.emit(self.index, 0, len(children)-1)

        for i, child in enumerate(children):
            child_path = os.path.join(self.item.path, child.name)
            child_item = Item(path=child_path, parent=self.item, stat=child.stat)
            self.item.append_child(child_item)
            self.signals.progressUpdate.emit(i, len(children))

        self.signals.endInsertRows.emit()

        self.item.mutex.unlock()
        self.signals.finished.emit()


class FetchItemWorker(QRunnable):
    def __init__(self, item, storage_element, destination):
        QRunnable.__init__(self)

        self.signals = _WorkerSignals()

        self.item = item
        self.storage_element = storage_element
        self.destination = destination


    def run(self):
        self.signals.started.emit()
        self.item.fetch(self.destination, self.storage_element)
        self.signals.finished.emit()



class Item:
    COLUMN_NAMES = [
        "Name",
        "Size",
        "Type",
        "Date",
    ]

    def __init__(self, path, stat=None, parent=None):
        self.parent = parent
        self.path = path
        self._children = None
        self._stat = stat
        self.mutex = QMutex()

        self.column_values = [
            self.basename(),
            format_file_size(self.size()) if not self.isdir() else "",
            self.typename(),
            self.modification_time().__format__("%d.%m.%Y %H:%M:%S"),
        ]

    def properties(self, storage_element):
        return storage_element.xattrs(self.path)

    def size(self):
        return self._stat.st_size

    def modification_time(self):
        return datetime.datetime.fromtimestamp(self._stat.st_mtime)

    def basename(self):
        return os.path.basename(self.path)

    def isdir(self):
        return stat.S_ISDIR(self._stat.st_mode)

    def delete(self, storage_element):
        self.parent.remove_child(self)
        self.parent = None
        storage_element.rm(self.path)

    def rename(self, new_name, storage_element):
        new_name = str(new_name)

        dirname = os.path.dirname(self.path)
        new_path = os.path.join(dirname, new_name)
        storage_element.mv(self.path, new_path)
        self.path = new_path

    def fetch(self, destination, storage_element):
        destination = str(destination)

        storage_element.fetch(self.path, destination)

    def children(self):
        return self._children

    def clear_children(self):
        self._children = []

    def append_child(self, item):
        self._children.append(item)

    def remove_child(self, item):
        self._children.remove(item)

    def child_at(self, i):
        return self._children[i] if i < len(self._children) else None

    def typename(self):
        mode = self._stat.st_mode
        if stat.S_ISDIR(mode):
            return "directory"
        elif stat.S_ISREG(mode):
            return "regular file"
        elif stat.S_ISLNK(mode):
            return "symbolic link"
        elif stat.S_ISSOCK(mode):
            return "socket"
        else:
            return ""

    def tree_loaded(self):
        return self._children is not None

    def tree_parent(self):
        return self.parent

    def tree_icon(self):
        if self.isdir():
            name = "folder"
        else:
            name = "application-x-executable"

        icon = QIcon.fromTheme(name)
        return QVariant(icon)


    def tree_text(self, column):
        return self.column_values[column]

    def tree_column_count(self):
        return min( len(self.COLUMN_NAMES), len(self.column_values) )

    def tree_row_count(self):
        return len(self._children) if self._children is not None else 0

    def tree_row(self):
        if self.parent:
            return self.parent._children.index(self)
        else:
            return None



class Model(QAbstractItemModel):
    modelBusy = pyqtSignal()
    modelReady = pyqtSignal()

    progressUpdate = pyqtSignal(int, int)

    def __init__(self, storage_element, root_path, multithreaded=True):
        super(Model, self).__init__()

        self.multithreaded = multithreaded
        self.storage_element = storage_element

        self.root_path = root_path

        self.root = None
        self.refresh()


    def columnCount(self, index):
        item = self.item_from_index(index)
        return item.tree_column_count()

    def rowCount(self, index):
        item = self.item_from_index(index)
        return item.tree_row_count()

    def data(self, index, role):
        if not index.isValid():
            return None

        item = index.internalPointer()

        if role == Qt.DisplayRole:
            return item.tree_text(index.column())
        elif role == Qt.DecorationRole and index.column() == 0:
            return item.tree_icon()


    def headerData(self, section, orientation, role):
        if orientation == Qt.Horizontal and role == Qt.DisplayRole:
            return Item.COLUMN_NAMES[section]

    def index(self, row, column, parent):
        if not self.hasIndex(row, column, parent):
            return QModelIndex()

        item = self.item_from_index(parent)
        child = item.child_at(row)
        if child:
            return self.createIndex(row, column, child)

    def parent(self, index):
        if not index.isValid():
            return None

        child = index.internalPointer()
        if not child:
            return None

        parent = child.tree_parent()
        return self.index_from_item(parent)

    def hasChildren(self, index):
        item = self.item_from_index(index)
        return item.isdir()

    def canFetchMore(self, index):
        item = self.item_from_index(index)
        return item.isdir() and not item.tree_loaded()

    def fetchMore(self, index):
        self.populate_item(index)

    def refresh(self):
        self.modelBusy.emit()

        self.beginResetModel()
        root_stat = self.storage_element.stat(self.root_path)
        self.root = Item(path=self.root_path, stat=root_stat)
        self.populate_item(self.index_from_item(self.root))
        self.endResetModel()

        self.modelReady.emit()

    def populate_item(self, index):
        item = self.item_from_index(index)

        if not item.isdir():
            return

        worker = PopulateModelWorker(item=item, index=index, storage_element=self.storage_element)

        worker.signals.beginInsertRows.connect(self.beginInsertRows)
        worker.signals.endInsertRows.connect(self.endInsertRows)

        self._run_worker(worker)

    def delete(self, index):
        item = self.item_from_index(index)
        row = item.tree_row()

        self.modelBusy.emit()
        self.beginRemoveRows(self.index_from_item(item.parent), row, row)
        item.delete(storage_element=self.storage_element)
        self.endRemoveRows()
        self.modelReady.emit()

    def rename(self, index, new_name):
        item = self.item_from_index(index)
        self.modelBusy.emit()
        item.rename(new_name, storage_element=self.storage_element)
        self.dataChanged.emit(index, index)
        self.modelReady.emit()

    def fetch(self, index, destination):
        item = self.item_from_index(index)

        worker = FetchItemWorker(item=item, storage_element=self.storage_element, destination=destination)
        worker.signals.started.connect(self.modelBusy)
        worker.signals.finished.connect(self.modelReady)
        self._run_worker(worker)

    def properties(self, index):
        item = self.item_from_index(index)

        props = item.properties(storage_element=self.storage_element)
        return props

    def item_from_index(self, index):
        return index.internalPointer() if index.isValid() else self.root

    def index_from_item(self, item):
        assert item is not None
        if item == self.root:
            return QModelIndex()
        return self.createIndex(item.tree_row(), 0, item)

    def _run_worker(self, worker):
        worker.signals.progressUpdate.connect(self.progressUpdate)
        worker.signals.started.connect(self.modelBusy)
        worker.signals.finished.connect(self.modelReady)

        if self.multithreaded:
            QThreadPool.globalInstance().start(worker);
        else:
            worker.run()


class DirectoryModelProxy(QAbstractItemModel):
    def __init__(self, model, *args, **kwargs):
        super(DirectoryModelProxy, self).__init__(*args, **kwargs)

        self.model = model

    def columnCount(self, index):
        return 1

    def rowCount(self, index):
        return self.model.rowCount(index)

    def data(self, index, role):
        if not index.isValid():
            return None

        if role == Qt.DisplayRole and index.column() == 0:
            return self.model.data(index, role)
        elif role == Qt.DecorationRole and index.column() == 0:
            return self.model.data(index, role)

    def headerData(self, section, orientation, role):
        return None

    def index(self, row, column, parent):
        return self.model.index(row, column, parent)

    def parent(self, index):
        return self.model.parent(index)

    def hasChildren(self, index):
        return self.model.hasChildren(index)

    def canFetchMore(self, index):
        return self.model.canFetchMore(index)

    def fetchMore(self, index):
        return self.model.fetchMore(index)
