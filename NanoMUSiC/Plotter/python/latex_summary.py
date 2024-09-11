import sys
from pathlib import Path
import os
from jinja2 import Template
from numpy import full
from pydantic import BaseModel
from typing import List

from tools import to_root_latex


# Define Pydantic models for Figure and Page
class Figure(BaseModel):
    path: str
    caption: str
    label: str


class SummaryPage(BaseModel):
    title: str
    raw_title: str
    figures: dict[str, Figure]
    main_caption: str


class LaTeXTemplate:
    header = r"""
\documentclass[a4paper]{article}
\usepackage{graphicx}
\usepackage[caption=false]{subfig}
\usepackage{hyperref}  
\usepackage[margin=1in]{geometry}  

\begin{document}

\tableofcontents  

\clearpage

"""

    body = r"""

{% for page in pages %}
    \section{ Event class: {{ page.title }} }
    \begin{figure}[htbp]
        \centering
             \subfloat[{{ page.figures.counts.caption }}]{
                \includegraphics[width=0.47\textwidth]{ {{ page.figures.counts.path }} }
                \label{fig:{{ page.figures.counts.label }}}
             }
            \hfill
             \subfloat[{{page.figures.invariant_mass.caption}}]{
                \includegraphics[width=0.47\textwidth]{ {{ page.figures.invariant_mass.path }} }
                \label{fig:{{ page.figures.invariant_mass.label }}}
             }

            \vspace{0.5cm}

             \subfloat[{{page.figures.sum_pt.caption}}]{
                \includegraphics[width=0.47\textwidth]{ {{ page.figures.sum_pt.path }} }
                \label{fig:{{ page.figures.sum_pt.label }}}
             }{% if 'met' in page.figures %}
            \hfill
             \subfloat[{{page.figures.met.caption}}]{
                \includegraphics[width=0.47\textwidth]{ {{ page.figures.met.path }} }
                \label{fig:{{ page.figures.met.label }}}
             }
            {% endif %}
            
             
            \vspace{0.5cm}

        \caption{ {{ page.main_caption }} }
        \label{page.raw_title}
    \end{figure}
    {% if not loop.last %}\clearpage{% endif %}
{% endfor %}
"""

    footer = r"""
\end{document}
"""


def get_distribution_name(file_name: str) -> str:
    distribution_name = None
    if "counts" in file_name:
        distribution_name = "counts"
    if "sum_pt" in file_name:
        distribution_name = "sum_pt"
    if "invariant_mass" in file_name:
        distribution_name = "invariant_mass"
    if "met" in file_name:
        distribution_name = "met"

    if distribution_name:
        return distribution_name

    print("ERROR: Could not get distribution name.", file=sys.stderr)
    sys.exit(-1)


def make_caption(distribution_name: str, has_met: bool) -> str:
    if distribution_name == "counts":
        return r"Total counts"
    if distribution_name == "sum_pt":
        return r"$S_{T}$"
    if distribution_name == "invariant_mass":
        return r"$M_{T}$" if has_met else r"$M_{inv}$"
    if distribution_name == "met":
        return r"$p_{T}^{miss}$"

    print("ERROR: Invalid distribution type.", file=sys.stderr)
    sys.exit(-1)


def render_latex(
    input_dir: str = "classification_plots",
    output_dir: str = "classification_plots",
    do_compile: bool = True,
):
    pages = []

    path = Path(input_dir)
    for subdir in path.iterdir():
        if subdir.is_dir() and "EC_" in str(subdir):
            figures = {}
            for root, _, files in os.walk(subdir):
                for file_name in files:
                    if str(file_name).endswith(".pdf") and "Run2" in str(file_name):
                        full_path = os.path.join(root, file_name)
                        distribution_name = get_distribution_name(str(file_name))
                        label = os.path.splitext(file_name)[0] + "_" + distribution_name

                        figures[distribution_name] = Figure(
                            path=full_path.replace(output_dir + "/", ""),
                            caption=make_caption(distribution_name, "MET" in full_path),
                            label=label,
                        )

            if len(figures) < 3:
                print(
                    "ERROR: Not all figures were found: Page: {} - Figures: {}.".format(
                        str(subdir), figures
                    ),
                    file=sys.stderr,
                )
                # sys.exit(-1)
            else:
                pages.append(
                    SummaryPage(
                        title=to_root_latex(class_name=str(subdir), latex_syntax=True),
                        raw_title=str(subdir),
                        figures=figures,
                        main_caption="Kinematic distributions for event class {} Red lines represent the selected Region of Interest (RoI). When no RoI information is present in the plot, it means the Scan have failed (no Region passed the quality control requirements.)".format(
                            to_root_latex(class_name=str(subdir), latex_syntax=True)
                        ),
                    )
                )

    pages = sorted(pages, key=lambda p: p.raw_title)
    output_tex_path = "{}/distributions_summary.tex".format(output_dir)

    # Render LaTeX content from template and data
    template = Template(LaTeXTemplate.body)
    rendered_tex = template.render(pages=pages)

    # Write rendered content to .tex file
    with open(output_tex_path, "w") as f:
        f.write(rendered_tex)

    with open("{}/full_distributions_summary.tex".format(output_dir), "w") as f:
        f.write(LaTeXTemplate.header + rendered_tex + LaTeXTemplate.footer)

    if do_compile:
        os.system("rm -rf {}/full_distributions_summary.pdf".format(output_dir))

        print("Rendering pdf ...")
        os.system(
            f"cd {output_dir} && lualatex  -interaction=nonstopmode full_distributions_summary.tex > /dev/null 2>&1"
        )
        os.system(
            f"cd {output_dir} && lualatex  -interaction=nonstopmode full_distributions_summary.tex > /dev/null 2>&1"
        )
        res = os.system(
            f"cd {output_dir} && lualatex  -interaction=nonstopmode full_distributions_summary.tex > /dev/null 2>&1"
        )
        if res != 0:
            print(f"ERROR: Could not compile TeX document document.")
            sys.exit(-1)

        print("... done.")
