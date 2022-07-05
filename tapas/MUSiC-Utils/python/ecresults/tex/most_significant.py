import os

from ectools.register import ecroot
from ecresults.util import ClassCountMixin

from .base_panel import TexBasePanel
from .base_table import TexBaseTable
import dbtools
import argparse
import sqlalchemy
from dbtools import Sdb

class RoiScanMostSignificantTable(TexBaseTable):

    def __init__(self,
                 out,
                 class_type,
                 distribution,
                 database,
                 n_significant,
                 scan_name=None,
                 scan_hash=None):
        self.class_type = class_type
        self.distribution = distribution
        self.n_significant = n_significant
        self.scan_name = scan_name
        self.scan_hash = scan_hash
        rows = self.get_rows(database,
                             n_significant,
                             class_type,
                             distribution,
                             scan_name=scan_name,
                             scan_hash=scan_hash)
        super(RoiScanMostSignificantTable, self).__init__(out, rows)
        self.out = out

    @classmethod
    def get_default_conf_args(cls, class_type):
        args = []
        args += ['--filter-{class_type}'.format(class_type=class_type)]
        return args

    @classmethod
    def get_default_conf_namespace(cls, class_type):
        ''' Create a argarse like namespace object to emulate ectools configs'''
        parser = argparse.ArgumentParser()
        ecroot.add_ec_standard_options(parser)
        args = cls.get_default_conf_args(class_type)
        conf = parser.parse_args(args)
        return conf

    @classmethod
    def get_rows(cls,
                 database,
                 n_significant,
                 class_type,
                 distribution,
                 scan_name=None,
                 scan_hash=None,
                 ):

        sdb = Sdb(database)
        for scan_info in sdb.scan_info(name=scan_name, hash=scan_hash):
            #if scan_info.scan_type != "dataScan":
            if scan_info.scan_type != "dataScan" and scan_info.scan_type != "signalScan":
                continue
            rows, header = sdb.significant(scan_info.hash,
                                           n=5000,
                                           mode="median",
                                           return_rows=True)
            ec_names = [r[0] for r in rows]
            ec_names, dropped = ecroot.filter_class_names(set(ec_names),
                                    cls.get_default_conf_namespace(class_type),
                                    distribution=distribution)
            rows = [r for r in rows if r[0] in ec_names]
            return [ {h:v for h,v in zip(header, row)} for row in rows[:n_significant]]

    @property
    def table_config(self):
        if self._config is None:
            self._config = super(RoiScanMostSignificantTable, self).table_config
            # Set chosen keys from possible keys in imput dict
            chosen_column_keys = [   "event_class",
                                     "roi_start",
                                     "roi_end",
                                     "mc_events",
                                     "sys_uncert",
                                     "data_events",
                                     "p",
                                     "p-tilde"
                                ]
            self._config.add_column_keys(chosen_column_keys)
            # Header replacements for different lines
            header_first_line_map = {
                                        'event_class'      :  '\\textbf{Eventclass}',
                                        'round'       :  '\\textbf{round}',
                                        'roi_start':  '\\textbf{RoI start}',
                                        'roi_end'         :  '\\textbf{RoI end}   ',
                                        'mc_events'  :  '$\\mathbf{N_{MC}}$      ',
                                        'sys_uncert'    :  '\\textbf{Systematatic}    ',
                                        'data_events'    :  '$\\mathbf{N_{Data}}$      ',
                                        'p' :  '\\textbf{p}   ',
                                        'p-tilde' :  '$\\mathbf{\\tilde{p}}$   ',
                                    }
            self._config.add_header_line(header_first_line_map)
            header_second_line_map = { 'sys_uncert' : '\\textbf{Uncertainty}',
                                       'roi_start':  '\\textbf{(GeV)}',
                                       'roi_end'         :  '\\textbf{(GeV)}',
            }
            self._config.add_header_line(header_second_line_map)
            def pretty_name(texrow):
                return "$" + ecroot.latex_ec_name(texrow.event_class,) + "$"
            self._config.add_column_func(pretty_name, 'event_class')

        return self._config

    @classmethod
    def get_outfile_name(cls, out, class_type, distribution):
        return os.path.join(out, "tex", "most_significant_{}-{}_table.tex".format(class_type, distribution))

    @property
    def outfile_name(self):
        return self.get_outfile_name(self.out, self.class_type, self.distribution)

class RoiScanMostSignificantPlotPanel(TexBasePanel):

    def __init__(self, out, class_type, distribution, plot_width=1.0):
        super(RoiScanMostSignificantPlotPanel, self).__init__(out, plot_width=plot_width)
        self.class_type = class_type
        self.distribution = distribution

    @classmethod
    def get_outfile_name(cls, out, class_type, distribution):
        return os.path.join(out, "tex", "most_significant_{}-{}_panel.tex".format(class_type, distribution))

    @property
    def outfile_name(self):
        return self.get_outfile_name(self.out, self.class_type, self.distribution)

    #~ @classmethod
    #~ def get_outfile_name(cls, out):
        #~ return os.path.join(out, "tex", "object_groups_simpsons_plot_panel.tex")

    def add_class_plot(self, rank, ec_name, distribution, plot_path):
        distribution_info = ecroot.get_distribution_type_info_by_name(distribution)
        class_type_info = ecroot.get_class_type_info_by_name(ec_name)
        label = "most-significant-{}-{}-{}".format(rank, class_type_info.name, distribution)
        ordinal = lambda n: "%d%s" % (n,"tsnrhtdd"[(n/10%10!=1)*(n%10<4)*n%10::4])


        caption = "{rank} most significant {class_type} class for the " \
                   "{distribution} distribution: ${class_name}$".format(rank=ordinal(rank),
                                                                      distribution=distribution_info.latex_tag,
                                                                      class_type=class_type_info.name,
                                                                      class_name=ecroot.latex_ec_name(ec_name, "latex")
                                                                      )
        comment = "\InputIfFileExists{class_comments/%s}{}{}" % (ec_name)
        self.add_plot(plot_path, label, caption, comment)

class RoiScanStatsTable(TexBaseTable):

    def __init__(self,
                 out,
                 distribution,
                 database_dict,#{"class_type: db_path}
                 scan_name=None,
                 scan_hash=None):
        self.database_dict = database_dict
        self.distribution = distribution
        self.scan_name = scan_name
        self.csv_rows_class_type_dict = {}
        self.mc_shift = mc_shift
        rows = self.get_rows()
        super(RoiScanStatsTable, self).__init__(out, rows)

    @property
    def table_config(self):
        if self._config is None:
            self._config = super(RoiScanStatsTable, self).table_config
            # Set chosen keys from possible keys in imput dict
            chosen_column_keys = [   "class_type",
                                     "borders",
                                     "excess",
                                     "deficit",
                                     "excess_skipped",
                                     "deficit_skipped",
                                ]
            self._config.add_column_keys(chosen_column_keys)
            # Header replacements for different lines
            header_first_line_map = {
                                        'class_type'      :  '\\textbf{Class Type}',
                                        'borders'       :  '\\textbf{Expected Yield}',
                                        'excess':  '\\textbf{Excess}',
                                        'deficit'         :  '\\textbf{Deficit}   ',
                                        'excess_skipped'  :  '$\\mathbf{Excess}$      ',
                                        'deficit_skipped'    :  '\\textbf{Deficit}    ',
                                    }
            self._config.add_header_line(header_first_line_map)
            header_second_line_map = { 'excess_skipped' : '\\textbf{(skipped)}',
                                       'deficit_skipped':  '\\textbf{(skipped)}',
            }
            self._config.add_header_line(header_second_line_map)
            # define lines which are alreade latex formatted
            self._config.add_raw_flag("class_type")
            self._config.add_raw_flag("borders")
            # group by class type
            self._config.groupkey="class_type"
            self._config.hide_group = False
            self._config.use_document = True
        return self._config


    def get_rows(self):
        rows = []
        bins = self._get_bins()
        for class_type, db_path in self.database_dict.items():
            engine = sqlalchemy.create_engine('sqlite:///' + db_path,
                                                echo=False)
            print(db_path)
            with dbtools.session_scope( engine ) as scan_session:

                scan = dbtools.match_hash(self.scan_name,
                                          scan_session,
                                          allow_name=True,
                                          multiple=True,
                                          scan_type="dataScan",
                                          distribution=self.distribution)[0]
                self.csv_rows_class_type_dict[class_type] = []
                stat_dict_list, stat_dict_skipped_list = self._get_stats_from_db(scan_session,
                                                                                 scan.hash,
                                                                                 class_type)
                for i,(statdict, stat_dict_skipped) in enumerate(zip(stat_dict_list,stat_dict_skipped_list)):
                    print statdict
                    rowdict = {'class_type': class_type,
                               'excess' : statdict['excess'],
                               'deficit' : statdict['deficit'],
                               'excess_skipped' : str(int(stat_dict_skipped['excess'])),
                               'deficit_skipped' : str(int(stat_dict_skipped['deficit']))}

                    if i < len(bins) -1 :
                        rowdict['borders'] = "$%s - %s$" %(self._format_border_string(bins[i]),
                                                         self._format_border_string(bins[i+1]))
                    else:
                        rowdict['borders'] = "$> %s$" % self._format_border_string(bins[-1])

                    rows.append(rowdict)
        return rows

    def _format_border_string(self, border):
        if border - int(border) > 0:
            borderstring = "%.2f" % border
        else:
            borderstring = "%d" % int(border)
        return borderstring

    def _get_stats_from_db(self, session, scan_hash, class_type):
        filters = []
        query = session.query(dbtools.Result).filter_by(hash = scan_hash)
        results = query.all()
        print(results[:10])
        stat_dicts = []
        stat_dicts_skipped = [{'excess' : 0 , 'deficit' : 0}]
        for ibin, border in enumerate(self._get_bins()):
            stat_dicts.append({'excess' : 0 , 'deficit' : 0, 'border': border})
            stat_dicts_skipped.append({'excess' : 0 , 'deficit' : 0, 'border': border})

        #~ stat_dicts.append({'excess' : 0 , 'deficit' : 0, 'border': 1e9})
        #~ stat_dicts_skipped.append({'excess' : 0 , 'deficit' : 0, 'border': 1e9})

        bins = self._get_bins()
        for r in results:
            prefix, objects, class_tag = ecroot.split_ec_name(r.event_class)
            class_type_object = ecroot.get_class_type_info_by_name_tag(class_tag)
            if class_type_object.name != class_type:
                continue
            #~ if r.score > 0.1:
                #~ continue
            stat_dict = None
            stat_dict_skipped = None
            for i, border in enumerate(bins):
                if i == len(bins) -1:
                    stat_dict = stat_dicts[-1]
                    stat_dict_skipped = stat_dicts_skipped[-1]
                elif border < r.ec_total_mc_events and r.ec_total_mc_events < bins[i+1]:
                    stat_dict = stat_dicts[i]
                    stat_dict_skipped = stat_dicts_skipped[i]
                    break
            if r.skipped:
                self._add_class_stat(stat_dict_skipped, r,class_type, skipped=True)
            else:
                self._add_class_stat(stat_dict, r, class_type)
        return stat_dicts, stat_dicts_skipped

    def _add_class_stat(self, stat_dict, result,class_type, skipped=False):
        if result.mc_events * self.mc_shift < result.data_events:
            self.csv_rows_class_type_dict[class_type].append(["excess", result.event_class, result.ec_total_mc_events, result.mc_events, result.data_events,result.score, int(skipped)])
            stat_dict['excess'] += 1
        else:
            stat_dict['deficit'] += 1

            self.csv_rows_class_type_dict[class_type].append(["deficit", result.event_class, result.ec_total_mc_events, result.mc_events, result.data_events,result.score, int(skipped)])

    def save_csv(self):
        import csv
        for class_type, row_list in self.csv_rows_class_type_dict.items():
            with open("deficit_excess_stats_%s.csv" % class_type, "w") as csv_file:
                writer = csv.writer(csv_file)
                writer.writerow(["deviation type", "name", "n mc total", "n mc", "n data", "p", "skipped"])
                for row in row_list:
                    writer.writerow(row)

    def _get_bins(self):
        return [0, 0.1, 1, 3, 10 , 100]
