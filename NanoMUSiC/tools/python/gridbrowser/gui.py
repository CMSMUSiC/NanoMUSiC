import os

from PyQt4.QtCore import pyqtSlot
from PyQt4.QtGui import QApplication, QTreeView, QMainWindow, QMenu, QAction, \
                        QFileDialog, QInputDialog, QLineEdit, QMessageBox

from .model import Item, Model

class TreeView(QTreeView):
    def __init__(self, model, *args, **kwargs):
        QTreeView.__init__(self, *args, **kwargs)

        self.setModel(model)

        self.collapsed.connect(self.fit_columns)
        self.expanded.connect(self.fit_columns)
        self.model().rowsInserted.connect(self.fit_columns)
        self.model().rowsRemoved.connect(self.fit_columns)
        self.model().dataChanged.connect(self.fit_columns)

    def contextMenuEvent(self, event):
        menu = QMenu(self)

        refresh_action = QAction("&Refresh", self)
        refresh_action.triggered.connect(self.refresh)
        menu.addAction(refresh_action)

        menu.addSeparator()

        fetch_action = QAction("&Fetch", self)
        fetch_action.triggered.connect(self.fetch_selected)
        menu.addAction(fetch_action)

        copy_action = QAction("&Copy path", self)
        copy_action.setShortcut("Ctrl+C")
        copy_action.triggered.connect(self.copy_selected)
        menu.addAction(copy_action)

        menu.addSeparator()

        rename_action = QAction("&Rename...", self)
        rename_action.triggered.connect(self.rename_selected)
        menu.addAction(rename_action)

        delete_action = QAction("Delete...", self)
        delete_action.triggered.connect(self.delete_selected)
        menu.addAction(delete_action)

        menu.addSeparator()
        properties_action = QAction("Properties", self)
        properties_action.triggered.connect(self.show_properties)
        menu.addAction(properties_action)

        menu.exec_(event.globalPos())

    def firstSelectedIndex(self):
        return self.selectedIndexes()[0]

    def copy_selected(self):
        index = self.firstSelectedIndex()
        if index.isValid():
            item = index.internalPointer()

        if item:
            clipboard = QApplication.clipboard()
            clipboard.setText(item.path)

    def delete_selected(self):
        index = self.firstSelectedIndex()

        item = self.model().item_from_index(index)

        button = QMessageBox.warning(
            self,
            "Delete File",
            "Do you really want to delete '%s'?" % item.path,
            QMessageBox.Yes | QMessageBox.No)

        if button == QMessageBox.Yes:
            self.model().delete(index)

    def fetch_selected(self):
        index = self.firstSelectedIndex()

        item = self.model().item_from_index(index)

        suggestion = os.path.join(".", item.basename())

        local_path = QFileDialog.getSaveFileName(self, "Fetch File", suggestion)
        if local_path:
            self.model().fetch(index, local_path)

    def rename_selected(self):
        index = self.firstSelectedIndex()

        item = self.model().item_from_index(index)

        new_name, success = QInputDialog.getText(
            self,
            "Rename File",
            "Enter new name:",
            QLineEdit.Normal,
            item.basename())

        if success and new_name != item.basename():
            self.model().rename(index, new_name)

    def show_properties(self):
        index = self.firstSelectedIndex()
        properties = self.model().properties(index)
        text = "\n".join("%s = %s" % (k,v) for k,v in properties.items())
        QMessageBox.information(self, "Properties", text)

    def refresh(self):
        self.model().refresh()

    def fit_columns(self, index=None):
        N = 3
        for i in xrange(N):
            self.resizeColumnToContents(i)


class GridBrowser(QMainWindow):
    def __init__(self, storage_element, root_path="/", *args, **kwargs):
        super(GridBrowser, self).__init__(*args, **kwargs)

        self.storage_element = storage_element

        self.model = Model(self.storage_element, root_path=root_path)

        self.model.modelBusy.connect(self.onBusy)
        self.model.modelReady.connect(self.onReady)
        self.model.progressUpdate.connect(self.onProgressUpdate)

        self.setWindowTitle("GridBrowser: %s: %s" % (self.storage_element.site, root_path))

        self._tree = TreeView(self.model)
        self.setCentralWidget(self._tree)

    @pyqtSlot(int, int)
    def onProgressUpdate(self, current, max):
        status = "Loading item %d/%d..." % (current+1, max)
        self.set_status(True, status)

    @pyqtSlot()
    def onBusy(self):
        self.set_status(True, "Working...")

    @pyqtSlot()
    def onReady(self):
        self.set_status(False, "Ready.")

    def set_status(self, is_busy, status):
        self.statusBar().showMessage(status)