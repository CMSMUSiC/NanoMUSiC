from __future__ import division
import curses
import time
import math
import logging
import logging.handlers
import multiprocessing
import threading
import sys
import traceback
import Queue
import re

logger = logging.getLogger(__name__)
logger.addHandler(logging.NullHandler())

def test():
    logger.warning("test")
def default(x, y):
    if x is not None: return x
    return y
def chunks(l, n):
    """ Yield successive n-sized chunks from l.
    """
    for i in xrange(0, len(l), n):
        yield l[i:i+n]

def bicolor(screen, row, col, splitter, color0, color1, text):
    items = re.split(splitter, text)
    colors = [color0, color1]
    currentcol = col
    for item, i in zip(items,range(len(items))):
        screen.addstr(row, currentcol, item, colors[i%2])
        currentcol+=len(item)


class BaseElement:
    def refresh(self):
        pass
    def goLeft(self):
        pass
    def goRight(self):
        pass
    def goUp(self):
        pass
    def goDown(self):
        pass
    def pageUp(self):
        pass
    def pageDown(self):
        pass
    def home(self):
        pass
    def end(self):
        pass

class TabbedText(BaseElement):
    def __init__(self, screen, maxrows=20000, top=0, left=0, height=None, width=None):
        self.parent = screen
        self.top, self.left = top, left
        self._height = default(height, screen.getmaxyx()[0]-top)
        self._width = default(width, screen.getmaxyx()[1])
        self.heading = curses.newwin(2, self.width, top, left)
        self.pads = []
        self.positions = []
        self.text = []
        self.activeCard = 0
        self._nrows = []
        self.maxrows = maxrows
    def clear(self):
        self.text = []
        self.pads = []
        self._redraw()
    def addFile(self, title, filename):
        f = open(filename, 'r')
        self.addText(title, f.read())
        f.close()
    def _countLines(self, text):
        lines=0
        for line in text.splitlines():
            lines += 1+(len(line ) // self.width)
        return lines
    def addText(self, title, text):
        self.text.append( (title, text) )
        newpad = curses.newpad(min(self.maxrows, max(self.height, self._countLines(text))), self.width)
        self.pads.append(newpad)
        self.positions.append(0)
        self._redraw()
    def refresh(self):
        self._redrawHeading()
        self.heading.refresh()
        self.pads[self.activeCard].noutrefresh(self.positions[self.activeCard], 0, self.top + 2, self.left, self.top + 2 + self.height - 1, self.left + self.width - 1)
    def goLeft(self):
        self.activeCard = (self.activeCard - 1) % len(self.text)
    def goRight(self):
        self.activeCard = (self.activeCard + 1) % len(self.text)
    def goUp(self):
        self.positions[self.activeCard]=max(0, self.positions[self.activeCard] - 1)
    def goDown(self):
        self.positions[self.activeCard]=min(self.positions[self.activeCard] + 1, self.nrows - self.height)
    def pageUp(self):
        self.positions[self.activeCard]=max(self.positions[self.activeCard] - self.height, 0)
    def pageDown(self):
        self.positions[self.activeCard]=min(self.positions[self.activeCard] + self.height, self.nrows - self.height)
    def home(self):
        self.positions[self.activeCard] = 0
    def end(self):
        self.positions[self.activeCard] = self.nrows-self.height
    def _redrawHeading(self):
        self.heading.clear()
        ncards = len(self.text)
        cardSpacing = 1
        cardWidth = (self.width-1) // ncards - 2 * (cardSpacing + 1)
        cardid = 0
        for (title, text) in self.text:
            self.heading.addstr(0, ((cardSpacing+1)*2+cardWidth)*cardid, "{0}{1}{0}".format((1+cardSpacing)*" ",cardWidth*"_"))
            self.heading.addstr(1, ((cardSpacing+1)*2+cardWidth)*cardid, ("{0}/{1:^"+str(cardWidth)+"."+str(cardWidth)+"}\{0}").format(cardSpacing*"_",title))
            if cardid==self.activeCard:
                self.heading.addstr(1, ((cardSpacing+1)*2+cardWidth)*cardid+cardSpacing+1, ("{0:^"+str(cardWidth)+"."+str(cardWidth)+"}").format(title), curses.A_REVERSE)
            cardid += 1

    def _redraw(self):
        self._redrawHeading()
        textid=0
        self._nrows=[]
        for (title, text) in self.text:
            rownumber=0
            for line in text.splitlines():
                rows=list(chunks(line,self.width))
                for row in rows:
                    # addstr cannot handle strings containing null-bytes
                    # so remove them before passing the row to addstr
                    row = row.replace('\x00','')
                    try:
                        self.pads[textid].addstr(rownumber, 0, row)
                    except curses.error:
                        self.pads[textid].addstr(rownumber-1, 0, "Maximum rows reached, please use a different viewer for more lines ",curses.A_REVERSE)
                        break
                    rownumber+=1
            self._nrows.append(rownumber)
            textid+=1
    @property
    def height(self):
        return min(self._height,self.parent.getmaxyx()[0]-self.top-3)
    @property
    def width(self):
        return min(self._width, self.parent.getmaxyx()[1])
    @property
    def nrows(self):
        return self._nrows[self.activeCard]


class BottomText(BaseElement):
    def __init__(self, screen, top=1, left=0, height=None, width=None):
        self.parent = screen
        self.top, self.left = top, left
        self._height = default(height, screen.getmaxyx()[0]-top)
        self._width = default(width, screen.getmaxyx()[1])
        self.pad =  curses.newwin(self.height, self.width, top, left)
        self.text = []
    def clear(self):
        self.pad.clear()
        self.text = []
    def addText(self, text):
        self.text.append(text)
        self.text = self.text[-100:]
        self._redraw()
    def refresh(self):
        self.pad.refresh()
    def _redraw(self):
        self.pad.clear()
        rownumber=0
        rows=[]
        for text in self.text:
            for line in text.splitlines():
                rows+=list(chunks(line,self.width))
        rownumber=self.height-1
        for row in reversed(rows):
            if rownumber==-1: break
            try:
                self.pad.addstr(rownumber, 0, row)
            except:
                raise Exception(str(self.top+rownumber)+" "+str(self.top)+" "+str(self.height))
            rownumber-=1
    @property
    def height(self):
        return min(self._height,self.parent.getmaxyx()[0]-self.top-1)
    @property
    def width(self):
        return min(self._width, self.parent.getmaxyx()[1])

class MultiText(BaseElement):
    def __init__(self, screen, maxrows=20000, top=1, left=0, height=None, width=None):
        self.parent = screen
        self.top, self.left = top, left
        self._height = default(height, screen.getmaxyx()[0]-top)
        self._width = default(width, screen.getmaxyx()[1])
        self.pad = curses.newpad(maxrows, self.width)
        self.position = 0
        self.text = []
        self.nrows=0
    def clear(self):
        self.pad.clear()
        self.text = []
    def addFile(self, title, filename):
        f = open(filename, 'r')
        self.addText(title, f.read())
        f.close()
    def addText(self, title, text):
        self.text.append( (title, text) )
        self._redraw()
    def refresh(self):
        self.pad.noutrefresh(self.position, 0, self.top, self.left, self.top+self.height-1, self.left+self.width-1)
    def goUp(self):
        self.position=max(0, self.position-1)
    def goDown(self):
        self.position=min(self.position+1, self.nrows-self.height)
    def pageUp(self):
        self.position=max(self.position-self.height, 0)
    def pageDown(self):
        self.position=min(self.position+self.height, self.nrows-self.height)
    def _redraw(self):
        rownumber=0
        for (title, text) in self.text:
            self.pad.addstr(rownumber, 0, ("{0:^"+str(self.width)+"."+str(self.width-1)+"}").format(title), curses.A_UNDERLINE)
            rownumber+=1
            rows=[]
            for line in text.splitlines():
                rows+=list(chunks(line,self.width))
            for row in rows:
                try:
                    self.pad.addstr(rownumber, 0, row)
                except:
                    raise Exception(str(rownumber))
                rownumber+=1
            rownumber+=2
        rownumber-=2
        self.nrows=rownumber
    def home(self):
        self.position = 0
    def end(self):
        self.position = self.nrows-self.height
    @property
    def height(self):
        return min(self._height,self.parent.getmaxyx()[0]-self.top-1)
    @property
    def width(self):
        return min(self._width, self.parent.getmaxyx()[1])

class Text(BaseElement):
    def __init__(self, screen, maxrows=2000, top=1, left=0, height=None, width=None):
        self.parent = screen
        self.top, self.left = top, left
        self._height = default(height, screen.getmaxyx()[0]-top)
        self._width = default(width, screen.getmaxyx()[1])
        self.pad = curses.newpad(maxrows, self.width)
        self.position = 0
        self.text = ""
        self.nrows=0
        self.pad.scrollok(True)
    def clear(self):
        self.setText("")
    def readFile(self, filename):
        f = open(filename, 'r')
        self.setText(f.read())
        f.close()
    def setText(self, text):
        self.text=text
        self._redraw()
    def refresh(self):
        self.pad.noutrefresh(self.position, 0, self.top, self.left, self.top+self.height-1, self.left+self.width-1)
    def goUp(self):
        self.position=max(0, self.position-1)
    def goDown(self):
        self.position=min(self.position+1, self.nrows-self.height)
    def pageUp(self):
        self.position=max(self.position-self.height, 0)
    def pageDown(self):
        self.position=min(self.position+self.height, self.nrows-self.height)
    def _redraw(self):
        rows=[]
        for line in self.text.splitlines():
            rows+=list(chunks(line,self.width))
        for (row, rownumber) in zip(rows, range(len(rows))):
            self.pad.addstr(rownumber, 0, row)
        self.nrows=len(rows)
    def home(self):
        self.position = 0
    def end(self):
        self.position = self.nrows-self.height
    @property
    def height(self):
        return min(self._height,self.parent.getmaxyx()[0]-self.top-1)
    @property
    def width(self):
        return min(self._width, self.parent.getmaxyx()[1])

def colWidthsReducerMaximum(colWidths, totalWidth):
    # reduces the maximum width by 1 as long as the desired total width is reached
    while sum(colWidths) > totalWidth:
        colWidths[max(xrange(len(colWidths)),key=colWidths.__getitem__)]-=1
    return colWidths

class SelectTable(BaseElement):
    """A Table where a single row can be selected
    """
    def __init__(self, screen, maxrows=1000, top=1, left=0, height=None, width=None, footer=False):
        if footer:
            nfooterlines = 1
        else:
            nfooterlines = 0
        self.parent = screen
        self.top, self.left = top+1, left
        self._height = default(height, screen.getmaxyx()[0]-top)-1-nfooterlines
        self._width = default(width, screen.getmaxyx()[1])
        self.pad = curses.newpad(maxrows, self.width)
        self.header = curses.newwin(1, self.width, top, left)
        self.cursor, self.position = 0, 0
        self.rows, self.formats, self.keys = [], [], []
        self.colFooters = None
    def setColHeaders(self, headers, colwidths=None):
        self.colHeaders = headers
        if colwidths is None:
            x=int(math.floor((self.width-1)/len(self.colHeaders)))
            self.colWidths = [x]*len(self.colHeaders)
        else:
            self.colWidths = colWidthsReducerMaximum(colwidths, self.width-1)
        self._drawColHeaders()
    def setFooters(self, footers):
        self.colFooters = footers
        self.footer = curses.newwin(1, self.width, self.top+self.height, self.left)
        self._drawFooters()
    def _drawFooters(self):
        if self.colFooters is not None:
            for cell, i in zip(self.colFooters, range(len(self.colFooters))):
                if isinstance(cell, (int, long, float, complex)):
                    direction=">"
                else:
                    direction="<"
                self.footer.addstr(0, sum(self.colWidths[:i]), ("{0:"+direction+str(self.colWidths[i]-1)+"."+str(self.colWidths[i]-1)+"} ").format(str(cell)))

    def _drawColHeaders(self):
        for i in range(len(self.colHeaders)):
            self.header.addstr(0, sum(self.colWidths[:i]), ("{0:^"+str(self.colWidths[i])+"."+str(self.colWidths[i]-1)+"}").format(self.colHeaders[i]), curses.A_UNDERLINE)
        if self.width-sum(self.colWidths)-1 > 0:
            self.header.addstr(0, sum(self.colWidths), " "*(self.width-sum(self.colWidths)-1), curses.A_UNDERLINE)

    def addRow(self, row, formatting=None, key=None):
        self.rows.append(row)
        self.formats.append(default(formatting,curses.A_NORMAL))
        self.keys.append(default(key, self.nrows))
        self._redrawRows(self.nrows-1)
    def clear(self):
        self.rows, self.formats, self.keys = [], [], []
        self.pad.clear()
        self.refresh()
    def refresh(self):
        self.pad.noutrefresh(self.position, 0, self.top, self.left, self.top+self.height-1, self.left+self.width-1)
        self._drawColHeaders()
        self.header.noutrefresh()
        if self.colFooters is not None:
            self._drawFooters()
            self.footer.noutrefresh()
    def goUp(self):
        oldcursor = self.cursor
        self.cursor=max(self.cursor-1, 0)
        self._redrawRows(self.cursor, oldcursor)
        if self.cursor-self.position<1/4*self.height and self.position>0:
            self.position -= 1
    def goDown(self):
        oldcursor = self.cursor
        self.cursor=min(self.cursor+1, self.nrows-1)
        self._redrawRows(self.cursor, oldcursor)
        #if self.position<self.cursor-2/3*self.height and self.position+self.height<self.nrows:
        if self.cursor-self.position>3/4*self.height and self.position+self.height<self.nrows:
            self.position += 1
    def _redrawRows(self, *rowids):
        if len(self.rows)==0:
            #nothing to display
            return
        for rownumber in rowids:
            if rownumber==self.cursor:
                formatting = self.formats[rownumber] | curses.A_REVERSE
            else:
                formatting=self.formats[rownumber]
            for cell, i in zip(self.rows[rownumber], range(len(self.rows[rownumber]))):
                if isinstance(cell, (int, long, float, complex)):
                    direction=">"
                else:
                    direction="<"
                #self.pad.addstr(rownumber, sum(self.colWidths[:i]), str(cell)[0:self.colWidths[i]-1], formatting)
                try:
                    self.pad.addstr(rownumber, sum(self.colWidths[:i]), ("{0:"+direction+str(self.colWidths[i]-1)+"."+str(self.colWidths[i]-1)+"} ").format(str(cell)), formatting)
                except:
                    #print ("{0:"+direction+str(self.colWidths[i]-1)+"."+str(self.colWidths[i]-1)+"} ").format(str(cell))
                    pass
            try:
                self.pad.addstr(rownumber, sum(self.colWidths), " "*(self.width-sum(self.colWidths)-1), formatting)
            except:
                pass
    @property
    def nrows(self):
        return len(self.rows)
    def pageUp(self):
        oldcursor = self.cursor
        self.cursor=max(self.cursor-self.height, 0)
        self.position=max(self.position-self.height, 0)
        self._redrawRows(self.cursor, oldcursor)
    def pageDown(self):
        oldcursor = self.cursor
        self.cursor=min(self.cursor+self.height, self.nrows-1)
        self.position=min(self.position+self.height, self.nrows-self.height)
        self._redrawRows(self.cursor, oldcursor)
    def home(self):
        oldcursor = self.cursor
        self.position, self.cursor = 0, 0
        self._redrawRows(self.cursor, oldcursor)
    def end(self):
        oldcursor = self.cursor
        self.position, self.cursor = self.nrows-self.height, self.nrows-1
        self._redrawRows(self.cursor, oldcursor)
    @property
    def selectedRow(self):
        return self.keys[self.cursor]
    @property
    def height(self):
        return min(self._height,self.parent.getmaxyx()[0]-self.top-1)
    @property
    def width(self):
        return min(self._width, self.parent.getmaxyx()[1])



class CursesHandler(logging.Handler):
    def __init__(self, stdscr,bottomText, level=logging.DEBUG):
        logging.Handler.__init__(self, level)
        self.bottomText = bottomText
        self.stdscr = stdscr
    def emit(self, record):
        attr = {"DEBUG":curses.A_NORMAL,
                "INFO":curses.A_NORMAL,
                "WARNING":curses.A_BOLD,
                "ERROR":curses.A_BOLD,
                "CRITICAL":curses.A_STANDOUT}

        self.bottomText.addText(self.format(record)+"\n")

# http://stackoverflow.com/questions/641420/how-should-i-log-while-using-multiprocessing-in-python
class CursesMultiHandler(logging.Handler):
    def __init__(self, stdscr,bottomText, log_q,level=logging.DEBUG):
        logging.Handler.__init__(self)

        self._handler = CursesHandler(stdscr , bottomText, level)
        #~ self.queue = multiprocessing.Queue(-1)
        self.queue = log_q

        t = threading.Thread(target=self.receive)
        t.daemon = True
        t.start()

    def setFormatter(self, fmt):
        logging.Handler.setFormatter(self, fmt)
        self._handler.setFormatter(fmt)

    def receive(self):
        while True:
            try:
                record = self.queue.get()
                self._handler.emit(record)
            except (KeyboardInterrupt, SystemExit):
                raise
            except Queue.Empty:
                pass
            except EOFError:
                break
            except:
                traceback.print_exc(file=sys.stderr)

    def send(self, s):
        self.queue.put_nowait(s)

    def _format_record(self, record):
        # ensure that exc_info and args
        # have been stringified.  Removes any chance of
        # unpickleable things inside and possibly reduces
        # message size sent over the pipe
        if record.args:
            record.msg = record.msg % record.args
            record.args = None
        if record.exc_info:
            dummy = self.format(record)
            record.exc_info = None

        return record

    def emit(self, record):
        try:
            s = self._format_record(record)
            self.send(s)
        except (KeyboardInterrupt, SystemExit):
            raise
        except:
            self.handleError(record)

    def close(self):
        self._handler.close()
        logging.Handler.close(self)
