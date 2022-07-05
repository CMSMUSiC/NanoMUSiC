import os

try:
    from table2latex.textable import TexTable, TexTableConfig, escape_latex
except ImportError:
    raise ImportError("table2latex not installed."
            "checkout https://github.com/tobias-pook/table2latex")


class TexBaseTable(object):

    def __init__(self, out, row_dict_list=None, config=None, input_csv_file=None):
        self.out = out
        self._config = config
        self.row_dict_list = row_dict_list
        if row_dict_list is None:
            self.row_dict_list = []
        self.input_csv_file = input_csv_file

    @property
    def table_config(self):
        if self._config is None:
            self._config = TexTableConfig()
        return self._config

    @classmethod
    def get_outfile_name(cls, out):
        return os.path.join(out, "tex", "table.tex")

    @property
    def outfile_name(self):
        return self.get_outfile_name(self.out)

    @property
    def tex_table(self):
        # create texttable object and fill data
        table = TexTable(config=self.table_config, out=self.outfile_name)
        if self.input_csv_file:
            table.read_csv(self.input_csv_file)
        for row_dict in self.row_dict_list:
            table.add_row_dict(row_dict)
        return table

    def write_table(self):
        self.tex_table.write_tex_file()
