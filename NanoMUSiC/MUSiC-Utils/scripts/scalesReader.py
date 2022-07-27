import argparse
import csv

import aix3adb


field_map = { "XSec" : "crosssection",
              "FilterEff" : "filterefficiency",
              "kFactor" : "kfactor",
              "XSecOrder" : "crosssection_order",
              "ProcessGroup" :"process_group"}
def compare_db_local(sample_dict):
    if not sample_dict["db"]:
        return "NoDB"
    for key in ("XSec", "FilterEff", "kFactor"):
        try:
            dbval = float(getattr(sample_dict["db"],field_map[key]))
            if (sample_dict[key] - dbval) > 0.:
                return "%s %.4f %.4f" % ( key, sample_dict[key], dbval)
        except:
            return "converting %s %s" % ( key, getattr(sample_dict["db"],field_map[key]) )

    if abs(sample_dict["kFactor"] - 1.) > 0.0001:
        dbval = getattr(sample_dict["db"],"kfactor_order")
    else:
        dbval = getattr(sample_dict["db"],"crosssection_order")
    if not sample_dict["XSecOrder"] == dbval:
        return "%s %s %s" % ( "XSecOrder", sample_dict["XSecOrder"], dbval)
    if sample_dict["ProcessGroup"] != getattr(sample_dict["db"],"process_group"):
        return "%s %s %s" % ( "ProcessGroup", sample_dict["ProcessGroup"], getattr(sample_dict["db"],"process_group"))

    return "OK"
def append_db_samples(scales_dict):

    dblink = aix3adb.createDBlink( "tpook", readOnly= False )
    for sample in scales_dict:
        dbsample = False
        try:
            dbsample = dblink.getMCSample(sample)
        except aix3adb.Aix3adbException:
            print "No sample found for " + sample
        scales_dict[sample]["db"] = dbsample
    return scales_dict

def read_scales(args):
    scales_dict = {}
    with open(args.scales, "r") as sf:
        lines = sf.read().split("\n")
    for line in lines:
        sline = line.split("=")
        if len(sline) < 2:
            continue
        value = sline[-1].strip()
        sline = sline[0].split(".")
        sample = sline[0]
        if len(sline) < 2:
            continue
        key = sline[1].strip()
        if not key in field_map.keys():
            continue


        if key in ("XSec", "FilterEff", "kFactor"):
            value = float(value)
        if not sample in scales_dict:
            scales_dict[sample] = {}
        scales_dict[sample][key] = value
    return scales_dict

def write_csv(scales_dict):
    keys = ("crosssection",
            #~ "crosssection_order",
            "kfactor",
            #~ "kfactor_order",
            "filterefficiency",
            "process_group")
    header = ("sa_name",
              "up_sa_crosssection",
              "up_sa_crosssection_order",
              "up_sa_kfactor",
              "up_sa_kfactor_order",
              "up_sa_filterefficiency",
              "up_sa_process_group",
              "up_sa_crosssection_reference",
             )
    inv_map = {v: k for k, v in field_map.iteritems()}
    with open("scales_dbupdate.csv", "wb") as output_csv:
        writer = csv.writer(output_csv)
        writer.writerow(header)
        for sample, sdict in scales_dict.items():
            if sdict["status"] == "OK" or not sdict['db']:
                continue
            if abs(sdict["kFactor"] - 1.) < 0.0001:
                xs_order = sdict["XSecOrder"]
                kfac_order = getattr(sdict["db"], "kfactor_order")
            else:
                xs_order = getattr(sdict["db"], "crosssection_order")
                kfac_order = sdict["XSecOrder"]

            row = []
            row.append(sample)
            row.append(sdict["XSec"])
            row.append(xs_order)
            row.append(sdict["kFactor"])
            row.append(kfac_order)
            row.append(sdict["FilterEff"])
            row.append(sdict["ProcessGroup"])
            row.append(getattr(sdict["db"], "crosssection_reference"))

            writer.writerow(row)
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("scales", help = "A MUSiC scales file")

    args = parser.parse_args()

    scales_dict = read_scales(args)
    scales_dict = append_db_samples(scales_dict)
    for sample, sdict in scales_dict.items():
        status = compare_db_local(sdict)
        scales_dict[sample]["status"] = status
        print "{0:<25} {1:<40}".format(status, sample)

    write_csv(scales_dict)

if __name__=="__main__":
    main()
