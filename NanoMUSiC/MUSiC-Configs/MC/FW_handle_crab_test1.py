import os,sys,commands,re,math
import shutil
import fileinput
import optparse
import logging
from termcolor import colored

log = logging.getLogger(__name__)

usage = 'usage: %prog -C cat'
parser = optparse.OptionParser(usage)
parser.add_option('-m', '--mode', dest='mode', action="store_true", default = False, help='Choose your mode:')
parser.add_option('-n', '--dryrun',dest='dryrun', default=False , help='If True, run in dryrun mode')
parser.add_option('-g', '--generator',dest='gen', type="choice", default= "ALL" , choices= ["ALL", "amcatnlo", "madgraph", "pythia"], help='Choose the gen of the folders you want to check. Choises ALL, amcatnlo, madgraph, pythia')

(opt, args) = parser.parse_args()

m1 =opt.mode
dryrun=opt.dryrun
gen=opt.gen

Yfile=open("./myskims_ram_LOR_SKIM1_LG1.yaml")
#Yfile=open("./mc_madgraph.txt")
#Yfile=open("./mc_powheg.txt")
#Yfile=open("./mc_pythia8.txt")
#Yfile=open("./mc_sherpa.txt")
#Yfile=open("./data_prompt.txt")

#ListFile= ["./mc_UL17_amcatnlo.txt", "./mc_UL17_madgraph.txt", "./mc_UL17_pythia.txt"]
p1="./crab_"

Stlist=[]
sest="&"
#sest="_MG"
#sest="_PH"
#sest="_P8"
#sest="_SP"
#sest="Data"

tmps=""
fins=""

cmdIn1=" status "
#cmdIn1=" resubmit "
#cmdIn1=" kill "
#cmdIn1=" purge "
cmdFinal=""

print "Start checking the files"
for line in Yfile.readlines():
    if "#" not in line:
        if sest in line:
            tmps=line.split("&")[1]
            print tmps
            #Stlist.append(tmps)
        tmps=""

#Yfile.close()
print "Finish checking the files"

#for j in Stlist:
#    print colored("\n Start working on Folder:", "green") 
#    print j
#    fins=p1+j
    #print("fins: " , fins)
#    if not os.path.exists(fins):
#        print "The folder  does not exist. Skipping that folder"
        #log.info("The folder: %s  does not exist. Skipping that folder", fins)
#    else:
#        print "Checking the status of the crab folder: ", fins
        #log.info("Checking the status of the crab folder: %s ", fins)
#        cmdFinal="crab"+cmdIn1+fins
            
#        if dryrun is False: 
#            print("Launching command: %s ", cmdFinal)
#            os.system(cmdFinal)
#        else:
#            print("DRYRUN mode. The cmdFinal that should be launched is: %s ", cmdFinal)
#            log.info("DRYRUN mode. The cmdFinal that should be launched is: %s ", cmdFinal)
                
#    print colored("*** NEXT FOLDER *** \n", "green") 
