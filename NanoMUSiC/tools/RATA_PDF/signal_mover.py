#!/usr/bin/env python
import os
from ROOT import  gROOT , TCanvas, TPad, gStyle,TFile, gPad, TLegend, TMath, TH1F, TStyle,TColor,TF1
import ROOT as ro
from array import array
from math import sqrt, fabs
import sys
sys.path.append("lib/")
from configobj import ConfigObj

basedir = "/user/erdweg/out/true_res2/"
outdir = "/net/scratch_cms/institut_3a/erdweg/public/Corrected_Results/"

#fileList=[
#"WprimeToMuNu_M-300",
#"WprimeToMuNu_M-500",
#"WprimeToMuNu_M-700",
#"WprimeToMuNu_M-900",
#"WprimeToMuNu_M-1100",
#"WprimeToMuNu_M-1300",
#"WprimeToMuNu_M-1500",
#"WprimeToMuNu_M-1700",
#"WprimeToMuNu_M-1900",
#"WprimeToMuNu_M-2000",
#"WprimeToMuNu_M-2100",
#"WprimeToMuNu_M-2200",
#"WprimeToMuNu_M-2300",
#"WprimeToMuNu_M-2400",
#"WprimeToMuNu_M-2500",
#"WprimeToMuNu_M-2600",
#"WprimeToMuNu_M-2700",
#"WprimeToMuNu_M-2800",
#"WprimeToMuNu_M-2900",
#"WprimeToMuNu_M-3000",
#"WprimeToMuNu_M-3100",
#"WprimeToMuNu_M-3200",
#"WprimeToMuNu_M-3300",
#"WprimeToMuNu_M-3400",
#"WprimeToMuNu_M-3500",
#"WprimeToMuNu_M-3700",
#"WprimeToMuNu_M-4000"
#]

#fileList=[
#"DM_M1_D5",
#"DM_M1_D8",
#"DM_M3_D5",
#"DM_M3_D8",
#"DM_M5_D5",
#"DM_M5_D8",
#"DM_M10_D5",
#"DM_M10_D8",
#"DM_M50_D5",
#"DM_M50_D8",
#"DM_M100_D5",
#"DM_M100_D8",
#"DM_M300_D5",
#"DM_M300_D8",
#"DM_M500_D5",
#"DM_M500_D8",
#"DM_M1000_D5",
#"DM_M1000_D8",
#"DM_M1500_D5",
#"DM_M1500_D8",
#"DM_M2000_D5",
#"DM_M2000_D8"
#]

fileList=[
"wprime_oppsign_mu_300_pt50to100",
"wprime_oppsign_mu_300_pt100to200",
"wprime_oppsign_mu_300_pt200",
"wprime_oppsign_mu_600_pt100to300",
"wprime_oppsign_mu_600_pt300to600",
"wprime_oppsign_mu_600_pt600",
"wprime_oppsign_mu_800_pt100to300",
"wprime_oppsign_mu_800_pt300to600",
"wprime_oppsign_mu_800_pt600",
"wprime_oppsign_mu_1000_pt250to500",
"wprime_oppsign_mu_1000_pt500to1000",
"wprime_oppsign_mu_1000_pt1000",
"wprime_oppsign_mu_1200_pt250to500",
"wprime_oppsign_mu_1200_pt500to1000",
"wprime_oppsign_mu_1200_pt1000",
"wprime_oppsign_mu_1400_pt250to500",
"wprime_oppsign_mu_1400_pt500to1000",
"wprime_oppsign_mu_1400_pt1000",
"wprime_oppsign_mu_1600_pt250to500",
"wprime_oppsign_mu_1600_pt500to1000",
"wprime_oppsign_mu_1600_pt1000",
"wprime_oppsign_mu_1800_pt250to500",
"wprime_oppsign_mu_1800_pt500to1000",
"wprime_oppsign_mu_1800_pt1000",
"wprime_oppsign_mu_2000_pt250to500",
"wprime_oppsign_mu_2000_pt500to1000",
"wprime_oppsign_mu_2000_pt1000",
"wprime_oppsign_mu_2200_pt250to500",
"wprime_oppsign_mu_2200_pt500to1000",
"wprime_oppsign_mu_2200_pt1000",
"wprime_oppsign_mu_2400_pt250to500",
"wprime_oppsign_mu_2400_pt500to1000",
"wprime_oppsign_mu_2400_pt1000",
"wprime_oppsign_mu_2500_pt250to500",
"wprime_oppsign_mu_2500_pt500to1000",
"wprime_oppsign_mu_2500_pt1000",
"wprime_oppsign_mu_2600_pt250to500",
"wprime_oppsign_mu_2600_pt500to1000",
"wprime_oppsign_mu_2600_pt1000",
"wprime_oppsign_mu_2800_pt250to500",
"wprime_oppsign_mu_2800_pt500to1000",
"wprime_oppsign_mu_2800_pt1000",
"wprime_oppsign_mu_3000_pt250to500",
"wprime_oppsign_mu_3000_pt500to1000",
"wprime_oppsign_mu_3000_pt1000",
"wprime_oppsign_mu_3200_pt250to500",
"wprime_oppsign_mu_3200_pt500to1000",
"wprime_oppsign_mu_3200_pt1000",
"wprime_oppsign_mu_3600_pt250to500",
"wprime_oppsign_mu_3600_pt500to1000",
"wprime_oppsign_mu_3600_pt1000",
"wprime_oppsign_mu_4000_pt250to500",
"wprime_oppsign_mu_4000_pt500to1000",
"wprime_oppsign_mu_4000_pt1000",
"wprime_samesign_mu_300_pt50to100",
"wprime_samesign_mu_300_pt100to200",
"wprime_samesign_mu_300_pt200",
"wprime_samesign_mu_600_pt100to300",
"wprime_samesign_mu_600_pt300to600",
"wprime_samesign_mu_600_pt600",
"wprime_samesign_mu_800_pt100to300",
"wprime_samesign_mu_800_pt300to600",
"wprime_samesign_mu_800_pt600",
"wprime_samesign_mu_1000_pt250to500",
"wprime_samesign_mu_1000_pt500to1000",
"wprime_samesign_mu_1000_pt1000",
"wprime_samesign_mu_1200_pt250to500",
"wprime_samesign_mu_1200_pt500to1000",
"wprime_samesign_mu_1200_pt1000",
"wprime_samesign_mu_1400_pt250to500",
"wprime_samesign_mu_1400_pt500to1000",
"wprime_samesign_mu_1400_pt1000",
"wprime_samesign_mu_1600_pt250to500",
"wprime_samesign_mu_1600_pt500to1000",
"wprime_samesign_mu_1600_pt1000",
"wprime_samesign_mu_1800_pt250to500",
"wprime_samesign_mu_1800_pt500to1000",
"wprime_samesign_mu_1800_pt1000",
"wprime_samesign_mu_2000_pt250to500",
"wprime_samesign_mu_2000_pt500to1000",
"wprime_samesign_mu_2000_pt1000",
"wprime_samesign_mu_2200_pt250to500",
"wprime_samesign_mu_2200_pt500to1000",
"wprime_samesign_mu_2200_pt1000",
"wprime_samesign_mu_2400_pt250to500",
"wprime_samesign_mu_2400_pt500to1000",
"wprime_samesign_mu_2400_pt1000",
"wprime_samesign_mu_2500_pt250to500",
"wprime_samesign_mu_2500_pt500to1000",
"wprime_samesign_mu_2500_pt1000",
"wprime_samesign_mu_2600_pt250to500",
"wprime_samesign_mu_2600_pt500to1000",
"wprime_samesign_mu_2600_pt1000",
"wprime_samesign_mu_2800_pt250to500",
"wprime_samesign_mu_2800_pt500to1000",
"wprime_samesign_mu_2800_pt1000",
"wprime_samesign_mu_3000_pt250to500",
"wprime_samesign_mu_3000_pt500to1000",
"wprime_samesign_mu_3000_pt1000",
"wprime_samesign_mu_3200_pt250to500",
"wprime_samesign_mu_3200_pt500to1000",
"wprime_samesign_mu_3200_pt1000",
"wprime_samesign_mu_3600_pt250to500",
"wprime_samesign_mu_3600_pt500to1000",
"wprime_samesign_mu_3600_pt1000",
"wprime_samesign_mu_4000_pt250to500",
"wprime_samesign_mu_4000_pt500to1000",
"wprime_samesign_mu_4000_pt1000"
]

histList1=["h1_mtsys_mures_scaled",
"h1_mtsys_muscaleup_scaled",
"h1_mtsys_muscaledown_scaled",
"h1_mtsys_MetUnclusteredEnDown_scaled",
"h1_mtsys_MetUnclusteredEnUp_scaled",
"h1_mtsys_MetJetEnDown_scaled",
"h1_mtsys_MetJetEnUp_scaled",
"h1_mtsys_MetJetResDown_scaled",
"h1_mtsys_MetJetResUp_scaled",
"h1_mtsys_MetTauEnDown_scaled",
"h1_mtsys_MetTauEnUp_scaled",
"h1_mtsys_MetElectronEnDown_scaled",
"h1_mtsys_MetElectronEnUp_scaled",
"h1_mtsys_pileup_scaled",
"h1_mtsys_kfacdown_scaled",
"h1_mtsys_kfacup_scaled"
]
histList2=[
"CT10_up",
"CT10_down",
"MSTW_up",
"MSTW_down",
"NNPDF_up",
"NNPDF_down"
]

try:
    xs_cfg = ConfigObj("xs.cfg")
except IOError as e:
    print("There was a error reading the File "+options.XsCfg)
    print(e)
    exit()
lumi = 19700.
print("Reading Files:")
bgFiles={}
for bg in fileList:
    print "\033[1;36m",bg,"\033[0m"
    bgFile=basedir+bg+".root"
    if os.path.exists(bgFile):
        #print "opening:",bgFile
        bgFiles.update({bg:TFile("file://"+bgFile,"READ")})
    else:
        print("\033[1;31m\n No file: "+bgFile+"\033[0m")
        exit()
    out_file = TFile(outdir+bg+".root","RECREATE")
    counter= bgFiles[bg].Get("h_counters")
    bgtree= bgFiles[bg].Get("mtntuple")
    print(bgtree.GetEntries())
    out_file.cd()
    newtree = bgtree.CloneTree()
    newtree.Write()
    file1 = bgFiles[bg].GetDirectory("results/")
    #file2 = bgFiles[bg].GetDirectory("pdf/")
    bgmain= file1.Get("h1_scaled_mt")
    scf = lumi * float(xs_cfg[bg]["xs"]) / float(counter.GetBinContent(1))
    #print(scf)
    #raw_input("test")
    counter.Write()
    bgmain.Write()
    for histname in histList1:
        try:
            bgtmp=file1.Get(histname)
            #print "opening:",histname,bgtmp.GetEntries()
        except:
            print("\033[1;31m"+histname+ " not in "+ bg+"\033[0m")
            continue
        bgtmp.Write(histname)
    for histname in histList2:
        try:
            bgtmp=bgFiles[bg].Get(histname)
            bgtmp.GetEntries()
            #print "opening:",histname,bgtmp.GetEntries()
        except:
            print("\033[1;31m"+histname+ " not in "+ bg+"\033[0m")
            continue
        bgtmp.Scale(1./scf)
        bgtmp.Write("h1_mtsys_pdf_"+histname)


