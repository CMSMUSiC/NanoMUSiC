from pathlib import Path
import tomli


def make_sample_list(xSection_file_path):
    input_file = ""
    input_file = xSection_file_path

    xsections = tomli.loads(Path(input_file).read_text(encoding="utf-8"))
    sample_list = []
    for mName in xsections:
        if "das_name_2016APV" in xsections[mName]:
            for das_name in xsections[mName]["das_name_2016APV"]:
                sample_list.append((mName, das_name, "2016APV", "_", False))
        if "das_name_2016" in xsections[mName]:
            for das_name in xsections[mName]["das_name_2016"]:
                sample_list.append((mName, das_name, "2016", "_", False))
        if "das_name_2017" in xsections[mName]:
            for das_name in xsections[mName]["das_name_2017"]:
                sample_list.append((mName, das_name, "2017", "_", False))
        if "das_name_2018" in xsections[mName]:
            for das_name in xsections[mName]["das_name_2018"]:
                sample_list.append((mName, das_name, "2018", "_", False))

        else:
            continue

    return sample_list
