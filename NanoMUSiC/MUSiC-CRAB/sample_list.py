from pathlib import Path
import optparse
import tomli
import pprint

usage = "%prog [options]  CROSS_SECTION_FILE"
description = "This script creates the sample list for crab_music using cross section file."

parser = optparse.OptionParser(
    usage=usage, description=description, version="%prog 0"   )
    
parser.add_option(
     "-o",
     "--output_filename",
     action="store",
     default="xSections_list",
     metavar="FILENAME",
     help="Specify a filename for your output file.",
 )
(options, args) = parser.parse_args()                                                                                                                                                                          


def make_sample_list(xSection_file_path):
    input_file=""
    
    try:
        input_file=args[0]
    except:
        input_file=xSection_file_path
    
    xsections = tomli.loads(Path(input_file).read_text(encoding="utf-8"))
    sample_list=[]
    for mName in xsections:
        if "das_name_2016APV" in xsections[mName]: 
            for das_name in xsections[mName]["das_name_2016APV"]:
                             sample_list.append((mName,das_name,"2016APV",False))
        if "das_name_2016" in xsections[mName]:
            for das_name in xsections[mName]["das_name_2016"]:
                             sample_list.append((mName,das_name,"2016",False))
        if "das_name_2017" in xsections[mName]:
            for das_name in xsections[mName]["das_name_2017"]:
                             sample_list.append((mName,das_name,"2017",False))
        if "das_name_2018" in xsections[mName]:
            for das_name in xsections[mName]["das_name_2018"]:
                             sample_list.append((mName,das_name,"2018",False))

        else:
            continue

    return sample_list

def main():

    pprint.pprint(make_sample_list(args[0]))


if __name__ == "__main__":
    main()


