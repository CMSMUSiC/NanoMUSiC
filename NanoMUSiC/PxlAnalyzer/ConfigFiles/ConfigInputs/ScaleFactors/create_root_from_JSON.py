#import numpy as np
import ROOT
import json
import os,sys,commands,re,math
import shutil
import fileinput
import optparse
import array
#import csv                                                                                                                                                                  

usage = 'usage: %prog -C cat'
parser = optparse.OptionParser(usage)

parser.add_option('-d', '--indirectory', dest='indir', type='string', default = '.', help="The File you want to convert")

(opt, args) = parser.parse_args()

InDir= opt.indir


print("****** START ******")



### Read the Data from JSON file ####
filename=InDir #Define the File Name

#print(filename)

with open(filename) as json_file:
	data = json.load(json_file)


### Save the data in a dictionary   ####
abseta_pt_path=data['NUM_TrackerMuons_DEN_genTracks']['abseta_pt']
keys=abseta_pt_path.keys()

liste=[]

for k in keys:
	if 'abseta' in k:
		liste.append(abseta_pt_path[k]['pt:[40,60]']['value'])
		


### Create the Histogram ####

xbins=array.array("d",[0.0,13000.0])
ybins=array.array("d",[-2.4,-2.1,-1.2,-0.9,0.0,0.9,1.2,2.1,2.4])


Name=filename.split('abseta')[0]+'eta_pt_Histogram'

hist=ROOT.TH2D('eta',Name,len(xbins)-1,xbins,len(ybins)-1,ybins)


for i in range(len(liste)):
	#print(len(liste)-i)
	hist.SetBinContent(1,i+1,liste[len(liste)-i-1])

for j in range(len(liste)):
	#print(liste[j])
	#print(j)
	hist.SetBinContent(1,len(liste)+j+1,liste[j])

### Creating the Output File ###
outfile=ROOT.TFile(Name+'.root','RECREATE')
hist.Write()
outfile.Close()		



print("***** END *****")
