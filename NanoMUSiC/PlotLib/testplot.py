#!/bin/env python

from lib.plotlib import Hist,Plot
from lib.configobj import ConfigObj
import ROOT
import sys,os

baseDir="/home/lo/rwth/user/out/output2014_10_8_19_12/merged/"




def main():
    ROOT.gROOT.Reset()
    #ROOT.SetMemoryPolicy( ROOT.kMemoryHeuristics )
    #ROOT.SetMemoryPolicy( ROOT.kMemoryStrict )
    #ROOT.gROOT.SetBatch()



    configFile="plotConfig.cfg"
    if len(sys.argv)<2:
        print("No config specified, will use "+configFile)
        print("If you want an other call "+sys.argv[0]+" yourConfig.cfg")
    else:
        configFile=sys.argv[1]

    try:
        cfg = ConfigObj(configFile)
    except IOError as e:
        print("There was a error reading the File "+ configFile)
        print e
        exit()



    bgFileList=readFiles(cfg["backgrounds"])
    sgFileList=readFiles(cfg["signal"])
    dataFileList=readFiles([cfg["data"]["file"]])

    plot=Plot("byMediumIsolationMVA3newDMwLT/h1_5_byMediumIsolationMVA3newDMwLT_MT")


    bghists=[]
    sghists=[]
    datahists=[]
    for name,f in bgFileList:

        plot.addBG(getHists(f,plot.name,histname=name),name)
        plot.bgDict[name].fcolor=int(cfg["backgrounds"][str(f.GetName()).split("/")[-1].split(".")[0]]["fcolor"])
        plot.bgDict[name].lcolor=int(cfg["backgrounds"][str(f.GetName()).split("/")[-1].split(".")[0]]["lcolor"])
    for name,f in sgFileList:
        plot.addSG(getHists(f,plot.name))
    for name,f in dataFileList:
        plot.addData(getHists(f,plot.name))
    plot.rebin=10
    plot.applyStyle()
    plot.draw()


def readFiles(flist):
    returnList=[]
    for f in flist:
        if ".root" not in f:
            f+=".root"
        fullname=baseDir+f

        if os.path.exists(fullname):
            returnList.append([f,ROOT.TFile(fullname,"READ")])
        else:
            print "no file "+fullname
    return returnList


def getHists( file, hist, histname="" ):
    if histname="":
        histname=hist
    file.cd()
    try:
        counter=file.Get("h_counters")
        Nev=max(counter.GetBinContent(1),counter.GetBinContent(2))
        weightedN=counter.GetBinContent(counter.FindBin(4))
        del counter
    except:
        print("[Info] If you want to use Nev from the root file store 'h_counters'")
        print(file.GetName())
        Nev=-1
    htmp=Hist(file.Get(hist),name=histname)
    if Nev>=0:
        htmp.setEventNumber(Nev)
    return htmp

if __name__ == '__main__':
  main()
