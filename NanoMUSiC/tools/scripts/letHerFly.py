#!/usr/bin/env python
from __future__ import division
import csv
import os
import optparse
import sys
import re
import time
import datetime
import curses
import subprocess

def addtime(tfor,tsince,tto):
    if tsince==None:
        return tfor
    deltat=tto-tsince
    if tfor==None:
        tfor=deltat
    else:
        tfor=tfor+deltat
    return tfor
   
def letHerFly(filename):
    f = open(filename, 'r')
    thiscluster=None
    terminated=dict()
    suspended=dict()
    hold=dict()
    running=dict()
    runningfor=dict()
    runningsince=dict()
    suspendedsince=dict()
    evicted=dict()
    s=""
    year=str(datetime.datetime.now().year)+"/"
    for line in f:
        if "(" not in line: continue
        res = re.match("([0-9]{3})\s\(([0-9]*)\.([0-9]*)\.([0-9]*)\)\s([0-9/]*)\s([0-9\:]*)\s(.*)",line)
        if res:
            if thiscluster==None: thiscluster=res.group(2)
            if thiscluster!=res.group(2): continue
            job=res.group(3)
            status=res.group(1)
            if status=="001":       #executing
                running[job]=True
                runningsince[job]=datetime.datetime.strptime(year+res.group(5)+" "+res.group(6), "%Y/%m/%d %H:%M:%S")
                suspended[job]=False
                runningfor[job]=None
                evicted[job]=False
            elif status=="011":       #unsuspended
                suspended[job]=False
                running[job]=True
                runningsince[job]=datetime.datetime.strptime(year+res.group(5)+" "+res.group(6), "%Y/%m/%d %H:%M:%S")
            elif status=="010":       #suspended
                suspended[job]=True
                suspendedsince[job]=datetime.datetime.strptime(year+res.group(5)+" "+res.group(6), "%Y/%m/%d %H:%M:%S")
                runningfor[job]=addtime(runningfor[job],runningsince[job],suspendedsince[job])
                runningsince[job]=None
                running[job]=False
            elif status=="004":       #evicted
                suspended[job]=False
                running[job]=False
                evicttime=datetime.datetime.strptime(year+res.group(5)+" "+res.group(6), "%Y/%m/%d %H:%M:%S")
                ####runningfor[job]=addtime(runningfor, runningsince, evicttime)
                runningsince[job]=None
                evicted[job]=True
            elif status=="005":       #terminated
                terminated[job]=True
                running[job]=False
                evicttime=datetime.datetime.strptime(year+res.group(5)+" "+res.group(6), "%Y/%m/%d %H:%M:%S")
                runningfor[job]=addtime(runningfor[job],runningsince[job],evicttime)
                runningsince[job]=None
            elif status=="012":       #held
                running[job]=False
                hold[job]=True
                suspended[job]=False
                runningfor[job]=None
                runningsince[job]=None
            elif status=="013":       #released
                hold[job]=False
                suspended[job]=False
    c_total=0
    c_hold=0
    c_suspended=0
    c_terminated=0
    c_running=0
    c_evicted=0
    LJRT=datetime.timedelta(hours=0)
    for i,j in sorted(suspended.items()):
        c_total+=1
        runningfor[i]=addtime(runningfor[i],runningsince[i],datetime.datetime.now())
        if runningfor[i]!=None:
            if runningfor[i]>LJRT: LJRT=runningfor[i]
        if i in running:
            if running[i]: 
                c_running+=1
        if i in evicted:
            if evicted[i]:
                c_evicted+=1
        if i in terminated:
            if terminated[i]==True:
                c_terminated+=1
                continue
        if i in hold:
            if hold[i]==True:
                c_hold+=1 
                #print "Hold:",i
                continue
        if suspended[i]==True:
            c_suspended+=1
            suspendedfor=addtime(None,suspendedsince[i],datetime.datetime.now())
            if suspended[i]==True and runningfor[i]<suspendedfor: 
                s+=thiscluster+"."+i+" "
    if s!="":
        os.system("condor_hold "+s+" >/dev/null")
        time.sleep(2)
        os.system("condor_release "+thiscluster+" >/dev/null")
    elif c_hold>0:
        os.system("condor_release "+thiscluster+" >/dev/null")
    return thiscluster, c_total, c_running, c_hold, c_suspended, c_terminated, c_evicted, LJRT




def main(stdscr):   
    parser = optparse.OptionParser( description='Speed up condor jobs. NB: It is assumed that there is one cluster per log file. Otherwise expect unpredictable behaviour.', usage='usage: %prog logfiles')
    (options, args ) = parser.parse_args()
    stdscr.keypad(1)
    updateInterval=180
    lastUpdate=datetime.datetime.now()-datetime.timedelta(seconds=2*updateInterval)
    stdscr.addstr(0, 0, "letHerFly -- Speed up condor jobs.", curses.A_REVERSE)
    stdscr.addstr(1, 0, "Exit: q  Raise/lower update interval: +/- ("+str(updateInterval)+")  Update:  <SPACE>")
    stdscr.addstr(10, 0, "Cluster  Total Run.  Hold  Susp. Done  Evic. longest job    ",curses.A_UNDERLINE)
    stdscr.timeout(1000)
    curses.curs_set(0)
    stdscr.refresh()
    waitstring="|/-\\"
    j=0
    while True:
        j=j+1
        j=j % len(waitstring)
        stdscr.addch(0,35,waitstring[j])
        i=0
        if lastUpdate+datetime.timedelta(seconds=updateInterval)<datetime.datetime.now():
            stdscr.addch(11,0," ")
            stdscr.clrtobot()
            for filename in args:
                i+=1
                cluster, c_total, c_running, c_hold, c_suspended, c_terminated, c_evicted, LJRT = letHerFly(filename)
                printmode=curses.A_NORMAL
                if c_total==c_terminated: printmode=curses.A_DIM
                stdscr.addstr(10+i, 0, str(cluster), printmode)
                stdscr.addstr(10+i, 9, str(c_total), printmode)
                stdscr.addstr(10+i, 9+6*1, str(c_running), printmode)
                stdscr.addstr(10+i, 9+6*2, str(c_hold), printmode)
                stdscr.addstr(10+i, 9+6*3, str(c_suspended), printmode)
                stdscr.addstr(10+i, 9+6*4, str(c_terminated), printmode)
                stdscr.addstr(10+i, 9+6*5, str(c_evicted), printmode)
                stdscr.addstr(10+i, 9+6*6, str(LJRT), printmode)
            stdscr.refresh()
            lastUpdate = datetime.datetime.now()
        c = stdscr.getch()
        if c == ord('+'):
            updateInterval+=10
            stdscr.addstr(1, 0, "Exit: q  Raise/lower update interval: +/- ("+str(updateInterval)+")  Update:  <SPACE>     ")
            stdscr.refresh()
        elif c == ord('-'): 
            updateInterval=max(10,updateInterval-10)
            stdscr.addstr(1, 0, "Exit: q  Raise/lower update interval: +/- ("+str(updateInterval)+")  Update:  <SPACE>     ")
            stdscr.refresh()
        elif c == ord('q'): break  # Exit the while()
        elif c == ord(' '):
            lastUpdate=datetime.datetime.now()-datetime.timedelta(seconds=2*updateInterval)

if __name__=="__main__":
    curses.wrapper(main)