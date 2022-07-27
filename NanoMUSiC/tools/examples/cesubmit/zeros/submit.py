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
        name = "testsubmit"
        print "Preparing task", name
        task=cesubmit.Task(name)
        task.executable=os.path.abspath("zeros.sh")
        task.outputfiles.append("zeros.txt")
        for i in range(10):
            job=cesubmit.Job()
            task.addJob(job)
        task.submit(6)
        print "[Done]"

if __name__=="__main__":
    main()
