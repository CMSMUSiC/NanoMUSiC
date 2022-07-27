#!/usr/bin/env python

# system imports
import sys
import os
import argparse
import textwrap

# CMSSW imports
from FWCore.PythonUtilities.LumiList import LumiList

## Compares two JSONs
#
# @param jsons List of exactly two JSONs
# @return string Summary of comparison
def compare(jsons):
    # load files and determine differences
    first = LumiList(filename=jsons[0])
    second = LumiList(filename=jsons[1])
    first_only = first - second
    second_only = second - first

    # prepare result string
    if not first_only and not second_only:
        result = 'Files {0} and {1} are the same'.format(jsons[0], jsons[1])
    else:
        # first JSON
        result = 'Lumi sections only in {0}:\n'.format(jsons[0])
        if first_only:
            result += first_only.__str__()
        else:
            result += 'None'

        # second JSON
        result += '\n\nLumi sections only in {0}:\n'.format(jsons[1])
        if second_only:
            result += second_only.__str__()
        else:
            result += 'None'

    return result


## Determines intersection between all JSONs
#
# @param jsons List of JSONS for which to perform the task
# @return json JSON object containing the result
def intersection(jsons):
    result = LumiList(filename=jsons[0])
    # determine intersection with remaining files
    for json in jsons[1:]:
        result = result & LumiList(filename=json)
    return result


## Gather all unique entries of first JSON, which are not in any of the other JSONs
#
# @param jsons List of JSONs for which to perform the task
# @return json JSON object containing the result
def union(jsons):
    result = LumiList(filename=jsons[0])
    # union with remaining files
    for json in jsons[1:]:
        result = result | LumiList(filename=json)
    return result


## Return entries of first JSON, which are not in any of the other JSONs
#
# @param jsons List of JSONs for which to perform the task
# @return json JSON object containing the result
def difference(jsons):
    result = LumiList(filename=jsons[0])
    # subtract remaining files
    for json in jsons[1:]:
        result = result - LumiList(filename=json)
    return result


## Main function
def main():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description='Tool for comparing and combining JSON files containing lumisections',
        usage='%(prog)s [-h] [-o output.json] (--intersection | --union | \n\
        --difference | --compare) first.json second.json [third.json ...]',
        epilog=textwrap.dedent(
            '''\
            Usual use cases are the following 3 steps
             - combine JSONs: -o union.json -u first.json second.json third.json
             - intersect with golden JSON: -o intersection.json -i union.json golden.json
             - sanity check: -d intersection.json golden.json'''
        )
    )
    parser.add_argument('jsons', nargs='*', metavar='files.json',
                        help='JSON files containing the luminosity sections')
    parser.add_argument('-o', '--output', metavar='output.json',
                        help='If provided, output is written to file instead of printed')
    # mutually exclusive modes
    modes = parser.add_mutually_exclusive_group(required=True)
    modes.add_argument('-i', '--intersection', action='store_true',
                       help='Entries which are in every individual JSON')
    modes.add_argument('-u', '--union', action='store_true',
                       help='All (distinct) entries of all JSONs')
    modes.add_argument('-d', '--difference', action='store_true',
                       help='Entries of first JSON, which are not in any other JSON')
    modes.add_argument('-c', '--compare', action='store_true',
                       help='Show differences between 2 JSONs (result is not JSON format)')
    args = parser.parse_args()

    # ensure that there are at least two provided JSONs
    if len(args.jsons) < 2:
        print 'ERROR: Please provide at least 2 JSON files to work with'
        return 1

    # choose requested mode
    if args.compare:
        # --compare is a special case; does not return JSON format list
        if args.output:
            print 'ERROR: --compare does not return JSON format - Can only print to console'
            return 1
        # make sure there are exactly 2 JSONS
        if len(args.jsons) > 2:
            print 'ERROR: --compare works only with exactly 2 JSON files'
            return 1
        output = compare(args.jsons)
    elif args.intersection:
        output = intersection(args.jsons)
    elif args.union:
        output = union(args.jsons)
    elif args.difference:
        output = difference(args.jsons)

    # write to file or print to screen
    if args.output:
        output.writeJSON(args.output)
    else:
        print output
    # done
    return 0


if __name__ == '__main__':
    sys.exit(main())
