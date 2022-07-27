#!/bin/env python

import physpic

#~ efile=physpic.EventFile("DY_50/all_GenEvent50.txt")
efile=physpic.EventFile("events_after_cuts.txt")

#fileFormat:
#<event>
#pdgid px py pz e
#11 -330.754 1016.55 670.842 1261.99
#0 445.077 -755.916 0 877.213
#1 -134.655 -219.953 198.493 329.008
#</event>
#~ plot=physpic.Plot(show=True)
plot=physpic.Plot()

for event in efile:
    partinfo=[]
    counter=0
    for p in event:
        tester=0
        if (abs(p.pdgId) < 17 and abs(p.pdgId) >10):
            partinfo.append(p.pdgId)
            partinfo.append(abs(p.px))
            partinfo.append(abs(p.py))
            partinfo.append(abs(p.pz))
            counter+=4
        statustester=0
        while (tester < counter):
            if (p.pdgId==1 and (abs(p.px) <partinfo[tester+1]+2  and abs(p.px) >partinfo[tester+1]-2) and (abs(p.py) < partinfo[tester+2]+2 and abs(p.py) >partinfo[tester+2]-2) and (abs(p.pz) < partinfo[tester+3]+2 and abs(p.pz) >partinfo[tester+3]-2)):
                statustester+=1
                print p.px, partinfo[tester+1]
            tester+=4
        if statustester==0:
            plot.addParticle(p)
    plot.drawPolar()
    #~ plot.draw()
    #~ plot.draw3d()
    #~ plot.
    plot.clear()
print "Will crash if polar draw was used"

