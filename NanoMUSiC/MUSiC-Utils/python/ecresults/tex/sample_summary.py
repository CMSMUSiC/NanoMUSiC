
import os

from .base_table import TexBaseTable
from .mc_sample_table_cfg import config as table_config
from .mc_sample_reference_table_cfg import config as table_reference_config


class SampleSummaryTable(TexBaseTable):

    def __init__(self, out, sample_csv_path, table_type="summary"):
        self.table_type = table_type
        super(SampleSummaryTable, self).__init__(out, input_csv_file=sample_csv_path)
        self.out = out

    @property
    def table_config(self):
        if self._config is None:
            if self.table_type == "summary":
                self._config = table_config
            if self.table_type == "references":
                self._config = table_reference_config
        return self._config

    @property
    def outfile_name(self):
        return self.get_outfile_name(self.out, table_type=self.table_type)

    @classmethod
    def get_outfile_name(cls, out, table_type="summary"):
        if table_type == "summary":
            return os.path.join(out, "tex", "mc_set_table.tex")
        if table_type == "references":
            return os.path.join(out, "tex", "mc_set_references.tex")
        raise RuntimeError("No valid option used for table type, choose from: summary, references")
