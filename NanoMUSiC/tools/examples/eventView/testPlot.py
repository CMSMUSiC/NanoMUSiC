#!/bin/env python

import physpic

efile=physpic.EventFile("events_after_cuts.txt")

#fileFormat:
#<event>
#pdgid px py pz e
#11 -330.754 1016.55 670.842 1261.99
#0 445.077 -755.916 0 877.213
#1 -134.655 -219.953 198.493 329.008
#</event>
plot=physpic.Plot()

for event in efile:
    for p in event:
        plot.addParticle(p)
    plot.drawPolar()
    #plot.draw()
    #plot.draw3d()
    plot.clear()
print "Will crash if polar draw was used"

