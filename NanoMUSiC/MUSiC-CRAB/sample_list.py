from pathlib import Path
import tomli


def get_era(process_name, is_data):
    if not is_data:
        return "DUMMY"

    return (
        process_name.replace("-HIPM", "")
        .replace("_HIPM", "")
        .replace("-ver1", "")
        .replace("-ver2", "")
        .split("_")[-1]
    )


def get_extension_suffix(idx: int):
    if idx > 0:
        return f"_ext{idx}"
    return ""


def make_sample_list(xSection_file_path, global_now):
    input_file = ""
    input_file = xSection_file_path

    xsections = tomli.loads(Path(input_file).read_text(encoding="utf-8"))
    sample_list = []
    for process_name in xsections:
        generator_filter_key = ""
        if "generator_filter_key" in xsections[process_name].keys():
            generator_filter_key = xsections[process_name]["generator_filter_key"]
        if "das_name_2016APV" in xsections[process_name]:
            for idx_das_name, das_name in enumerate(
                xsections[process_name]["das_name_2016APV"]
            ):
                sample_list.append(
                    (
                        process_name + get_extension_suffix(idx_das_name),
                        das_name,
                        "2016APV",
                        get_era(process_name, xsections[process_name]["is_data"]),
                        xsections[process_name]["is_data"],
                        generator_filter_key,
                        global_now,
                    )
                )
        if "das_name_2016" in xsections[process_name]:
            for idx_das_name, das_name in enumerate(
                xsections[process_name]["das_name_2016"]
            ):
                sample_list.append(
                    (
                        process_name + get_extension_suffix(idx_das_name),
                        das_name,
                        "2016",
                        get_era(process_name, xsections[process_name]["is_data"]),
                        xsections[process_name]["is_data"],
                        generator_filter_key,
                        global_now,
                    )
                )
        if "das_name_2017" in xsections[process_name]:
            for idx_das_name, das_name in enumerate(
                xsections[process_name]["das_name_2017"]
            ):
                sample_list.append(
                    (
                        process_name + get_extension_suffix(idx_das_name),
                        das_name,
                        "2017",
                        get_era(process_name, xsections[process_name]["is_data"]),
                        xsections[process_name]["is_data"],
                        generator_filter_key,
                        global_now,
                    )
                )
        if "das_name_2018" in xsections[process_name]:
            for idx_das_name, das_name in enumerate(
                xsections[process_name]["das_name_2018"]
            ):
                sample_list.append(
                    (
                        process_name + get_extension_suffix(idx_das_name),
                        das_name,
                        "2018",
                        get_era(process_name, xsections[process_name]["is_data"]),
                        xsections[process_name]["is_data"],
                        generator_filter_key,
                        global_now,
                    )
                )
        else:
            continue

    return sample_list
