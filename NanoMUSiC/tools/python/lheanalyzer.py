#!/usr/bin/env python
from __future__ import division
import gzip
import ROOT
from math import cos
import sys


class LHEFileFormatError(Exception):
    def __init__(self, filename,line):
        self.line = line
        self.filename = filename
    def __str__(self):
        return repr("Error in LHE File "+self.filename+" in line "+str(self.line))


class Particle:
    """Describes a single particle"""
    def __init__(self,initstr, index):
        """Constuctor call with line from lhe file"""
        ls=initstr.split()
        self.pdgId,self.status,self.mother, self.color,self.momentum,self.lifetime,self.spin=int(ls[0]),int(ls[1]),[int(ls[2]),int(ls[3])],[int(ls[4]),int(ls[5])],[float(ls[6]),float(ls[7]),float(ls[8]),float(ls[9]),float(ls[10])],float(ls[11]),float(ls[12])
        self.index = index
    def getTLorentzVector(self):
        """Returns the ROOT TLorentzVector of the particle"""
        return ROOT.TLorentzVector(self.momentum[0],self.momentum[1],self.momentum[2],self.momentum[3])
    def add(self,p):
        self.energy+=p.energy
        self.px+=p.px
        self.py+=p.py
        self.pz+=p.pz
        for i in range(4):
            self.momentum[i]+=p.momentum[i]
        self.mass = self.momentum[4]
        self.pt = (self.px**2+self.py**2)**0.5
        self.tLorentzVector = self.getTLorentzVector()
        self.eta = self.tLorentzVector.Eta()
        self.phi = self.tLorentzVector.Phi()
    def __repr__(self):
        return "ID:%f  pt:%f  eta:%f  phi:%f  e:%f  m:%f status:%d"%(self.pdgId,self.pt,self.eta,self.phi,self.energy,self.mass,self.status)
    px = property(lambda self: self.momentum[0])
    py = property(lambda self: self.momentum[1])
    pz = property(lambda self: self.momentum[2])
    energy = property(lambda self: self.momentum[3])
    mass = property(lambda self: self.momentum[4])
    pt = property(lambda self: (self.px**2+self.py**2)**0.5)
    tLorentzVector = property(getTLorentzVector)
    eta = property(lambda self: self.tLorentzVector.Eta())
    phi = property(lambda self: self.tLorentzVector.Phi())
    mother1 = property(lambda self: self.mother[0])
    mother2 = property(lambda self: self.mother[1])


class Event:
    def __init__(self,initstr):
        """Constuctor call with line from lhe file"""
        ls=initstr.split()
        try:
            self.nParticles,self.processId,self.weight,self.scale,self.QEDCoupling,self.QCDCoupling=int(ls[0]),int(ls[1]),float(ls[2]),float(ls[3]),float(ls[4]),float(ls[5])
        except:
            self.nParticles,self.processId,self.weight,self.scale,self.QEDCoupling,self.QCDCoupling=int(ls[0]),int(ls[1]),float(ls[2]),float(ls[3]),0.,0.
        self.particles=[]
    def addParticle(self,particle):
        """adds a particle to the event"""
        self.particles.append(particle)


class Process:
    def __init__(self,initstr):
        ls=initstr.split()
        self.crossSection,self.crossSectionUncertainty,self.maxWeight,self.id=float(ls[0]),float(ls[1]),float(ls[2]),int(ls[3])


class LHEFile:
    def __init__(self, filename, forcegzipped=False):
        self.filename=filename
        if filename[-2:]=="gz" or forcegzipped:
            self.fp = gzip.open(filename,"r")
        else:
            self.fp = open(filename,"r")
        self.lineCounter=0
        for line in self.fp:
            self.lineCounter+=1
            if line.strip()=="<init>": break
    def __iter__(self):
        return self
    def next(self):
        for line in self.fp:
            self.lineCounter+=1
            if line[0]!="#":
                return line
                break
        self.fp.close()


class LHEAnalysis:
    def __init__(self, filename):
        self.lhefile=LHEFile(filename)
        line=self.lhefile.next()
        ls = line.split()
        try:
            self.beamId ,self.beamEnergy,self.PDFAuthor,self.PDFSet,self.weightSwitch,self.nProcesses=[int(ls[0]),int(ls[1])],[float(ls[2]),float(ls[3])],[int(ls[4]),int(ls[5])],[int(ls[6]),int(ls[7])],int(ls[8]),int(ls[9])
        except (ValueError, IndexError):
            raise LHEFileFormatError(self.lhefile.filename,self.lhefile.lineCounter)
        self.processes=[]
        for i in range(self.nProcesses):
            line=self.lhefile.next()
            try:
                self.processes.append(Process(line))
            except (ValueError, IndexError):
                print sys.exc_info()[0]
                raise LHEFileFormatError(self.lhefile.filename,self.lhefile.lineCounter)

    def __iter__(self):
        return self
    def next(self):
        beginevent=False
        for line in self.lhefile:
            if line==None:
                raise StopIteration
                return
            if line.strip()=="<event>":
                beginevent=True
                break
        if beginevent is False:
            raise StopIteration
        initline=self.lhefile.next()
        event=Event(initline)
        indexCounter = 1
        for line in self.lhefile:
            if line.strip()=="</event>" or "<" in line.strip():
                break
            if 'wgt' in line:
                continue
            try:
                particle=Particle(line, indexCounter)
                indexCounter += 1
            except (ValueError, IndexError):
                print sys.exc_info()
                raise LHEFileFormatError(self.lhefile.filename,self.lhefile.lineCounter)
            event.addParticle(particle)
        return event
    totalCrossSection = property(lambda self: sum(p.crossSection for p in self.processes))


class LorentzVector:
    def __init__(self,momentum):
        self.px=momentum[0]
        self.py=momentum[1]
        self.pz=momentum[2]
        self.energy=momentum[3]
    def add(self,p):
        self.energy+=p.energy
        self.px+=p.px
        self.py+=p.py
        self.pz+=p.pz
    def invariantMass(self):
        return (self.energy**2-self.px**2-self.py**2-self.pz**2)**0.5
    pt = property(lambda self: (self.px**2+self.py**2)**0.5)


def invariantMass(p0,*particles):
    l0=LorentzVector(p0.momentum)
    if l0.pt==0 or particles[0].pt==0:

        print l0
        print particles
        raw_input("kjnk")
    for p in particles:
        l0.add(LorentzVector(p.momentum))
    return l0.invariantMass()


def transverseMass(p0,p1):
    l0=p0.tLorentzVector
    l1=p1.tLorentzVector
    if l0.Pt()==0 or l1.Pt()==0:
        l0.Print()
        l1.Print()
        raw_input("kjnk")

    return (l0.Pt()**2+l1.Pt()**2-2*l0.Pt()*l1.Pt()*cos(l0.DeltaPhi(l1)))**0.5

def deltaR(p0,p1):
    l0=p0.tLorentzVector
    l1=p1.tLorentzVector
    if l0.Pt()==0 or l1.Pt()==0:
        l0.Print()
        l1.Print()
        raw_input("kjnk")
    return l0.DeltaR(l1)

