import os
from pydantic import BaseModel
from jinja2 import Template


# Define the models using Pydantic
class FileModel(BaseModel):
    name: str


class DirectoryModel(BaseModel):
    name: str
    files: list[FileModel]
    subdirectories: list[str]


# Jinja2 template as a string
template_string = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Index of {{ directory.name }}</title>
    <style>
        /* Always-On Dark Mode Styles */
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            padding: 20px;
            background-color: #121212;
            color: #ffffff;
        }
        h1 {
            color: #ffffff;
        }
        ul {
            list-style-type: none;
            padding: 0;
        }
        a {
            text-decoration: none;
            color: #80BFFF;
        }
        a:hover {
            color: #FFFFFF;
        }
        # img {
        #     display: block;
        #     margin-top: 10px;
        #     max-width: 100px;
        #     height: auto;
        # }
    </style>
</head>
<body>
    <h1>Index of {{ directory.name }}</h1>
    <ul>
        {# Subdirectories #}
        {% for subdirectory in directory.subdirectories %}
        <li><a href="{{ subdirectory }}/">{{ subdirectory }}/</a></li>
        {% endfor %}

        {# Images #}
        {% for file in directory.files %}
        {% if file.name.endswith('.svg') %}
        <h3>{{ file.name.replace(".svg", "") }}</h3>
        <a href="{{ file.name.replace(".svg", ".pdf") }}">
        <img src="{{ file.name }}" alt="{{ file.name }}" width="1000">
        </a>
        {% endif %}
        {% endfor %}
        
        {# Files #}
        <h2>All files:</h2>
        {% for file in directory.files %}
        <li><a href="{{ file.name }}">{{ file.name }}</a></li>
        {% endfor %}
    </ul>
</body>
</html>
"""


# Function to create the HTML page using Jinja2
def create_html_page(
    directory_data: DirectoryModel, template_string: str, output_path: str
):
    # Create a Jinja2 template from the string
    template = Template(template_string)

    # Render the template with directory data
    html_content = template.render(directory=directory_data)

    # Save the rendered HTML to index.html in the current directory
    with open(output_path, "w") as html_file:
        html_file.write(html_content)


# Function to walk through directories and generate HTML using Pydantic and Jinja2
def make_web_summary(root_directory: str) -> None:
    print("Generating summary for: {}".format(root_directory))
    for current_directory, subdirs, files in os.walk(root_directory):
        # Remove 'index.html' from the files to avoid listing it
        if "index.html" in files:
            files.remove("index.html")

        # Create the directory data using Pydantic models
        directory_data = DirectoryModel(
            name=os.path.basename(current_directory),
            files=[FileModel(name=file) for file in sorted(files)],
            subdirectories=sorted(subdirs),
        )

        # Define the path for the HTML output
        output_path = os.path.join(current_directory, "index.html")

        # Create the HTML page for the directory
        create_html_page(directory_data, template_string, output_path)


if __name__ == "__main__":
    # Usage: Define the root directory to start from
    root_directory = "/path/to/root_directory"  # Replace with your actual directory

    # Create the necessary HTML pages for the directory tree
    make_web_summary(root_directory)
