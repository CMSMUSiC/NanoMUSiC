def get_year_era(process_name):
    process_name_components = (
        process_name.replace("-HIPM", "")
        .replace("_HIPM", "")
        .replace("-ver1", "")
        .replace("-ver2", "")
        .split("_")
    )
    return process_name_components[-2], process_name_components[-1]
