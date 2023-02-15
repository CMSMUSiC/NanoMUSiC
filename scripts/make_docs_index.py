import re  

def get_file_content(input_file):
    with open(input_file, 'r') as file:
        return input_file_str = file.read()

def main():
    print(re.sub(r"(?<!\\){[^}]*(?<!\\)}", , get_file_content(input_file)))

if __name__ == "__main___":
    main()