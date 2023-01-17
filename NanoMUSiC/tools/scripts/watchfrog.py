#!/usr/bin/env python
from multiprocessing import Process, Condition, Lock
from multiprocessing.managers import BaseManager
import threading
import os,glob,sys
import argparse
import logging
import fnmatch
import datetime
import time
import curses
import multiprocessing
import Queue

# custom modules
import aachen3adb
import curseshelpers
import gridlib.util
# Command line parsing is added in commandline_parsing
import crabFunctions

from CRABClient.UserUtilities import getConsoleLogLevel, setConsoleLogLevel
from CRABClient.ClientUtilities import LOGLEVEL_MUTE

from music_crab3 import readSampleFile as readMusicCrabConfig

serverLock = threading.Lock()
optionsLock = threading.Lock()
#logging.basicConfig(filename='example.log',level=logging.DEBUG)
logging.basicConfig(filename='example.log',level=logging.DEBUG, format='%(asctime)s %(message)s', datefmt='%m/%d/%Y %I:%M:%S %p')
mylogger = logging.getLogger("watchfrog")

def runserver( options, args):
    # Start a shared manager server and access its queues
    manager = make_server_manager(5001, "blablibliub")
    optionsLock.acquire()
    options.shared_job_q = manager.get_job_q()
    options.shared_result_q = manager.get_result_q()
    options.shared_log_q = manager.get_log_q()
    optionsLock.release()

    mp_crab_worker(options.shared_job_q , options.shared_result_q , max(options.nCores-1 , 1) )

    time.sleep(2)
    serverLock.acquire()
    manager.shutdown()
    serverLock.release()

#~ def main( options , args):
def main(  ):
    print printFrogArt()
    options = commandline_parsing()
    #~ curseshelpers.outputWrapper(runGui, 5,options,args)
    curses.wrapper(runGui, options )

def runGui(stdscr , options ):
    class CrabManager( multiprocessing.managers.BaseManager ):
        pass
    job_q = multiprocessing.Queue()
    result_q = multiprocessing.Queue()
    log_q = multiprocessing.Queue()

    #~ multiprocessing.freeze_support()
    CrabManager.register('Controller', crabFunctions.CrabController)

    CrabManager.register('get_job_q', callable=lambda: job_q)
    CrabManager.register('get_result_q', callable=lambda: result_q)
    CrabManager.register('get_log_q', callable=lambda: log_q)

    #manager = CrabManager(address=('', 5001), authkey='blabliblub')
    manager = CrabManager()
    manager.start()
    optionsLock.acquire()
    options.shared_job_q = manager.get_job_q()
    options.shared_result_q = manager.get_result_q()
    options.shared_log_q = manager.get_log_q()
    optionsLock.release()

    #ch =logging.FileHandler('frog.log', mode='a', encoding=None, delay=False)
    #formatter = logging.Formatter( '%(asctime)s - %(name)s - %(levelname)s - %(message)s' )
    #ch.setFormatter(formatter)
    #ch.setLevel(logging.DEBUG)

    #mylogger.addHandler(ch)
    #mylogger.setLevel(logging.DEBUG)
    #mylogger.setLevel(logging.INFO)

    crabController = manager.Controller( )

    crabWorkers = mp_crab_worker(options.shared_job_q,
                                 options.shared_result_q,
                                 options.shared_log_q,
                                 max(options.nCores-1 , 1),
                                 options.db_key,
                                 options.db_cert)

    # curses color pairs
    curses.init_pair(1, curses.COLOR_RED, curses.COLOR_BLACK)
    curses.init_pair(2, curses.COLOR_GREEN, curses.COLOR_BLACK)
    curses.init_pair(3, curses.COLOR_YELLOW, curses.COLOR_BLACK)
    curses.init_pair(4, curses.COLOR_BLUE, curses.COLOR_BLACK)
    curses.init_pair(5, curses.COLOR_MAGENTA, curses.COLOR_BLACK)

    # setting up curses
    curses.noecho()
    stdscr.keypad(1)

    curses.curs_set(0)
    stdscr.refresh()
    logText = curseshelpers.BottomText(stdscr,top=40)
    # handler without multiprocessing layer
    #ch = curseshelpers.CursesHandler(stdscr,logText)
    #~ ch = curseshelpers.CursesMultiHandler( stdscr, logText, options.shared_log_q )


    stdscr.timeout(1000)
    curses.curs_set(0)

    waitingForExit =  False
    count = 0

    lastUpdate=datetime.datetime.now()

    # get all needed tasks
    if options.musicCrab3Input:
        tasknameList = options.musicCrabSamples
    else:
        crabFolderList = getAllCrabFolders(options)
        tasknameList = []
        for folder in crabFolderList:
            tasknameList.append( folder.replace("crab_","") )

    logText.clear()
    resubmitList = []
    if options.musicCrab3Input:
        overview = Overview( stdscr,
                             tasknameList,
                             resubmitList,
                             job_q,
                             options.dblink,
                             options.proxy,
                             options.globalTag,
                             options.skimmerTag,
                             options.jsonFile )
    else:
        overview = Overview( stdscr,
                             tasknameList,
                             resubmitList,
                             job_q,
                             options.dblink,
                             options.proxy )

    logText._redraw()
    updateFlag = True
    firstUpdate = True
    nInitUpdate = len(overview.tasks)
    mylogger.info(" Finished init. Running GUI now")
    while not waitingForExit:
        count+=1
        curseshelpers.bicolor(stdscr, 3, 0, "(<.*?>)", curses.color_pair(0), curses.color_pair(3)|curses.A_BOLD, "Next update {0}       ".format(timerepr(lastUpdate+datetime.timedelta(seconds=options.updateInterval)-datetime.datetime.now())))
        stdscr.refresh()
        overview.currentView.refresh()
        #filter tasks which should be dropped
        lenbefore = len(overview.tasks)
        overview.tasks[:] = [task for task in overview.tasks if not task.state =="DROP" ]
        if lenbefore  > len(overview.tasks):
            logging.info("Renewing task overview")
            overview.level= 0
            overview.renewTaskOverviews()
            overview.stdscr.clear()
            overview.update()
            if len(overview.tasks)==0: break
        if nInitUpdate != len([task for task in overview.tasks if task.state =="INITUPDATE"]):
            nInitUpdate = len([task for task in overview.tasks if task.state =="INITUPDATE"])
            overview.renewTaskOverviews()
            overview.update()

        # check if new update should be started and add crabTasks to q
        logging.info("Checking for update")
        if lastUpdate+datetime.timedelta(seconds=options.updateInterval)<datetime.datetime.now() or updateFlag:
            if options.proxy.passphrase:
                options.proxy.ensure(3 * options.updateInterval)
            tasks = overview.tasks

            #filter tasks which are still updating
            tasks = filter(lambda task: not task.isUpdating, tasks)
            # check if we can skip crab for finalized samples
            #filter tasks which are already marked as final
            tasks[:] = [task for task in tasks if not task.state in ("FINAL", "FINALIZING") ]

            optionsLock.acquire()
            for task in tasks:
                mylogger.info("adding task %s with state %s updateTime %s to queue"% ( task.name , task.state, task.lastUpdate ))

                #resubmit failed tasks
                if "FAILED" in task.state:
                    task.state == "RESUBMIT"
                elif "COMPLETED" in task.state:
                    task.state = "FINALIZING"
                elif firstUpdate:
                    task.state = "INITUPDATE"
                else:
                    task.state = "UPDATING"
                task.pickle_prepare()
                options.shared_job_q.put(( task.state , task))
                time.sleep(0.1)
            mylogger.info("Added all tasks")
            optionsLock.release()
            firstUpdate = False
            updateFlag = False
            lastUpdate = datetime.datetime.now()

            overview.update()

        try:
            finishedTask = options.shared_result_q.get_nowait()
            finishedTask.pickle_aftercare(options.dblink)
            overview.tasks[:] = [task for task in overview.tasks if not finishedTask.uuid == task.uuid]
            mylogger.info("Appending Task %s with update time %s"% ( finishedTask.name, finishedTask.lastUpdate ) )
            overview.tasks.insert(finishedTask.taskId, finishedTask)
            overview.taskStats = crabFunctions.TaskStats( overview.tasks )
            overview.update()
        except Queue.Empty:
            pass

        stdscr.refresh()
        logText.refresh()
        addInfoHeader(stdscr, options)
        c = stdscr.getch()
        #~ if c < 256 and c > 0:
            #~ mylogger.info(chr(c))
        #~ elif c>0:
            #~ mylogger.info(str(c))
        if c == ord('q') or c == 27 or c == curses.KEY_BACKSPACE:
            # q escape or backspace
            if overview.level:
                overview.up()
            else:
                waitingForExit=True
        elif c == ord('+'):
            options.updateInterval+=30
        elif c == ord('-'):
            options.updateInterval=max(30,options.updateInterval-30)
        elif c == ord(' '):
            overview.update_currentTask()
            overview.update()
        elif c == ord('u'):
            updateFlag = True
        elif c == ord('r'):
            overview.resubmit_currentTask()
            overview.update()
        elif c == ord('f'):
            for task in overview.tasks:
                if task.state == "FINAL":
                    task.state = "DROP"
            overview.currentTask = 0
            overview.update()
        elif c == ord('c'):
            for task in overview.tasks:
                if task.state == "COMPLETED":
                    task.state = "DROP"
            overview.currentTask = 0
            overview.update()
        elif c == curses.KEY_DOWN:
            overview.currentView.goDown()
        elif c == curses.KEY_UP:
            overview.currentView.goUp()
        elif c == curses.KEY_NPAGE:
            overview.currentView.pageDown()
        elif c == curses.KEY_PPAGE:
            overview.currentView.pageUp()
        elif c == curses.KEY_HOME:
            overview.currentView.home()
        elif c == curses.KEY_END:
            overview.currentView.end()
        elif c == 10:   #enter key
            overview.down()
        time.sleep(0.004)
    # free shell from curses
    curses.nocbreak(); stdscr.keypad(0); curses.echo()
    curses.endwin()

    del overview
    for p in crabWorkers:
        try:
            p.terminate()
        except:
            p.terminate()
    time.sleep(2)
    manager.shutdown()

def printFrogArt():
    return "              _     __        __    _       _      __                      _                \n"\
           "  __   ___.--'_`.   \ \      / /_ _| |_ ___| |__  / _|_ __ ___   __ _    .'_`--.___   __    \n"\
           " ( _`.'. -   'o\ )   \ \ /\ / / _` | __/ __| '_ \| |_| '__/ _ \ / _` |  ( /o`   - .`.'_ )   \n"\
           " _\.'_'      _.-'     \ V  V / (_| | || (__| | | |  _| | | (_) | (_| |   `-._      `_`./_   \n"\
           "( \`. )    //\`        \_/\_/ \__,_|\__\___|_| |_|_| |_|  \___/ \__, |     '/\\    ( .'/ )  \n"\
           " \_`-'`---'\\__,                                                 |___/    ,__//`---'`-'_/   \n"\
           "  \`        `-\                                                            /-'        '/    \n"\
           "   `                                                                                 '      \n"\
           " Upquark                       ... setting up the watchfrog ...               DownQuark     \n"


def mp_crab_worker(shared_job_q,
                   shared_result_q,
                   shared_log_q,
                   nprocs,
                   dbkey,
                   dbcert):
    """ Split the work with jobs in shared_job_q and results in
        shared_result_q into several processes. Launch each process with
        factorizer_worker as the worker function, and wait until all are
        finished.
    """
    procs = []
    for i in range(nprocs):
        p = multiprocessing.Process(
                target=crab_worker,
                args=(shared_job_q,
                      shared_result_q,
                      shared_log_q,
                      dbkey,
                      dbcert))
        procs.append(p)
        p.start()

    #~ for p in procs:
        #~ p.join()
    return procs


def crab_worker(job_q, result_q, log_q, dbkey, dbcert):
    """ A worker function to be launched in a separate process. Takes jobs from
        job_q -. When the job is done,
        the result is placed into result_q. Runs until job_q is empty.
    """
    import random
    ch = logging.FileHandler('frog.log', mode='a', encoding=None, delay=False)
    ch.setLevel(logging.DEBUG)
    # create formatter
    formatter = logging.Formatter( '%(asctime)s - %(name)s - %(levelname)s - %(message)s' )
    time.sleep(random.randint(0, 150) / 100.  )
    # add formatter to ch
    ch.setFormatter(formatter)
    mylogger = logging.getLogger('worker')
    mylogger.setLevel(logging.ERROR)
    mylogger.addHandler(ch)
    failcount = 0
    dblink = aachen3adb.ObjectDatabaseConnection(key=dbkey, certificate=dbcert)
    dblink.authorize()
    while True:
        try:
            #~ mylogger.info('tring to git from q')
            mylogger.info('Getting next job from queue')
            ( state, crabTask ) = job_q.get_nowait()

            crabTask.pickle_aftercare(dblink)
            mylogger.info('in worker updating Task %s now state %s '% (crabTask.name,
                                                                       state))
            if "RESUBMIT" in state:
                failedJobIds = []
                now = datetime.datetime.now()
                mylogger.info('in worker rseubmit taks')
                crabTask.resubmit_failed()
            if "FINALIZING" in state:
                if not crabTask.isFinal:
                    crabTask.finalizeTask()

            if "INITUPDATE" in state:
                crabTask.update(updateDB=False)
            else:
                crabTask.update()
            mylogger.info('in worker updated Task %s now state %s '% (crabTask.name, crabTask.state ) )
            crabTask.pickle_prepare()
            result_q.put( crabTask )
        except Queue.Empty:
            #~ mylogger.info("finished all crab update tasks in q")
            time.sleep(1.)
            time.sleep( random.randint(0, 100) / 100. )
        except Exception as e:
            failcount += 1
            mylogger.error('something went wrong in the worker %d times: \n %s' % (failcount,str(e)))
            if failcount == 5:
                return
            mylogger.error('something went wrong in the worker: \n %s' % str(e))
            mylogger.error('Task: %s State: %s' % (crabTask.name, state) )
            failcount += 1
            if failcount == 5:
                return

def getAllCrabFolders(options):
    # get all crab folders in working directory
    crabFolders = [f for f in os.listdir(options.workingArea) if os.path.isdir(os.path.join(options.workingArea, f)) and "crab_" in f]
    # check if only folders, which match certain patterns should be watched
    if options.only:
        filteredFolders = []
        # loop over all user specified patterns
        for pattern in options.only:
            #loop over all crabFolders in working directory and
            # them to filteredFolder list if they match the pattern
            for crabFolder in crabFolders:
                if fnmatch.fnmatchcase( crabFolder, pattern ):
                    filteredFolders.append(crabFolder)
        crabFolders = filteredFolders
    if len(crabFolders) < 1:
        mylogger.error("found no folder with crab_ in working directory")
        sys.exit(1)
    return crabFolders


class Overview(object):
    def __init__(self,
                 stdscr,
                 taskNameList,
                 resubmitList,
                 shared_job_q,
                 dblink,
                 proxy,
                 default_globalTag = None,
                 default_skimmer_version = None,
                 default_json_file =  None):
        self.level = 0
        self.taskId = 0
        self.cursor = 0
        self.stdscr = stdscr
        self.shared_job_q = shared_job_q
        self.taskOverviews = []
        self.tasks = []
        # can be deleted in cleanup ?
        for taskName in taskNameList:
            self.tasks.append( crabFunctions.CrabTask( taskName,
                initUpdate = False,
                dblink= dblink,
                proxy=proxy,
                globalTag = default_globalTag,
                skimmer_version = default_skimmer_version,
                json_file = default_json_file ) )
        self.height=stdscr.getmaxyx()[0]-16
        self.height=stdscr.getmaxyx()[0]-16
        self.tasktable = curseshelpers.SelectTable(stdscr, top=5, height=self.height, maxrows=50+len(self.tasks))
        widths=[5, 100, 13, 11, 11, 11, 11, 11, 11, 11 , 11, 20]
        self.tasktable.setColHeaders(["#","Task", "Status", "nJobs", "Unsubmitted", "Idle", "Run.","Cooloff","Fail.","Trans","Finished", "last Update"],widths)
        self.renewTaskOverviews()
        self.taskStats = crabFunctions.TaskStats(self.tasks)
        self.currentView = self.tasktable


        self.update()
    def __del__(self):
        serverLock.release()

    def renewTaskOverviews(self):
        self.taskOverviews = []
        for task in self.tasks:
            #~ taskOverview = curseshelpers.SelectTable(stdscr, top=4, height=self.height, maxrows=100+task.nJobs)
            taskOverview = curseshelpers.SelectTable(self.stdscr, top=5, height=self.height, maxrows=50+task.nJobs)
            widths=[5, 15, 22, 5, 5, 35 ,20 ,20 ,20]
            taskOverview.setColHeaders(["#","JobID", "State", "Retries", "Restarts", "Sites","SubmitTime" , "StartTime","EndTime"], widths)
            self.taskOverviews.append(taskOverview)

    def update(self):
        self.tasktable.clear()

        for (taskId, taskOverview, task) in zip(range(len(self.tasks)), self.taskOverviews, self.tasks):

            printmode = self.getPrintmode(task)
            task.taskId = taskId
            cells = [task.taskId, task.name ,task.state ,task.nJobs , task.nUnsubmitted , task.nIdle, task.nRunning , task.nCooloff , task.nFailed, task.nTransferring , task.nFinished , task.lastUpdate]
            self.tasktable.addRow( cells , printmode )
            taskOverview.clear()
            for jobkey in sorted(task.jobs.keys(), key=int):
                job = task.jobs[jobkey]
                if not 'EndTimes' in job.keys():
                    jobendtimes = ''
                elif len(job['EndTimes']) > 0:
                    jobendtimes = formatedUnixTimestamp(job['EndTimes'][-1])
                else:
                    jobendtimes = ''
                try:
                    taskOverview.addRow( [jobkey,
                                          job['JobIds'][-1],
                                          job['State'],
                                          job['Retries'],
                                          job['Restarts'],
                                          ' '.join(job['SiteHistory']),
                                          formatedUnixTimestamp(job['SubmitTimes'][-1]),
                                          formatedUnixTimestamp(job['StartTimes'][-1]),
                                           jobendtimes] )
                except:
                    pass
        self.tasktable.refresh()
        cells = [self.taskStats.nTasks, "Tasks total", "Job Stats" , sum(t.nJobs for t in self.tasks ) , self.taskStats.nUnsubmitted, self.taskStats.nIdle, self.taskStats.nRunning, self.taskStats.nCooloff,self.taskStats.nFailed, self.taskStats.nTransferring , self.taskStats.nFinished ]
        self.tasktable.setFooters(cells)
        self._refresh()

    def getPrintmode(self,task):
        if task.state in ["UPDATING", "FINALIZING", "INITUPDATE"]:
            # blue
            printmode = curses.color_pair(4)
            #~ printmode = printmode | A_BLINK
        elif "SUBMITTED" in task.state:
            if task.nRunning > 0:
                # blue
                printmode = curses.color_pair(4)
            else:
                # yellow
                printmode = curses.color_pair(3)
        elif task.state.startswith("CREATED:"):
            # yellow
            printmode = curses.color_pair(3)
        elif "COMPLETE" in task.state:
            #green
            printmode = curses.color_pair(2)
        elif "FINAL" in task.state:
            #green
            printmode = curses.color_pair(2)
        elif "DONE" in task.state:
            printmode = curses.color_pair(2)
            printmode = printmode | curses.A_BOLD
        else:
            # red
            printmode = curses.color_pair(1)
        return printmode

    @property
    def currentTask(self):
        return self.tasktable.cursor

    @currentTask.setter
    def currentTask(self, value):
        self.tasktable.cursor = 0

    def update_currentTask(self):
        if self.level == 0:
            if self.tasks[self.currentTask].state == "COMPLETED":
                self.tasks[self.currentTask].state = "FINALIZING"
            elif self.tasks[self.currentTask].state == "FINALIZING":
                return
            elif self.tasks[self.currentTask].state == "FINAL":
                return
            else:
                self.tasks[self.currentTask].state = "UPDATING"
            self.tasks[self.currentTask].pickle_prepare()
            self.shared_job_q.put_nowait( (self.tasks[self.currentTask].state
                                        ,self.tasks[self.currentTask]) )
    def resubmit_currentTask(self):
        if self.level == 0:
            self.tasks[self.currentTask].state = "RESUBMIT"
            self.tasks[self.currentTask].pickle_prepare()
            self.shared_job_q.put_nowait( (self.tasks[self.currentTask].state
                                        ,self.tasks[self.currentTask]) )
    def down(self):
        self.stdscr.clear()
        self.level=min(self.level+1,1)
        self._refresh()
    def up(self):
        self.stdscr.clear()
        self.level=max(self.level-1,0)
        self._refresh()
    def _refresh(self):
        if self.level==0:
            self.currentView = self.tasktable
        elif self.level==1:
            self.currentView = self.taskOverviews[self.currentTask]
        else:
            print "no recognized level for overview"
        self.currentView.refresh()

def addInfoHeader(stdscr, options):
    stdscr.addstr(0, 0, ("{0:^"+str(stdscr.getmaxyx()[1])+"}").format("watchfrog quark...quark"), curses.A_REVERSE)
    #~ self.stdscr.addstr(0, 0, ("{0:^"+str(self.stdscr.getmaxyx()[1])+"}").format(self.asciiFrog), curses.A_REVERSE)
    #~ self.stdscr.addstr(8, 0, "Exit: q  Raise/lower update interval: +/- ("+str(options.updateInterval)+"s)  Update:  <SPACE>")
    curseshelpers.bicolor(stdscr, 1, 0, "(<.*?>)", curses.color_pair(0), curses.color_pair(3)|curses.A_BOLD, "Exit: <q>  Back <BACKSPACE>  Raise/lower update interval: <+>/<-> ("+str(options.updateInterval)+"s)  More information <return>")
    curseshelpers.bicolor(stdscr, 2, 0, "(<.*?>)", curses.color_pair(0), curses.color_pair(3)|curses.A_BOLD, "Update Task: <SPACE> Update all: <u> Resubmit failed: <r> clear final: <f> clear completed: <c>")

def timerepr(deltat):
    """Return formatted time interval
    """
    if deltat.days<0:
        return "now"
    hours, seconds=divmod(deltat.seconds, 60*60)
    minutes, seconds=divmod(seconds, 60)
    if deltat.days: return "in {0}d {1}h {2}m {3}s".format(deltat.days, hours, minutes, seconds)
    if hours: return "in {0}h {1}m {2}s".format(hours, minutes, seconds)
    if minutes: return "in {0}m {1}s".format(minutes, seconds)
    return "in {0}s".format(seconds)

def formatedUnixTimestamp (unixTimeStamp):
    return datetime.datetime.fromtimestamp( int(unixTimeStamp) ).strftime('%Y-%m-%d %H:%M:%S')


def commandline_parsing():
    descr = 'Watchfrog a tool for interactive monitoring of PxlSkim campaigns'
    parser = argparse.ArgumentParser(description= descr)
    parser.add_argument( '-o', '--only', metavar='PATTERNS', default=None,
                       help='Only check samples matching PATTERNS (bash-like ' \
                            'patterns only, comma separated values. ' \
                            'E.g. --only QCD* ). [default: %(default)s]' )
    parser.add_argument( '-u','--user', help='Alternative username [default is HN-username]')
    parser.add_argument( '-p','--ask-passphrase', help='Alternative username [default is HN-username]')
    parser.add_argument('-m', '--musicCrab3Input',  type=str, nargs='+',
                   help='A list of music_crab input files. Monitores global progress via db')
    parser.add_argument( '-g','--globalTag', help='Global tag to be used for db queries if no crabConfig is found')
    parser.add_argument( '-s','--skimmerTag', help='SkimmerTag to be used if no crabConfig found')
    parser.add_argument( '-j','--jsonFile', help='Json to be used if no crabConfig found')
    parser.add_argument( '-c','--clearFinal', help='clearFinal')
    parser.add_argument('--db-key', default='~/private/CERN_UserCertificate.pem',
                       help="Key for authentication to 'https://cms-project-aachen3a-db.web.cern.ch'. [default: %(default)s]" )
    parser.add_argument( '--db-cert', default='~/private/CERN_UserCertificate.key',
                       help="Cert for authentication to 'https://cms-project-aachen3a-db.web.cern.ch'. [default: %(default)s]" )
    parser.add_argument( '--workingArea',metavar='DIR',help='The area (full or relative path) where the CRAB project directories are saved. ' \
                     'Defaults to the current working directory.'       )
    parser.add_argument( '--updateInterval', default=600,help='Time between two updates for crab tasks in seconds.')
    parser.add_argument( '--nCores', default=multiprocessing.cpu_count(),help='Number of cores to use [default: %(default)s]')
    parser.add_argument( '--crabDebug',action="store_true",
    help='Allow crab debug (may create large logs)')


    options = parser.parse_args()

    if options.musicCrab3Input:
        options.maxJobRuntimeMin = -1
        options.config = ""
        options.config_dir = ""
        options.unitsPerJob = -1
        options.musicCrabSamples = []

        for path in options.musicCrab3Input:
            print "reading",path
            outdict = readMusicCrabConfig( path, options )
            musicCrabSamples = outdict['sampledict'].keys()
            options.musicCrabSamples += musicCrabSamples
        if not options.globalTag:
            print "no --globalTag specified, will only check samples with existing crabConfig"
            options.globalTag = None
        if not options.skimmerTag:
            print "no --skimmerTag specified, will only check samples with existing crabConfig"
            options.skimmerTag = None
        if not options.jsonFile:
            print "no --jsonFile specified, will only check samples with existing crabConfig"
            options.jsonFile = None

    now = datetime.datetime.now()
    isodatetime = now.strftime( "%Y-%m-%d_%H.%M.%S" )
    options.isodatetime = isodatetime

    # supress crab output if needed
    if not options.crabDebug:
        setConsoleLogLevel(LOGLEVEL_MUTE)
    if options.workingArea:
        options.workingArea = os.path.abspath(options.workingArea)
    else:
        options.workingArea = os.path.abspath(os.getcwd())

    options.runServer = True
    # get pass before starting

    options.dblink = aachen3adb.ObjectDatabaseConnection(key=options.db_key,
                                                         certificate=options.db_cert)
    options.dblink.authorize()
   # check if user has valid proxy
    import getpass
    proxy = gridlib.util.VomsProxy()
    expired = False
    min_lifetime = 15 * options.updateInterval
    if proxy.timeleft < min_lifetime:
        print("Your proxy is valid for only %d seconds." % proxy.timeleft)
        expired = True
    if options.ask_passphrase or expired:
        proxy.passphrase = getpass.getpass('Please enter your GRID pass phrase for proxy renewal:')
        proxy.ensure(min_lifetime)
    options.proxy = proxy
    #get current user HNname
    if not options.user:
        options.user = gridlib.util.get_username_cern()
    return options

if __name__ == '__main__':
    # get command line arguments

    #~ runserver( options, args )
    #~ main( options, args )
    main(  )
