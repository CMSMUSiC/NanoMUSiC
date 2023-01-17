#! /usr/bin/env python
from __future__ import division
import math
import multiprocessing
import optparse
import os
import random
import re
import string
import subprocess
import time
def id_generator(size=3, chars=string.ascii_letters + string.digits):
   #generate a random string
   return ''.join(random.choice(chars) for x in range(size))
def jdlreplace(value, cluster,process):
    value=re.sub(r"(?i)\$\(cluster\)",str(cluster),value)
    value=re.sub(r"(?i)\$\(process\)",str(process),value)
    return value

def parseSplit(splitstring,nprocesses):
    fractions=int(splitstring.split("/")[1])
    if "-" in splitstring:
        startfraction=int(splitstring.split("/")[0].split("-")[0])
        stopfraction=int(splitstring.split("/")[0].split("-")[1])
    else:
        startfraction=stopfraction=int(splitstring.split("/")[0])
    startprocess=int(math.ceil((startfraction-1)*(nprocesses/fractions)))
    stopprocess=int(math.ceil((stopfraction)*(nprocesses/fractions)-1))
    return startprocess,stopprocess,stopprocess-startprocess+1

def main():
    parser = optparse.OptionParser(usage="usage: %prog [options] file1.jdl file2.jdl...")
    parser.add_option("-c", "--cluster", dest="cluster", help="specify cluster name. Default is a random 3 character alphanumeric string", default=id_generator())
    parser.add_option("-n", "--numberofprocesses", action="store", type="int", dest="numberofprocesses", default=None, help="specify number of parallely run processes. If not specified, number of cores is used")
    parser.add_option("-s", "--split", action="store", type="string", dest="split", default=None, help="use like --split 1/3 to execute the first third of jobs or 1-2/3 to execute the first two thirds of jobs. Helpful when ran on multiple machines")
    parser.add_option("--nice", action="store", type="int", dest="niceness", default=5, help="Set process priority level, default is 5. If >0 it will not block standard processes MAY NOT WORK :(")
    (options, args) = parser.parse_args()
    os.nice(options.niceness)
    q=[]
    for filename in args:
        q.extend(parseJDL(filename,options.cluster))
    numberoftasks=len(q)
    global totaljobs
    totaljobs=len(q)
    print "Found a total of",totaljobs,"jobs."
    if options.split is not None:
        startjob,stopjob,thistotaljobs=parseSplit(options.split,totaljobs)
        print "Running on",thistotaljobs,"jobs."
        print "First job #",startjob,"last job #",stopjob,"."
        q=q[startjob:(stopjob+1)]
    else:
        print "Running on all jobs."
        startjob,stopjob,thistotaljobs=0,totaljobs,totaljobs
    pool = multiprocessing.Pool(options.numberofprocesses)
    r=pool.map_async(worker, q)
    r.wait()
    while True:
        time.sleep(1)
        if not pool._cache: break
    pool.close()
    #pool.terminate()
    #pool.join()
def parseJDL(filename,clustername):
    #parse JDL file
    f = open(filename,"r")
    i,q,item=0,[],dict()
    item['cluster']=clustername
    for line in f:
        if "=" in line:
            key,value = line.split("=",2)[0].strip().lower(), line.split("=",2)[1].strip()
            if key=="executable": item['executable']=value.strip()
            if key=="arguments": item['arguments'] =value.split()
            if key=="error": item['error'] =value.strip()
            if key=="output": item['output'] =value.strip()
        if line.strip().lower()=="queue":
            item['id' ]=i
            i+=1
            q.append(item.copy())
        elif line.strip().lower()[0:5]=="queue":
            numberofexecs=int(line.split()[1])
            for j in range(numberofexecs):
                item['id' ]=i
                i+=1
                q.append(item.copy())
    return q

def worker(item):
    #multiply called function thats starts the program
    calling=[item['executable']]
    if type(item['arguments'])==type([]):
        item['arguments']=" ".join(item['arguments'])
    if "cluster" in item['arguments']:
        item['arguments']=jdlreplace(item['arguments'],item['cluster'],str(item['id']))
    calling.extend(item['arguments'].split(" "))
    print "Starting #",item['id'],"/",totaljobs-1, " ".join(calling)
    outfilename=jdlreplace(item['output'],item['cluster'],str(item['id']))
    errfilename=jdlreplace(item['error'],item['cluster'],str(item['id']))
    fstdout=open(outfilename,"w")
    fstderr=open(errfilename,"w")
    subprocess.call(calling,stdin=None, stdout=fstdout,stderr=fstderr)
    fstdout.close()
    fstderr.close()
    print "Finished #",item['id'],"/",totaljobs-1, " ".join(calling)
if __name__ == "__main__":
    main()
