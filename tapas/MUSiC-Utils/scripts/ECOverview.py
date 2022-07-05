#!/bin/env python
""" This program reads scanner output jsons and saves them back into EventClasses
    or shows a summary of scan results in an EventClass instead        """
from __future__ import print_function

import argparse

from ectools.register import ecroot

def main():
    """main function of the project."""
    conf = parse_arguments()
    ecroot.ec_loop( conf, print_summary )

def print_summary( conf, i=0, N=0, data_ec=None, mc_ec=None, cparser=None, **kwargs ):
    # the_ec points to at least one existing TEventClass, thus can be
    # used to obtain the event class name, etc.
    the_ec = data_ec if data_ec else mc_ec

    name = the_ec.GetName()

    weighted_events = ecroot.total_event_yield(the_ec, "SumPt").Integral()

    line = "{:>50s} {:>20g}".format(name, weighted_events)

    print(line)

def parse_arguments():
    """Argument parsing. Configuration is returned as namespace object."""

    parser = argparse.ArgumentParser( description="Create MUSiC plots from ROOT files." )

    general_group = parser.add_argument_group(title="General options")
    ecroot.add_ec_standard_options( general_group )

    args = parser.parse_args()

    return args

if __name__=="__main__":
    main()
