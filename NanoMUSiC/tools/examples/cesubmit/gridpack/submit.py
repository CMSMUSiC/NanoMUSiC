#! /usr/bin/env python2
import cesubmit
import os
import time
import glob
import subprocess
import logging
logging.basicConfig(level=logging.WARNING)
logger = logging.getLogger(__name__)
import ConfigParser

def main():
        name = "gridpacktest"
        print "Preparing task", name
        uploadurl = cesubmit.createAndUploadGridPack("zeros.sh", "gridpacks/gridpacktest/{createdatetime}.tar.gz")
        task = cesubmit.Task(name)
        task.addGridPack(uploadurl, extractdir="./")
        task.uploadexecutable=False
        task.executable="zeros.sh"
        task.copyResultsToDCache("zeros.txt", "test/{createdatetime}/zeros-{nodeid}_{runid}.txt")
        job=cesubmit.Job()
        task.addJob(job)
        task.submit(6)
        print "[Done]"

if __name__=="__main__":
    main()
