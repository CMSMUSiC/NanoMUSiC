#!/bin/env python

import gzip
import os
#import hashlib as hashlib
import hashlib
import matplotlib.pyplot as plt
from numpy import sign,pi
import ROOT


from matplotlib.patches import FancyArrowPatch
from mpl_toolkits.mplot3d import proj3d
from io import open


##helper classes
class Arrow3D(FancyArrowPatch):
    def __init__(self, xs, ys, zs, *args, **kwargs):
        FancyArrowPatch.__init__(self, (0,0), (0,0), *args, **kwargs)
        self._verts3d = xs, ys, zs

    def draw(self, renderer):
        xs3d, ys3d, zs3d = self._verts3d
        xs, ys, zs = proj3d.proj_transform(xs3d, ys3d, zs3d, renderer.M)
        self.set_positions((xs[0],ys[0]),(xs[1],ys[1]))
        FancyArrowPatch.draw(self, renderer)

class EventFileFormatError(Exception):
    def __init__(self, filename,line):
        self.line = line
        self.filename = filename
    def __str__(self):
        return repr(u"Error in File "+self.filename+u" in line "+unicode(self.line))


#core particle class
class Particle(object):
    #"""Describes a single particle"""
    #def __init__(self,initstr):
        #"""Constuctor call with line from lhe file"""
        #if type(initsr)!=str:
            #raise ValueError('input is not a string')

    def __init__(self,pdgId,px=0,py=0,pz=0,energy=0):
    #def __init__(self,pdgid,px,py,pz,energy):
        u"""Constuctor call with line from lhe file"""
        if type(pdgId)==unicode:

            pdgId=pdgId.replace(u"b'",u"").replace(u"\\n'",u"")
            ls=pdgId.split()
            #standard lhe input:
            if len(ls)==13:
                self.pdgId,self.status,self.mother, self.color,self.momentum,self.lifetime,self.spin=abs(int(ls[0])),int(ls[1]),[int(ls[2]),int(ls[3])],[int(ls[4]),int(ls[5])],[float(ls[6]),float(ls[7]),float(ls[8]),float(ls[9]),float(ls[10])],float(ls[11]),float(ls[12])
            #if len(ls)==14:
                #self.pdgId,self.status,self.mother, self.color,self.momentum,self.lifetime,self.spin=abs(int(ls[1])),int(ls[2]),[int(ls[3]),int(ls[4])],[int(ls[5]),int(ls[6])],[float(ls[7]),float(ls[8]),float(ls[9]),float(ls[10]),float(ls[11])],float(ls[12]),float(ls[13])
            #pdg and 4vector input
            elif len(ls)==5:
                self.pdgId,self.momentum=int(ls[0]),[float(ls[1]),float(ls[2]),float(ls[3]),float(ls[4])]
            elif len(ls)==6:
                self=None
        else:
            self.pdgId=pdgId
            self.momentum=[px,py,pz,energy]
    def getTLorentzVector(self):
        u"""Returns the ROOT TLorentzVector of the particle"""
        return ROOT.TLorentzVector(self.momentum[0],self.momentum[1],self.momentum[2],self.momentum[3])
    def deltaR(self,other):
        return self.tLorentzVector.DeltaR(other.tLorentzVector)
    def __repr__(self):
        return u"ID:%f  pt:%f  eta:%f  phi:%f  e:%f  m:%f"%(self.pdgId,self.pt,self.eta,self.phi,self.energy,self.mass)
    def __str__(self):
        return u"ID:%f  pt:%f  eta:%f  phi:%f  e:%f  m:%f"%(self.pdgId,self.pt,self.eta,self.phi,self.energy,self.mass)

    px = property(lambda self: self.momentum[0])
    py = property(lambda self: self.momentum[1])
    pz = property(lambda self: self.momentum[2])
    energy = property(lambda self: self.momentum[3])
    mass = property(lambda self: self.energy**2 -(self.px**2+self.py**2+self.pz**2))
    pt = property(lambda self: (self.px**2+self.py**2)**0.5)
    tLorentzVector = property(getTLorentzVector)
    eta = property(lambda self: self.tLorentzVector.Eta())
    phi = property(lambda self: self.tLorentzVector.Phi())

class EventFile(object):
    def __init__(self,filename, forcegzipped=False):
        self.filename=filename
        if filename[-2:]==u"gz" or forcegzipped:
            self.fp = gzip.open(filename,u"r")
        else:
            self.fp = open(filename,u"r")
        self.beginevent=False
        self.linecounter=0
        for line in self.fp:
            self.linecounter+=1
            if type(line)!=unicode:
                line=unicode(line)
            if u"<event>" in line.strip():
                self.beginevent=True
                break
    def __iter__(self):
        return self
    def next(self):
        for line in self.fp:
            self.linecounter+=1
            if line==None:
                raise StopIteration
                return
            if type(line)!=unicode:
                line=unicode(line)
            if line.strip()==u"<event>" or u"<event>" in line:
                self.beginevent=True
                break
        event=[]
        for line in self.fp:
            self.linecounter+=1
            if type(line)!=unicode:
                line=unicode(line)
            if line[0]==u"#":
                continue
            if line.strip()==u"</event>" or u"</event>" in line:
                self.beginevent=False
                break
            try:
                if len(line.split())==7:
                    continue
                particle=Particle(line)
                if hasattr(particle,u"status"):
                    if particle.status==-1:
                        continue
            except (ValueError, IndexError):
                import sys
                print sys.exc_info()
                raise EventFileFormatError(self.filename,self.linecounter)
            event.append(particle)
        if len(event)==0:
            raise StopIteration
        return event
        self.fp.close()


##plotter class
class Plot(object):
    def __init__(self,show=False):
        self.show=show

        self.particles=[]
        #colors try to use pdgids with the exeptions:
        # jets=1
        # met=0
        self.styles={
                    11:{u"color":u"blue",u"label":u"e",u"linestyle":u"solid"},
                    13:{u"color":u"lightgreen",u"label":u"$\\mu$",u"linestyle":u"solid"},
                    15:{u"color":u"orange",u"label":u"$\\tau$",u"linestyle":u"solid"},
                    0:{u"color":u"red",u"label":u"$E_T^{miss}$",u"linestyle":u"dashed"},
                    1:{u"color":u"darkgreen",u"label":u"jet",u"linestyle":u"dashdot"},
                    22:{u"color":u"lightblue",u"label":u"$\gamma$",u"linestyle":u"dotted"},
                    #gen infos:
                    12:{u"color":u"red",u"label":u"$\\nu_{e}$",u"linestyle":u"dashed"},
                    14:{u"color":u"red",u"label":u"$\\nu_{\\mu}$",u"linestyle":u"dashed"},
                    16:{u"color":u"red",u"label":u"$\\nu_{\\tau}$",u"linestyle":u"dashed"},
                    2:{u"color":u"darkgreen",u"label":u"d",u"linestyle":u"dashdot"},
                    3:{u"color":u"darkgreen",u"label":u"c",u"linestyle":u"dashdot"},
                    4:{u"color":u"darkgreen",u"label":u"s",u"linestyle":u"dashdot"},
                    5:{u"color":u"darkgreen",u"label":u"t",u"linestyle":u"dashdot"},
                    6:{u"color":u"darkgreen",u"label":u"b",u"linestyle":u"dashdot"},
                    21:{u"color":u"darkgreen",u"label":u"g",u"linestyle":u"dashdot"},
                    24:{u"color":u"darkblue",u"label":u"w",u"linestyle":u"dashdot"},
                    }
        if not os.path.exists("events/"):
            os.mkdir("events/")



    def addParticle(self,particle):
        if type(particle)==type(Particle(0,0,0,0,0)):
            self.particles.append(particle)
        if type(particle)==unicode:
            self.particles.append(Particle(particle))
        if type(particle)==list:
            if len(particle)==5:
                self.particles.append(Particle(particle[0],particle[1],particle[2],particle[3],particle[4]))
    def removeParticle(self,part):
        self.particles.remove(part)

    def drawPolar(self):
        self.fig = plt.figure()
        self.ax = self.fig.add_subplot(111, polar=True)

        xT=plt.xticks()[0]
        xL=[u'0',ur'$\frac{\pi}{4}$',ur'$\frac{\pi}{2}$',ur'$\frac{3\pi}{4}$',ur'$\pi$',ur'$\frac{5\pi}{4}$',ur'$\frac{3\pi}{2}$',ur'$\frac{7\pi}{4}$']
        plt.xticks(xT, xL)
        ptmax=0
        offset=0.1
        self.particles=sorted(self.particles, key=lambda p: p.pt)
        for p in self.particles[::-1]:
            if p.pdgId in self.styles:
                style=self.styles[p.pdgId]
            else:
                style={u"color":u"grey",u"label":unicode(p.pdgId),u"linestyle":u"dotted"}
            self.ax.annotate(u"",
                  xy=(p.phi, p.pt), xycoords=u'data',
                  xytext=(0., 0.), textcoords=u'data',
                  size=20, va=u"center", ha=u"center",
                  arrowprops=dict(arrowstyle=u"-|>",**style)
                  )
            ptmax=max(ptmax,p.pt)
            signphi=-1.*sign(p.phi)
            self.ax.text(p.phi+signphi*0.2, p.pt+sign(p.pt)*ptmax*offset, u"{:s}: $p_T$={:.2f} GeV".format(style[u"label"],p.pt))
        self.ax.set_xlim(xmin=0.,xmax=1.2*ptmax)
        self.ax.set_ylim(ymin=0.,ymax=1.2*ptmax)
        pstr=[str(i) for i in self.particles]
        pstr=" ".join(pstr)

        plt.savefig(u"events/event_polar_"+hashlib.sha1(pstr).hexdigest()+".png",dpi=300, facecolor = "w",transparent=True)
        if self.show:
            plt.show()

    def draw(self):
        self.fig = plt.figure()
        self.ax = self.fig.add_subplot(111)
        xlim=[0,0]
        ylim=[0,0]
        offset=0.1
        self.particles=sorted(self.particles, key=lambda p: max(abs(p.px),abs(p.py)))
        for p in self.particles[::-1]:
            if p.pdgId in self.styles:
                style=self.styles[p.pdgId]
            else:
                style={u"color":u"grey",u"label":unicode(p.pdgId),u"linestyle":u"dotted"}
            self.ax.annotate(u"",
                  xy=(p.px, p.py), xycoords=u'data',
                  xytext=(0., 0.), textcoords=u'data',
                  size=20, va=u"center", ha=u"center",
                  arrowprops=dict(arrowstyle=u"-|>",**style),
                  )
            xlim=min(xlim[0],-abs(p.px),-abs(p.py)),max(xlim[1],abs(p.px),abs(p.py))
            self.ax.text(p.px+ sign(p.px)*  xlim[1]*offset, p.py+sign(p.py)*xlim[1]*offset, u"{:s}: $p_T$={:.2f} GeV".format(style[u"label"],p.pt))
        self.ax.set_xlim(xmin=xlim[0]*1.2,xmax=xlim[1]*1.2)
        self.ax.set_ylim(ymin=xlim[0]*1.2,ymax=xlim[1]*1.2)
        self.ax.set_xlabel(u'$p_{x}$ GeV')
        self.ax.set_ylabel(u'$p_{y}$ GeV')
        pstr=[str(i) for i in self.particles]
        pstr=" ".join(pstr)
        plt.savefig(u"events/event_"+hashlib.sha1(pstr).hexdigest()+".png",dpi=300, facecolor = "w",transparent=True)
        if self.show:
            plt.show()

    def draw3d(self):
        self.fig = plt.figure()
        self.ax = self.fig.add_subplot(111, projection=u'3d')
        xlim=[0,0]
        ylim=[0,0]
        offset=0.1
        self.particles=sorted(self.particles, key=lambda p: max(abs(p.px),abs(p.py),abs(p.pz)))
        for p in self.particles[::-1]:
            if p.pdgId in self.styles:
                style=self.styles[p.pdgId]
            else:
                style={u"color":u"grey",u"label":u"p",u"linestyle":u"dotted"}
            a = Arrow3D([0,p.pz],[0,p.py],[0,p.px], mutation_scale=20, lw=1, arrowstyle=u"-|>", **style)
            self.ax.add_artist(a)
            xlim=min(xlim[0],-abs(p.px),-abs(p.py),-abs(p.pz)),max(xlim[1],abs(p.px),abs(p.py),abs(p.pz))
            self.ax.text(p.px+ sign(p.px)*  xlim[1]*offset, p.py+sign(p.py)*xlim[1]*offset, p.pz+sign(p.pz)*xlim[1]*offset, u"{:s}: $p_T$={:.2f} GeV".format(style[u"label"],p.pt))

        self.ax.set_xlim(xmin=xlim[0]*1.2,xmax=xlim[1]*1.2)
        self.ax.set_ylim(ymin=xlim[0]*1.2,ymax=xlim[1]*1.2)
        self.ax.set_zlim(zmin=xlim[0]*1.2,zmax=xlim[1]*1.2)
        self.ax.set_zlabel(u'$p_{x}$ GeV')
        self.ax.set_ylabel(u'$p_{y}$ GeV')
        self.ax.set_xlabel(u'$p_{z}$ GeV')
        pstr=[str(i) for i in self.particles]
        pstr=" ".join(pstr)
        plt.savefig(u"events/event_3d_"+hashlib.sha1(pstr).hexdigest()+".png",dpi=300, facecolor = "w",transparent=True)
        if self.show:
            plt.show()

    def clear(self):
        self.particles=[]
        plt.close()
        del self.fig










