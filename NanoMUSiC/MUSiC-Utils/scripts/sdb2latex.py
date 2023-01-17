#!/bin/env python
import os
import argparse
from ectools.register import ecstyle

from dbtools import Sdb
try:
    from table2latex.textable import TexTable, TexTableConfig, escape_latex
except ImportError:
    raise ImportError("table2latex not installed."
            "checkout https://github.com/tobias-pook/table2latex")


def main():

    # Read command line options
    parser = argparse.ArgumentParser("")
    parser.add_argument("-c", "--config", help="table config")
    parser.add_argument("-d", "--database",
                         default="scandb.sqlite",
                         help="Scan database")
    parser.add_argument("-o","--out",
                        default="most_significant",
                        help="Output name without .tex. More info will be added to the name")
    parser.add_argument("--pdf",
                        action='store_true',
                        help="Create a pdf document")
    parser.add_argument("-n",
                        "--nclasses",
                        default=25,
                        type=int,
                        help="Number of most significant claswses to include in table")
    parser.add_argument("--hash", help="Scan hash to use, otherwise all hashes will be evaluated")
    parser.add_argument("--name", help="Scan name to use, otherwise all names will be evaluated")

    args = parser.parse_args()
    if not (args.hash or args.name):
        raise ValueError("Either --hash or --name option is required")
    args.out = args.out.replace(".tex", "")
    # Create table config
    config = TexTableConfig()
    # Set chosen keys from possible keys in imput dict
    chosen_column_keys = [   "event_class",
                             "roi_start",
                             "roi_end",
                             "mc_events",
                             "sys_uncert",
                             "data_events",
                             "p",
                             "p-tilde"
                        ]
    config.add_column_keys(chosen_column_keys)
    # Header replacements for different lines
    header_first_line_map = {
                                'event_class'      :  '\\textbf{Eventclass}',
                                'round'       :  '\\textbf{round}',
                                'roi_start':  '\\textbf{RoI start}',
                                'roi_end'         :  '\\textbf{RoI end}   ',
                                'mc_events'  :  '$\\mathbf{N_{MC}}$      ',
                                'sys_uncert'    :  '\\textbf{Systematatic}    ',
                                'data_events'    :  '$\\mathbf{N_{Data}}$      ',
                                'p' :  '\\textbf{p}   ',
                                'p-tilde' :  '$\\mathbf{\\tilde{p}}$   ',
                            }
    config.add_header_line(header_first_line_map)

    header_second_line_map = { 'sys_uncert' : '\\textbf{Uncertainty}',
                               'roi_start':  '\\textbf{(GeV)}',
                               'roi_end'         :  '\\textbf{(GeV)}',
     }
    config.add_header_line(header_second_line_map)

    def pretty_name(texrow):
        return "$" + ecstyle.latex_ec_name(texrow.event_class,) + "$"



    config.add_column_func(pretty_name, 'event_class')

    # Read data from database with sdb
    sdb = Sdb(args.database)
    for scan_info in sdb.scan_info(name=args.name, hash=args.hash):
        if scan_info.scan_type != "dataScan":
            continue
        print scan_info.hash, args.nclasses
        rows, header = sdb.significant(scan_info.hash,
                                       n=args.nclasses,
                                       mode="median",
                                       return_rows=True)
        row_dicts = [ {h:v for h,v in zip(header, row)} for row in rows]
        filename = "%s_%s.tex" % (os.path.basename(args.out), scan_info.distribution)
        out = os.path.join(os.path.dirname(args.out), filename )
        # create texttable object and fill data
        table = TexTable(config=config, out=out, significant_digits=2)

        for row_dict in row_dicts:
            table.add_row_dict(row_dict)
        # save table
        # create and write the latex table
        table.write_tex_file()
        if args.pdf:
            table.write_pdf_file()

if __name__=="__main__":
    main()


