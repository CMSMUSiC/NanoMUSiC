##@package helper
# helper for RATA_PDF pdf calculator
#
# Important helping functions for the actual PDF uncertainty calculation
#
# written by Soeren Erdweg 2013-2014

#!/usr/bin/env python

## Necessary imports
import os
import sys
import subprocess
sys.path.append("config/")
sys.path.append("lib/")
from configobj import ConfigObj
import time
from datetime import datetime
import ROOT as r
import logging
from ctypes import *

## @var Process info
info = r.ProcInfo_t()

## Class for the different bash text colors
#
# To have an easy way to change the bash text color and to unset them
# again, this calss can be used.
class bcolors:
    HEADER = '\033[35m'
    OKBLUE = '\033[34m'
    OKGREEN = '\033[32m'
    WARNING = '\033[33m'
    FAIL = '\033[31m'
    CYAN = '\033[36m'
    WHITE = '\033[37m'
    ENDC = '\033[0m'

    ## Function to unset all colors
    def disable(self):
        self.HEADER = ''
        self.OKBLUE = ''
        self.OKGREEN = ''
        self.WARNING = ''
        self.FAIL = ''
        self.CYAN = ''
        self.WHITE = ''
        self.ENDC = ''

## Function to get the size of the terminal
#
# Reads the width and height of the current terminal and returns
# this two values.
# @param[out] int(cr[1]) Width of the terminal
# @param[out] int(cr[0]) Height of the terminal
def getTerminalSize():
    env = os.environ
    def ioctl_GWINSZ(fd):
        try:
            import fcntl, termios, struct, os
            cr = struct.unpack('hh', fcntl.ioctl(fd, termios.TIOCGWINSZ,
        '1234'))
        except:
            return
        return cr
    cr = ioctl_GWINSZ(0) or ioctl_GWINSZ(1) or ioctl_GWINSZ(2)
    if not cr:
        try:
            fd = os.open(os.ctermid(), os.O_RDONLY)
            cr = ioctl_GWINSZ(fd)
            os.close(fd)
        except:
            pass
    if not cr:
        cr = (env.get('LINES', 25), env.get('COLUMNS', 80))
    return int(cr[1]), int(cr[0])

## Function to create and update a progress bar
#
# This function displays or updates a console progress bar
# It accepts a float between 0 and 1. Any int will be converted to a float.
# A value under 0 represents a 'halt'.
# A value at 1 or bigger represents 100%
# @param[in] progress Relative progress that should be displayed
def update_progress(progress):
    (width, height) = getTerminalSize()
    barLength = width-30 # Modify this to change the length of the progress bar
    status = ""
    if isinstance(progress, int):
        progress = float(progress)
    if not isinstance(progress, float):
        progress = 0
        status = bcolors.FAIL+"error: progress var must be float\r\n"+bcolors.ENDC
    if progress < 0:
        progress = 0
        status = bcolors.WARNING+"Halt...\r\n"+bcolors.ENDC
    if progress >= 1:
        progress = 1
        status = bcolors.OKGREEN+"Done...\r\n"+bcolors.ENDC
    block = int(round(barLength*progress))
    text = "\rPercent: [{0}] {1}% {2}".format( bcolors.HEADER+"#"*block+bcolors.ENDC + "-"*(barLength-block), int(progress*100), status)
    sys.stdout.write(text)
    sys.stdout.flush()

## Welcome output function
#
# Function to print the welcome output, consisting out of the ASCII art,
# the programs name and the local time
def welcome_output():
    logging.info("starting RATA PDF")
    logging.info("")
    print("")
    print(bcolors.HEADER+"8 888888888o.            .8.    8888888 8888888888   .8.                    8 888888888o   8 888888888o.      8 8888888888   ")
    print("8 8888    `88.          .888.         8 8888        .888.                   8 8888    `88. 8 8888    `^888.   8 8888         ")
    print("8 8888     `88         :88888.        8 8888       :88888.                  8 8888     `88 8 8888        `88. 8 8888         ")
    print("8 8888     ,88        . `88888.       8 8888      . `88888.                 8 8888     ,88 8 8888         `88 8 8888         ")
    print("8 8888.   ,88'       .8. `88888.      8 8888     .8. `88888.                8 8888.   ,88' 8 8888          88 8 888888888888 ")
    print("8 888888888P'       .8`8. `88888.     8 8888    .8`8. `88888.               8 888888888P'  8 8888          88 8 8888         ")
    print("8 8888`8b          .8' `8. `88888.    8 8888   .8' `8. `88888.              8 8888         8 8888         ,88 8 8888         ")
    print("8 8888 `8b.       .8'   `8. `88888.   8 8888  .8'   `8. `88888.             8 8888         8 8888        ,88' 8 8888        ")
    print("8 8888   `8b.    .888888888. `88888.  8 8888 .888888888. `88888.            8 8888         8 8888    ,o88P'   8 8888       ")
    print("8 8888     `88. .8'       `8. `88888. 8 8888.8'       `8. `88888.           8 8888         8 888888888P'      8 8888      "+bcolors.ENDC)
    print("")
    print("RWTH Aachen Three A Parton Distribution Functions calculator")
    print("")
    print(time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime()))
    print("")

## Farewell output function
#
# Function to print the farewell output, consisting out of the local time,
# the programs run time and memory usage
# @param[in] t0 Total wall time
# @param[in] t1 Total CPU time
# @todo fix the crash at the end of the program
def farewell_output(t0,t1):
    print("")
    print("-"*20)
    print(bcolors.HEADER + "\t All calculations done" + bcolors.ENDC)
    print("-"*20)
    print("\t" + time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime()))
    print("\truntime in seconds : ")
    print("\t" + bcolors.OKGREEN + str(time.clock() - t0) + bcolors.ENDC + " (process time)")
    print("\t" + bcolors.OKGREEN + str(time.time() - t1) + bcolors.ENDC + " (wall time)")
    print("")
    r.gSystem.GetProcInfo(info)
    print("\tmemory in MB : ")
    print("\t" + bcolors.OKGREEN + str(info.fMemResident/1000.) + bcolors.ENDC + " (resident) ")
    print("\t" + bcolors.OKGREEN + str(info.fMemVirtual/1000.) + bcolors.ENDC + " (virtual) ")
    print("-"*20)
    print("")
    print("")
    raw_input("The program will now crash")

## Usage function, prints the usage information
def Usage():
    return '%prog [options] CONFIG_FILE'

## Function to do the options input parsing
#
# In this function the different possible comand line arguments are defined
# and read from the user input, they are also checked for sanity.
# @param[out] options Options object with all user defined input options
# @param[out] numeric_level Numeric value for the logging level
def option_parsing():
    import optparse

    date_time = datetime.now()
    usage = Usage()
    parser = optparse.OptionParser( usage = usage )
    parser.add_option( '--debug', metavar = 'LEVEL', default = 'INFO',
                       help= 'Set the debug level. Allowed values: ERROR, WARNING, INFO, DEBUG. [default = %default]' )
    parser.add_option( '--logfile', default = 'log_file.log',
                       help= 'Set the logfile. [default = %default]' )

    run_group = optparse.OptionGroup( parser, 'Run options', 'The same as for the Run executable!' )

    run_group.add_option( '-s', '--Signal', action = 'store_true', default = False,
                            help = 'Run on Signal samples. [default = %default]' )
    run_group.add_option( '-b', '--Background', action = 'store_true', default = False,
                            help = 'Run on Background samples. [default = %default]' )
    parser.add_option_group( run_group )

    cfg_group = optparse.OptionGroup( parser, 'Cfg options', 'The same as for the Run executable!' )

    cfg_group.add_option( '-a', '--SignalCfg', default = 'Sig.cfg', metavar = 'DIRECTORY',
                            help = 'Signal sample config file. [default = %default]' )
    cfg_group.add_option( '-e', '--BackgroundCfg', default = 'Bag.cfg', metavar = 'DIRECTORY',
                            help = 'Signal sample config file. [default = %default]' )
    cfg_group.add_option( '-c', '--XsCfg', default = 'xs.cfg', metavar = 'DIRECTORY',
                            help = 'Cross section config file. [default = %default]' )
    cfg_group.add_option( '-d', '--PDFCfg', default = 'pdf.cfg', metavar = 'DIRECTORY',
                            help = 'PDF config file. [default = %default]' )
    parser.add_option_group( cfg_group )

    ( options, args ) = parser.parse_args()

    if not options.Signal and not options.Background:
        parser.error( 'Specify to either run on the Signal or on the Background samples!' )

    numeric_level = getattr(logging, options.debug.upper(), None)
    if not isinstance(numeric_level, int):
        raise ValueError('Invalid log level: %s' % options.debug)
    if os.path.exists(options.logfile):
        os.remove(options.logfile)
    logging.basicConfig(filename=options.logfile,level=numeric_level,format='%(asctime)s - %(name)s - %(levelname)s - %(message)s', datefmt='%m/%d/%Y %I:%M:%S %p')
    return options,numeric_level

## Function to do the config file parsing
#
# This functions reads in the necessary config files, for the samples that
# should be analyzed (mc_cfg), which cross section they have (xs_cfg) and
# which PDF sets should be used how (pdf_cfg). The config files are also
# checked for sanity.
# @param[out] mc_cfg Config for the samples that should be analyzed
# @param[out] xs_cfg Config for the cross section of the samples
# @param[out] pdf_cfg Config for the PDF sets that should be used
def config_parsing(options):
    if options.Signal:
        try:
            mc_cfg = ConfigObj(options.SignalCfg)
            logging.info('read config file %s',options.SignalCfg)
        except IOError as e:
            print("There was a error reading the File "+options.SignalCfg)
            print(e)
            logging.error("There was a error reading the File "+options.SignalCfg)
            logging.error(e)
            exit()
    else:
        try:
            mc_cfg = ConfigObj(options.BackgroundCfg)
            logging.info('read config file %s',options.BackgroundCfg)
        except IOError as e:
            print("There was a error reading the File "+options.BackgroundCfg)
            print(e)
            logging.error("There was a error reading the File "+options.BackgroundCfg)
            logging.error(e)
            exit()
    try:
        xs_cfg = ConfigObj(options.XsCfg)
        logging.info('read config file %s',options.XsCfg)
    except IOError as e:
        print("There was a error reading the File "+options.XsCfg)
        print(e)
        logging.error("There was a error reading the File "+options.XsCfg)
        logging.error(e)
        exit()
    try:
        pdf_cfg = ConfigObj(options.PDFCfg)
        logging.info('read config file %s',options.PDFCfg)
    except IOError as e:
        print("There was a error reading the File "+options.PDFCfg)
        print(e)
        logging.error("There was a error reading the File "+options.PDFCfg)
        logging.error(e)
        exit()
    return mc_cfg,xs_cfg,pdf_cfg

## Function that does a control output
#
# To check if all program parameters are correct, the most important
# ones are printed in this function for a quick cross check.
# @param[in] options Options object with all user defined input options
# @param[in] mc_cfg Config for the samples that should be analyzed
# @param[in] pdf_cfg Config for the PDF sets that should be used
# @param[in] xs_cfg Config for the cross section of the samples
def control_output(options,mc_cfg,pdf_cfg,xs_cfg):
    print("\n"+"-"*20)
    if options.Signal:
        print("\t Running on "+bcolors.OKGREEN+"Signal"+bcolors.ENDC+" samples")
        logging.info("Running on Signal samples")
    else:
        print("\t Running on "+bcolors.OKGREEN+"Background"+bcolors.ENDC+" samples")
        logging.info("Running on Background samples")
    print("-"*20)
    print("\t PDF sets to be used:")
    logging.info("PDF sets to be used:")
    for pdfs in pdf_cfg["PDFs"]:
        print("\t  -"+bcolors.OKGREEN+pdfs+bcolors.ENDC)
        logging.info("-"+pdfs)
    print("-"*20)
    print("\t MC samples to be used:")
    logging.info("MC samples to be used:")
    for sample in mc_cfg["samples"]:
        print("\t  -"+bcolors.OKGREEN+sample+bcolors.ENDC+"  xs: %s"%(float(xs_cfg[sample]["xs"])*float(xs_cfg[sample]["weight"])))
        logging.info("-"+sample+"  xs: %s"%(float(xs_cfg[sample]["xs"])*float(xs_cfg[sample]["weight"])))

## Function to get list of event numbers
#
# This functions calculates the number of total events that should be
# analyzed and returns a list of the files that should be anlyzed and
# a list of events in each file.
# @param[in] mc_cfg Config for the samples that should be analyzed
# @param[in] pdf_cfg Config for the PDF sets that should be used
# @param[in] path Path to the files that should be analyzed
# @param[out] filelist List of files to be analyzed
# @param[out] total_events Total number of events
# @param[out] eventlist List of number of events for each file
def get_event_number_list(mc_cfg,pdf_cfg,path):
    filelist = []
    total_events = 0.
    eventlist = {}
    for sg in mc_cfg["samples"]:
        dummy_events = get_event_number(path+sg+".root",pdf_cfg["Tree"]["tree_name"],pdf_cfg["Tree"]["cut_string"])
        if dummy_events > 0:
            total_events += dummy_events
            eventlist.update({sg:dummy_events})
            filelist.append(sg)
    print("-"*20)
    print("\t Running on "+bcolors.OKGREEN+str(int(total_events))+bcolors.ENDC+" events")
    print("-"*20+"\n")
    return filelist,total_events,eventlist

## Function to get the number of events in one file
#
# This function reads the number of events from a given file
# @param[in] file_name Name of the file that should be analyzed
# @param[in] tree_name Name of the tree in the file
# @param[in] cut_string Cuts that should be apllied on the tree
# @param[out] nentries Number of events in the file
def get_event_number(file_name,tree_name,cut_string):
    try:
        tfile = r.TFile(file_name,"READ")
        tree = tfile.Get(tree_name)
        dummy_file = r.TFile("tmp/tmpFile2_.root","RECREATE")
        smallerTree = tree.CopyTree(cut_string)
        nentries = smallerTree.GetEntries()
        tfile.Close()
        dummy_file.Close()
        return nentries
    except:
        print("-"*20)
        print("\t Can't read "+bcolors.FAIL+file_name+bcolors.ENDC+", skipping it")
        print("-"*20)
        return 0

## Function to check if there is anythin in a file
#
# This function checks if the list of keys for a given file
# is bigger than zero, to check if the file is usable
# @param[in] file_name Name of the file to be checked
# @param[out] bool Either true (good file) or False (bad file)
def check_file(file_name):
    tfile = r.TFile(file_name,"READ")
    hists = tfile.GetListOfKeys().GetSize()
    if hists <= 1:
        print("-"*20)
        print("\t Output of "+bcolors.FAIL+file_name+bcolors.ENDC+" is not okay, will not be used for PDF calculation")
        print("-"*20)
        return False
    else:
        return True

## Function to check all output files
#
# This function calls check_file on every outputfile that should
# be produced. If the PDF calculation is done on background samples
# the output is also merged.
# @param[in] options Options object with all user defined input options
# @param[in] run_samples List of samples that were analyzed
# @param[in] pdf_cfg Config for the PDF sets that should be used
# @param[out] run_samples List of samples that were analyzed and pass the check
def final_file_check(options,run_samples,pdf_cfg):
    print("-"*20)
    print("\t Now checking all output files")
    print("-"*20+"\n")
    if options.Signal:
        for sg in run_samples:
            if not check_file(sg):
                run_samples.remove(sg)
    else:
        for sg in run_samples:
            if not check_file(sg):
                run_samples.remove(sg)
        print("-"*20)
        print("\t Now merging all background files")
        print("-"*20+"\n")
        command= "hadd -f9 "+pdf_cfg["general"]["temp_path"]+"allMCs.root "
        for i in run_samples:
            command += " " + i
        p = subprocess.Popen(command,shell=True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
        out, err = p.communicate()
        logging.debug(out)
        logging.debug(err)
        run_samples = [pdf_cfg["general"]["temp_path"]+"allMCs.root"]
    return run_samples

## Function to convert the parameters for the C++ library
#
# To have all parameters readable by the C++ library functions,
# they need to be converted, this is done by this function.
# @param[in] mc_cfg Config for the samples that should be analyzed
# @param[in] pdf_cfg Config for the PDF sets that should be used
# @param[out] paras Dictionary of the converted parameters
def make_c_parameters(mc_cfg,pdf_cfg):
    paras = {}
    logging.info('preparing paramters for C++ functions ...')
    # Get the path of the MC samples:
    path = mc_cfg["general"]["path"]
    paras.update({"path":path})
    logging.debug('path: %s',path)
    # Get the name of the tree branches:
    c_branches = (c_char_p * 7)()
    logging.debug('branches: %s, %s, %s, %s, %s, %s, %s',
    pdf_cfg["Tree"]["b_pdf_scale"],
    pdf_cfg["Tree"]["b_pdf_id1"],
    pdf_cfg["Tree"]["b_pdf_id2"],
    pdf_cfg["Tree"]["b_pdf_x1"],
    pdf_cfg["Tree"]["b_pdf_x2"],
    pdf_cfg["Tree"]["b_observe"],
    pdf_cfg["Tree"]["b_weight"])
    paras.update({"branches":c_branches})
    c_branches[:] = [pdf_cfg["Tree"]["b_pdf_scale"],
    pdf_cfg["Tree"]["b_pdf_id1"],
    pdf_cfg["Tree"]["b_pdf_id2"],
    pdf_cfg["Tree"]["b_pdf_x1"],
    pdf_cfg["Tree"]["b_pdf_x2"],
    pdf_cfg["Tree"]["b_observe"],
    pdf_cfg["Tree"]["b_weight"]]
    # Get the names of the PDF sets:
    n_pdf_sets = 0
    PDFsets = []
    for i_pdf in pdf_cfg["PDFs"]:
        n_pdf_sets+=1
        PDFsets.append(i_pdf)
        logging.debug('append pdf set: %s',i_pdf)
    logging.debug('number of pdf sets: %i',n_pdf_sets)
    c_PDFsets = (c_char_p * n_pdf_sets)()
    c_PDFsets[:] = PDFsets
    paras.update({"PDFSets":c_PDFsets})
    paras.update({"n_pdfs":c_int(n_pdf_sets)})
    # Get the path of the PDF sets
    PDFPath = pdf_cfg["general"]["PDFpath"]
    paras.update({"PDF_path":create_string_buffer(PDFPath)})
    logging.debug('pdf_path: %s',PDFPath)
    # Get the histogram binning
    dummy_binning = []
    logging.debug('binning:')
    for i in mc_cfg["general"]["binning"]:
        dummy_binning.append(float(i))
        logging.debug(i)
    c_binning = (c_double * len(dummy_binning))()
    c_binning[:] = dummy_binning
    paras.update({"n_bins":c_int(len(dummy_binning))})
    paras.update({"binning":c_binning})
    paras.update({"tree_name":create_string_buffer(pdf_cfg["Tree"]["tree_name"])})
    paras.update({"cut_string":create_string_buffer(pdf_cfg["Tree"]["cut_string"])})
    paras.update({"lumi":c_double(float(mc_cfg["general"]["lumi"]))})

    logging.debug('number of bins: %i',len(dummy_binning))
    logging.info('done')
    return paras

