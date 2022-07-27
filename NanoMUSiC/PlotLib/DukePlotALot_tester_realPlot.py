#!/bin/env python

from DukePlotALot import *
from plotlib import HistStorage,getColorList,getDictValue,HistStorageContainer
import matplotlib.pyplot as plt
from configobj import ConfigObj
try:
    from collections import OrderedDict
except ImportError:
    from ordered import OrderedDict

from rootpy.plotting.views import ScaleView

import style_class as sc

def main():

    basedir="/user/padeken/out/output2014_12_16_14_49/merged"
    lumi=19712

    xs= ConfigObj("/home/home1/institut_3a/padeken/Analysis/SirPlotAlot/xsv100.cfg")


    bghists=HistStorage(xs,lumi,path=basedir)



    bghists.setDataDriven("dataDrivenQCD")

    bglist=OrderedDict()
    bglist["Diboson"]=['WW_TuneZ2star_8TeV_pythia6_tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'ZZ_TuneZ2star_8TeV_pythia6_tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'WZtoAnything_ptmin500_TuneZ2Star_8TeV-pythia6-tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'ZZtoAnything_ptmin500_TuneZ2Star_8TeV-pythia6-tauola_Summer12_DR53X-PU_S10_START53_V7A-v2SIM',
     'WWtoAnything_ptmin500_TuneZ2Star_8TeV-pythia6-tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'WZ_TuneZ2star_8TeV_pythia6_tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM']
    bglist["DY"]=['DYToTauTau_M_10To20_TuneZ2star_8TeV_pythia6_tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'DYToTauTau_M_20_TuneZ2star_8TeV_pythia6_tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'DYToTauTau_M-100to200_TuneZ2Star_8TeV-pythia6-tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'DYToTauTau_M-200to400_TuneZ2Star_8TeV-pythia6-tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'DYToTauTau_M-400to800_TuneZ2Star_8TeV-pythia6-tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'DYJetsToLL_PtZ-50To70_TuneZ2star_8TeV-madgraph-tarball_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'DYJetsToLL_PtZ-70To100_TuneZ2star_8TeV-madgraph-tarball_Summer12_DR53X-PU_S10_START53_V7A-v2SIM',
     'DYJetsToLL_PtZ-100_TuneZ2star_8TeV-madgraph_Summer12_DR53X-PU_S10_START53_V7A-v2SIM',
     'DYJetsToLL_PtZ-180_TuneZ2star_8TeV-madgraph-tarball_Summer12_DR53X-PU_S10_START53_V7C-v1SIM']
    bglist["Top"]=['Tbar_tW-channel-DR_TuneZ2star_8TeV-powheg-tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'T_tW-channel-DR_TuneZ2star_8TeV-powheg-tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'T_t-channel_TuneZ2star_8TeV-powheg-tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'TT_CT10_TuneZ2star_8TeV-powheg-tauola_Summer12_DR53X-PU_S10_START53_V7A-v2SIM',
     'Tbar_s-channel_TuneZ2star_8TeV-powheg-tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'Tbar_t-channel_TuneZ2star_8TeV-powheg-tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'T_s-channel_TuneZ2star_8TeV-powheg-tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM']
    bglist["QCD jet"]=["dataDrivenQCD"]
    bglist["W"]=['WToTauNu_ptmin500_TuneZ2Star_8TeV-pythia6-tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'WToTauNu_ptmin100_ptmax500_TuneZ2Star_8TeV-pythia6-tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'WJetsToLNu_PtW-70To100_TuneZ2star_8TeV-madgraph_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'WToENu_ptmin500_TuneZ2Star_8TeV-pythia6_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'WJetsToLNu_PtW-180_TuneZ2star_8TeV-madgraph-tarball_Summer12_DR53X-PU_S10_START53_V7C-v1SIM',
     'WToMuNu_ptmin500_TuneZ2Star_8TeV-pythia6_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'WToMuNu_ptmin100_ptmax500_TuneZ2Star_8TeV-pythia6_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'WToENu_ptmin100_ptmax500_TuneZ2Star_8TeV-pythia6_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'WJetsToLNu_PtW-100_TuneZ2star_8TeV-madgraph_Summer12_DR53X-PU_S10_START53_V7A-v1SIM',
     'WJetsToLNu_TuneZ2Star_8TeV-madgraph-tarball_Summer12_DR53X-PU_S10_START53_V7A-v2SIM',
     'WJetsToLNu_PtW-50To70_TuneZ2star_8TeV-madgraph_Summer12_DR53X-PU_S10_START53_V7A-v1SIM']

    colorList={}
    colorList["W"]="lightblue"
    colorList["QCD jet"]="darkblue"
    colorList["Top"]="pink"
    colorList["Diboson"]="green"
    colorList["DY"]="red"

    #print bglist
    bghists.addFileList(bglist)

    bghists.views["dataDrivenQCD"]=ScaleView(bghists.files["dataDrivenQCD"],0.63)
    bghists.colorList=colorList

    sghist=HistStorage(xs,lumi,path=basedir)
    #sgName="$\mathsf{W' \, M=2.3\,TeV \cdot 0.02}$"
    sgName="W' M=2.3TeV $\cdot$ 0.02"
    sghist.additionalWeight={"WprimeToTauNu_M-2300_TuneZ2star_8TeV-pythia6-tauola_Summer12_DR53X-PU_S10_START53_V7A-v1SIM":0.02}
    sghist.addAllFiles(tag="WprimeToTauNu_M-2300",joinName=sgName)
    sghist.colorList={sgName :"darkred"}


    dat_hist=HistStorage(xs,lumi,path=basedir,isData=True)
    dat_hist.addFile("allDataMET")



    hists=["byLooseCombinedIsolationDeltaBetaCorr3Hits/h1_5_byLooseCombinedIsolationDeltaBetaCorr3Hits_MT",
    "byLooseCombinedIsolationDeltaBetaCorr3Hits/h1_5_byLooseCombinedIsolationDeltaBetaCorr3Hits_tau_pt",
    "byLooseCombinedIsolationDeltaBetaCorr3Hits/h1_5_byLooseCombinedIsolationDeltaBetaCorr3Hits_met_et",
    ]

    histContainer=HistStorageContainer(bg=bghists,data=dat_hist,sg=sghist)

    binning={
            "_pt":10,
            "_MT":range(200,300,20)+range(300,400,50)+range(400,1600,100)+range(1600,2000,200),
            #"_MT":20,
            "_met_et":30,
    }

    xranges={
            "_pt":[70,1400],
            #"_MT":[200,1500],
            "_met_et":[120,1000],
    }

    cumulative=[]
    #cumulative=["_pt"]

    bghists.initStyle(style="bg")
    sghist.initStyle(style="sg")

    hist_style = sc.style_container(style = 'CMS', useRoot = False)

    hist_style.SetBatchMode(False)

    for hist in hists:
        histContainer.getHist(hist)



        binf=getDictValue(hist,binning)
        if binf is not None:
            if isinstance(binf,list):
                histContainer.rebin(vector=binf)
            else:
                histContainer.rebin(width=binf)
        if getDictValue(hist,cumulative):
            ##histContainer.makeCumulative(width=2)
            histContainer.makeCumulative()

        print histContainer.bg["W"]
        sgPbghist=histContainer.bg.getAllAdded()+histContainer.sg.getAllAdded()
        fakeData=sgPbghist.empty_clone()
        fakeData.SetTitle("pseudo data")
        fakeData.FillRandom(sgPbghist,int(sgPbghist.Integral()))

        test = plotter(hist=histContainer.getBGList(),sig=histContainer.getSGList(),style=hist_style,cmsPositon="upper left")



        #test.Add_data(histContainer.getData())
        test.Add_data(fakeData)
        #test.Add_plot('DiffRatio',pos=1, height=15)
        #test.Add_plot('Signi',pos=2, height=15)
        #test.Add_plot('Diff',pos=2, height=15)
        #test.Add_plot('Ratio',pos=0, height=15)
        #test.Add_error_hist([sys_hist_2,sys_hist], band_center = 'ref')
        #test.ChangeStyle(cms_val=8,lumi_val=lumi)
        #test._cms_val=8
        #test._lumi_val=19700

        mxrange=getDictValue(hist,xranges)
        if mxrange is not None:
            test.Set_axis(xmin=mxrange[0],xmax=mxrange[1],ymin=1.01e-1,ymax=0.5e4)
        else:
            test.Set_axis(ymin=1.01e-1,ymax=.5e4)
        name=hist.replace("/","")

        test.make_plot('%s.pdf'%(name))


    histContainer.getHistFromTree(300,0,3000,"MET /GeV"," ( deltaPhi>2.4&& tau_et/met_et<1.4&& tau_et/met_et>0.7 &&    (mt>200))*ThisWeight","met_et","mtTupleQCD")
    sgPbghist=histContainer.bg.getAllAdded()+histContainer.sg.getAllAdded()
    fakeData=sgPbghist.empty_clone()
    fakeData.SetTitle("pseudo data")
    fakeData.FillRandom(sgPbghist,int(sgPbghist.Integral()))
    test = plotter(hist=histContainer.getBGList(),sig=histContainer.getSGList(),style=hist_style,cmsPositon="upper left")
    test.Add_data(fakeData)
    test.Add_plot('DiffRatio',pos=1, height=15)
    name="metTreePlot"
    test.Set_axis(xmin=140,xmax=1500,ymin=1.01e-1,ymax=0.5e4)

    test.make_plot('%s.pdf'%(name))

    return 42



main()
