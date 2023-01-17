#!/bin/env python
from __future__ import print_function

# stdlib imports
import sys
import os
import argparse

# custom imports
import ROOT

## Container for scale factor root files and their luminosity fractions
class ScaleFactors():
    # Constructor
    #
    # @param self Object pointer
    # @param tfile ROOT file containing the scale factors
    # @param luminosity Run's luminosity for which scale factors are determined
    # @param fraction Fraction of the overall luminosity
    def __init__(self, tfile, luminosity=0.0, fraction=0.0):
        self.tfile = ROOT.TFile(tfile, 'READ')
        self.luminosity = luminosity
        self.fraction = fraction


    ## Human readable string representation of class
    #
    # @param self Object pointer
    # @return string Class object description
    def __repr__(self):
        return 'Luminosity: {0:>6.1f} ({1:.1f}%)  File: {2}'.format(
            self.luminosity,
            self.fraction * 100.0,
            self.tfile.GetName()
        )

    ## Return scaled histogram (detached from file) from given path
    #
    # TGraph (or children of it) are skipped and None is returned. Scaling and
    # merging those would require a lot of individual work. Alternative
    # historgam versions of these distributions are available.
    #
    # @param self Object pointer
    # @param path Path to histogram within ROOT file
    # @return ROOT.TH1 Scaled histogram or None if path leads to TGraph
    def get_hist(self, path):
        hist = self.tfile.Get(path)
        # Skip TGraph(AsymmErrors), as they cannot be combined properly
        if isinstance(hist, ROOT.TGraphAsymmErrors):
            return None
        hist.Scale(self.fraction)
        return hist


## Create ScaleFactors objects from list of pairs
#
# @param pairs List of [tfile_path, luminosity] pairs
# @return list List of ScaleFactors objects
def load(pairs):
    # Calculate integral of luminosities
    integral = 0.0
    for pair in pairs:
        integral += float(pair[1])

    # Create ScaleFactors objects
    sfs = []
    for pair in pairs:
        sfs.append(ScaleFactors(
            pair[0],
            float(pair[1]),
            float(pair[1]) / integral)
        )
        print(sfs[-1])

    return sfs

## Yield keys/paths of root file
#
# Recursively walk through a root file and yield the keys/paths for the
# different objects. Files are returned first, then directories. One can supply
# a path from which to start, which defaults to the root directory of the file.
#
# @param tfile ROOT file from which to traverse
# @param path Starting point from which to traverse the file
# @return string Yields paths to file entries
def get_paths(tfile, path=''):
    folders = []
    # Walk through folder contents
    for key in tfile.GetListOfKeys():
        # Defer traversing folders
        if key.IsFolder():
            folders.append(key)
        else:
            yield path + key.GetName()

    # Finally, traverse folders
    for key in folders:
        name = key.GetName()
        for subkey in get_paths(tfile.Get(name), path + name + '/'):
                yield subkey

## Create new ROOT file containing (weighted) average of historgams
#
# @param sfs List of ScaleFactors objects
# @param output_name Name of the output ROOT file
# @return ROOT.TFile Output TFile with averaged histograms
def average(sfs, output_name):
    # Create new ROOT file
    avg = ROOT.TFile(output_name, 'CREATE')

    # Walk through TFile's contents
    cwd = ''
    for path in get_paths(sfs[0].tfile):
        # Create and switch to respective directory if necessary
        tmp = os.path.dirname(path)
        if cwd != tmp:
            cwd = tmp
            avg.mkdir(cwd)
            avg.cd(cwd)

        # Calculate average of hists and write it
        hist = sfs[0].get_hist(path)
        # Skip unretrievable entries
        if not hist:
            continue
        for sf in sfs[1:]:
            hist.Add(sf.get_hist(path))
        hist.Write()

    # Return output ROOT file
    return avg


## Main function
def main():
    # Command line argument parsing
    parser = argparse.ArgumentParser(
        description='Script to combine scale factors of multiple run periods',
        usage=('%(prog)s [-h] [-o output.root] -p file1.root 1234.56'
               '-p file2.root 7890.12 [-p file3.root ...]')
    )
    parser.add_argument('-p', '--pairs', nargs=2, action='append',
                        metavar='VAL', required=True,
                        help='Pair of scale factor ROOT file and luminosity')
    parser.add_argument('-o', '--output', default='averaged.root',
                        help='Name of the output file')
    args = parser.parse_args()
    # Ensure that there are at least 2 pairs
    if len(args.pairs) < 2:
        print('Need at least 2 pairs to work with')
        return 1

    # Load scale factor file and luminosity pairs
    sfs = load(args.pairs)

    # Create new file with averaged histograms
    avg = average(sfs, args.output)

    # Close output file of averaged histograms
    avg.Close()

    return 0


if __name__ == '__main__':
    sys.exit(main())
