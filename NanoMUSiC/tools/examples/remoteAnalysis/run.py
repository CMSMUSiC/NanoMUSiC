#! /usr/bin/env python2
from __future__ import division
import ROOT
import sys
import random
def main():
    ROOT.TDirectory.AddDirectory(ROOT.kFALSE) #no automatic assignment of TObjects to TFiles
    ROOT.TH1.AddDirectory(ROOT.kFALSE) #same again
    f = open('testdir/inputfile.txt', 'r')
    print "Content of inputfile:", f.read()
    print "argv:", sys.argv
    f.close()
    h=ROOT.TH1F("myhist","",100,0,100)
    x = random.gauss(50, 10)
    h.Fill(x)
    outfile=ROOT.TFile("outputfile.root","RECREATE")
    outfile.WriteTObject(h)

if __name__=="__main__":
    main()