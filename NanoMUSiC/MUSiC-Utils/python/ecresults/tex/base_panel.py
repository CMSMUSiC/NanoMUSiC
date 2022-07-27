import os

class TexBasePanel(object):

    def __init__(self, out, plot_width=1.0):
        self.tex = ""
        self.out = out
        self.plot_width = plot_width

    @classmethod
    def get_outfile_name(cls, out):
        return os.path.join(out, "tex", "panel.tex")

    @property
    def outfile_name(self):
        return self.get_outfile_name(self.out)

    @classmethod
    def get_outfile_name(cls, out):
        return os.path.join(out, "tex", cls.panelname + ".tex")

    def add_plot(self, plot_path, label, caption, comment=None ):
        self.tex += """
\\begin{{figure}}[h]
    \\begin{{center}}
        \\includegraphics[width={width}\\textwidth]{{{path}}}
        \\caption{{{caption}}}
        \\label{{fig:{label}}}
    \\end{{center}}
\\end{{figure}}
               """.format(path=plot_path,
                          caption=caption,
                          label=label,
                          width=self.plot_width,
                          )
        if comment:
            self.tex += comment

    def write_tex(self):
        with open(self.outfile_name, "wb") as f:
            f.write(self.tex)

