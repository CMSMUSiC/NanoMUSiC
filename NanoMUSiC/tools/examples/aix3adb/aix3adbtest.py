import aix3adb
import pprint
import string
import random

def id_generator(size=6, chars=string.ascii_uppercase + string.digits):
   return ''.join(random.choice(chars) for x in range(size))
   

def main():
    pp = pprint.PrettyPrinter(indent=4)
    dblink=aix3adb.aix3adb()
    # Ein Datenbank-Objekt erstellen
    skim, sample = dblink.getMCLatestSkimAndSampleByDatasetpath("/DarkMatter_MonoZToLL_A_Mx-50_Mv-50_gDMgQ-1_TuneCUETP8M1_13TeV-madgraph/RunIISpring15DR74-Asympt25ns_MCRUN2_74_V9-v1/MINIAODSIM")
    print skim.__dict__
    liste = len(dblink.searchDataSkimsAndSamples({"owner":"serdweg",'id':'>80'},dict()))
    print liste
    #dblink=aix3adb.aix3adbAuth(username="olschew")
    
    #dblink.searchMCSkimsAndSamples
    raise Exception()
    # Insert a sample
    print "Insert a sample"
    sample0 = aix3adb.MCSample()
    sample0.name=id_generator()
    sample1 = dblink.insertMCSample(sample0)
    print sample1.__dict__

    # Insert a skim
    print "Insert a skim"
    skim0 = aix3adb.MCSkim()
    file1={"path":"blub1","nevents":19999}
    file2={"path":"blub2","nevents":1}
    files=[file1,file2]
    skim0.files = files
    skim0.datasetpath=id_generator()
    skim0.sampleid=sample1.id
    skim1 = dblink.insertMCSkim(skim0)
    print skim1.__dict__

    # Request Sample
    print "Request Sample"
    sample2 = dblink.getMCSample(sample1.name)
    print sample2.__dict__

    # Request Skim
    print "Request Skim"
    skim2 = dblink.getMCSkim(skim1.id)
    print skim2.__dict__

    # Edit sample
    print "Edit sample - set energy to 99"
    sample2.energy="99"
    sample3 = dblink.editMCSample(sample2)
    print sample3.__dict__

    # Edit skim
    print "Edit skim - change files and set to finished"
    skim2.isfinished="1"
    file1={"path":"neuerpath","nevents":5}
    file2={"path":"neuerpath2","nevents":5}
    files=[file1,file2]
    skim2.files = files
    skim3 = dblink.editMCSkim(skim2)
    print skim3.__dict__
    
    # Request Sample and skim with skimid and samplename
    print "Request Sample and skim with skimid and samplename"
    skim4, sample4 = dblink.getMCSkimAndSample(skim3.id, sample3.name)
    print skim4.__dict__, sample4.__dict__

    # Request Sample and skim with samplename only
    print "Request Sample and skim with samplename only"
    skim5, sample5 = dblink.getMCSkimAndSample(None, sample3.name)
    print skim5.__dict__, sample5.__dict__

    # Request Sample and skim with skimid only
    print "Request Sample and skim with skimid only"
    skim6, sample6 = dblink.getMCSkimAndSample(skim3.id, None)
    print skim6.__dict__, sample6.__dict__

    # Request Sample and skim with not skimid and samplename not matching
    print "Request Sample and skim with not skimid and samplename not matching"
    skim7, sample7 = dblink.getMCSkimAndSample(skim3.id, "das ist der falsche sample name")
    print skim7.__dict__, sample7.__dict__
    
    
if __name__=="__main__":
    main()
