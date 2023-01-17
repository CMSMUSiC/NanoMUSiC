def main():
    buffer = {}
    with open('../NanoMUSiC/MUSiC-Configs/MC/scales.txt', 'r') as f_in:
        for line in filter(None, (line.rstrip() for line in f_in)):
            if not line.startswith("#") and not line.startswith(" ") and not line.startswith("Lumi") and line != "":
                config, value = line.split("=")
                process, parameter = config.split(".")
                buffer[process.strip(), parameter.strip()] = value.strip()

    x_sections = {}
    for process_parameter in buffer:
        process, parameter = process_parameter
        x_sections[process] = []

    for process_parameter in buffer:
        process, parameter = process_parameter
        x_sections[process].append((parameter, buffer[process_parameter]))
    

    x_sections_sorted = dict(sorted(x_sections.items()))
    toml_config = ""
    for process in x_sections_sorted:
        toml_config += f"\n[{process}]\n"
        for item in x_sections_sorted[process]:
            parameter, value = item
            if parameter == "XSecOrder" or parameter == "ProcessGroup":
                value = "\"" + value + "\"" 
            toml_config += f"{parameter} = {value}\n"

    print(toml_config)

    

if __name__ == "__main__":
    main()