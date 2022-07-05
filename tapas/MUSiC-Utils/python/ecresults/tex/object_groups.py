import os
from ectools.register import ecroot

from .base_panel import TexBasePanel
from .base_table import TexBaseTable

class ECObjectGroupList(TexBaseTable):

    def __init__(self, out):
        object_group_dicts = [og.to_dict() for og in ecroot.object_groups()]
        super(ECObjectGroupList, self).__init__(out, row_dict_list=object_group_dicts)
        self.out = out

    @property
    def table_config(self):
        if self._config is None:
            self._config = super(ECObjectGroupList, self).table_config
            # Set chosen keys from possible keys in imput dict
            chosen_column_keys = [ "name",
                                   "description_latex"
                                 ]
            self._config.add_column_keys(chosen_column_keys)
            # Header replacements for different lines
            header_first_line_map = {
                                        'name'        :  '\\textbf{Name}',
                                        'description_latex' :  '\\textbf{Description}',
                                    }
            self._config.add_header_line(header_first_line_map)
            self._config.add_column_widths({"description_latex" : 9.5})
            self._config.add_raw_flag("description_latex")
            self._config.add_raw_flag("name")
        return self._config

    @classmethod
    def get_outfile_name(cls, out):
        return os.path.join(out, "tex", "object_groups.tex")


class ECObjectGroupSimpsonsPanel(TexBasePanel):

    def __init__(self, out, class_type):
        super(ECObjectGroupSimpsonsPanel, self).__init__(out)
        self.class_type = class_type

    @classmethod
    def get_outfile_name(cls, out, class_type):
        return os.path.join(out, "tex", "object_groups_simpsons_plot_panel_{}.tex".format(class_type))

    #~ @classmethod
    #~ def get_outfile_name(cls, out):
        #~ return os.path.join(out, "tex", "object_groups_simpsons_plot_panel.tex")

    def add_simpsons_plot(self, group_tag, plot_path):
        label = "Simpsons-{}".format(group_tag)
        group_info = ecroot.get_object_group_info_by_name_tag(group_tag)
        caption = "Overview of total contributions (single bin) for the " \
                  "{} object group. The numbers on the top indicate " \
                  "the observed p-value for the data / simulation agreement.".format(group_info.name)
        self.add_plot(plot_path, label, caption)

        #~ self.tex += """
#~ \\begin{{figure}}[h]
    #~ \\begin{{center}}
        #~ \\includegraphics[width=0.47\\textwidth]{{{path}}}
        #~ \\caption{{Overview of total contributions (single bin) for the {object_group_name} object group. \
        #~ The numbers on the top indicate the observed p-value for the data / simulation agreement. }}
        #~ \\label{{fig:{label}}}
    #~ \\end{{center}}
#~ \\end{{figure}}
               #~ """.format(path=plot_path,
                          #~ object_group_name=group_info.name,
                          #~ label=label,
                          #~ )

    def write_tex(self):
        with open(self.get_outfile_name(self.out, self.class_type), "wb") as f:
            f.write(self.tex)

