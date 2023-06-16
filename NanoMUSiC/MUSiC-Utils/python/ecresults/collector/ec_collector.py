import os
import pickle
import hashlib
import sqlalchemy
import traceback
import numpy as np
import math

from colors import PROCESS_GROUP_STYLES, SYSTEMATIC_STYLES, DETAILED_SYSTEMATIC_STYLES, HistStyle

import roothelpers
import dbtools

from ectools.register import ecroot

from ..plots import ECUncertaintyPlot, GeneralPlots
from ..util import ECRecordList

class ECCollector(object):

    ''' This class collects information from event classes and scan databases.

        Information about each event class is aggreagted into a single record (dict)
    '''

    def __init__(self, conf):
        self.conf = conf
        # cached properties
        self._cache_hash = None
        self._records = ECRecordList([])

    @classmethod
    def cache_hash(cls, conf, distribution, ec_class_names=None):
        hash_components = [f for f in conf.filter]
        hash_components += [v for v in conf.veto]
        hash_components.append(os.path.abspath(conf.mc))
        hash_components.append(distribution)
        if conf.scan_db_path:
            hash_components.append(conf.scan_db_path)
        if conf.data:
            hash_components.append(os.path.abspath(conf.data))
        if ec_class_names:
            hash_components += ec_class_names
        if conf.scan_id:
            hash_components.append(conf.scan_id)

        hash_components.append("%.6f" % conf.min_yield)
        return hashlib.sha1(''.join(hash_components)).hexdigest()

    @classmethod
    def cache_file_name(cls,
                        conf,
                        distribution,
                        ec_class_names=None,
                        object_group_tag=None):
        cache_file_name = "ec_global_cache_%s.pkl" % cls.cache_hash(conf,
                                                                    distribution,
                                                                    ec_class_names)
        if conf.cache_dir and not os.path.isdir( os.path.dirname(conf.cache_dir) ):
            os.makedirs( os.path.dirname(conf.cache_dir) )
            cache_file_name = os.path.join(conf.cache_dir, cache_file_name)
        return cache_file_name

    @classmethod
    def get_records(cls, conf, distribution, scan_hash=None):
        kwargs = {}
        if hasattr(conf, "object_group_tag"):
            kwargs["object_group_tag"] = conf.object_group_tag
        cache_file_name = cls.cache_file_name(conf, distribution, **kwargs)
        if not conf.renew_cache and os.path.exists(cache_file_name):
            print "loading records from cached file " + cache_file_name
            cached_records = cls.load_cache(cache_file_name)

            ec_names = cached_records.class_names
            print("number of names before filter", len(ec_names), len(cached_records))
            filtered_names, vetoed = ecroot.filter_class_names(set(ec_names),
                                                               conf,
                                                               distribution=distribution,
                                                               **kwargs)
            print("number of records before filter")
            cached_records = cached_records.filter_in('name', filtered_names)
            print("number of names after filter", len(filtered_names), len(cached_records))
            return cached_records
        else:
            records_raw = ecroot.ec_loop(conf,
                                         collect_record,
                                         distribution=distribution,
                                         scan_hash=scan_hash)
            records_raw = [ record_raw for record_raw in records_raw if record_raw ]
            print("Collected %d hist entries " % len(records_raw))
            cls.dump_to_cache(cache_file_name, records_raw)
        return ECRecordList(records_raw)

    @classmethod
    def load_cache(cls, cache_file_name):
        records_raw = pickle.load(open(cache_file_name, "rb"))
        return ECRecordList(records_raw)

    @classmethod
    def dump_to_cache(cls, cache_file_name, ec_records):
        pickle.dump( list(ec_records), open( cache_file_name , "wb" ) )

    @classmethod
    def get_record(cls,
                   conf,
                   distribution,
                   i=0,
                   N=1,
                   mc_ec=None,
                   data_ec=None,
                   status_line=True,
                   scan_hash=None):
        ''' collect and return a single record '''
        return collect_record(conf,
                              i=i,
                              N=N,
                              data_ec=data_ec,
                              mc_ec=mc_ec,
                              distribution=distribution,
                              status_line=status_line,
                              scan_hash=scan_hash )

    @property
    def records(self):
        if self._records is None:
            self._records = self.get_records(self.conf)
        return self._records

    @records.setter
    def records(self, value):
        if isinstance(value, ECRecordList):
            self._records = value
        else:
            self._records = ECRecordList(value)

    @classmethod
    def add_commandline_options( cls, parser ):
        # uncertainty parser is needed in collection and also contains all needed
        # options for other tasks with GeneralPlots
        ECUncertaintyPlot.add_commandline_options(parser)
        parser.add_argument('--renew-cache', action="store_true", help="Renew cache for ec records")


def collect_record(conf,
                   i=0,
                   N=0,
                   data_ec=None,
                   mc_ec=None,
                   distribution=None,
                   status_line=True,
                   scan_hash=None,
                   **kwargs ):
    # If not scanned skip class
    if not mc_ec:# or not mc_ec.hasDataScan( distribution ):
        return {}
    conf.scale_to_area = False
    ec = GeneralPlots( conf,
                       distribution,
                       mc_ec=mc_ec,
                       data_ec=data_ec
                     )
    ec.create_mc_stack( "mc", add_to_legend=True )
    result_dict = {}

    result_dict["name"] = mc_ec.GetName()
    try:
        result_dict["class_type"] = ecroot.get_class_type_info_by_name(result_dict["name"]).name
    except:
        print "Unable to find class type for class: {}".format(result_dict["name"])
        print ecroot.get_class_type_info_by_name(result_dict["name"])
        print ecroot.split_ec_name(result_dict["name"])
        print ecroot.get_class_type_info_by_name(result_dict["name"])
    result_dict["distribution"] = distribution
    result_dict["label"] = ecroot.latex_ec_name( mc_ec.GetName(), style="root" )
    result_dict["lumi"] = ec.lumi
    result_dict["run_hash"] = mc_ec.getRunHash()
    result_dict["cme"] = ec.cme
    result_dict["legend_dict"] = ec.legend_entries

    object_dict = mc_ec.getCountMap()
    if status_line:
        print( "{:<40} {:>4}/{:<4}".format( mc_ec.GetName(),
                                            i+1,
                                            N ) )
    total_bin_content = 0.
    integral_process_dict = {}
    for hist_ec in ec.mc_stack.GetHists():
        integral = hist_ec.Integral()

        total_bin_content += integral
        name = hist_ec.GetName()
        process_group_info = PROCESS_GROUP_STYLES.get( name, HistStyle( label=name, color=33 ) )
        color = hist_ec.GetFillColor()
        integral_process_dict[ name ] = { "integral": integral,
                                          "color" : color,
                                          "name" : name,
                                          "legend_name" : roothelpers.root_latex_convert( process_group_info.label )
                                        }
    if conf.min_yield > total_bin_content:
        return {}
    if data_ec:
        ec.create_data_hist()
        data_integral = ec.data_hist.Integral()
    else:
        data_integral = 0.
    result_dict["integral_process_dict"] = integral_process_dict
    result_dict["integral"] = total_bin_content

    result_dict["data_integral"] = data_integral


    result_dict["syst_stats"] = {}
    combined_rel_systematics = ecroot.combined_uncertainty_by_systematic_name( mc_ec,
                                                          distribution,
                                                          filter_systematics=conf.filter_systematics,
                                                           with_stat=True,
                                                           relative=True,
                                                           )
    for syst, hist in combined_rel_systematics.items():
        vals = [hist.GetBinContent(ibin) for ibin in range(hist.GetNbinsX()) if hist.GetBinContent(ibin) > 0 and ec.total_mc_hist.GetBinContent(ibin) > 0.001]
        mean = np.mean(vals)
        median = np.median(vals)
        if not np.isnan(median):
            result_dict["syst_stats"][syst] = {'median' : median, 'mean': mean}

    combined_systematics = ecroot.combined_uncertainty_by_systematic_name( mc_ec,
                                                          distribution,
                                                          filter_systematics=conf.filter_systematics,
                                                           with_stat=False)
    syst_integral = 0.
    for syst, hist in combined_systematics.items():
        syst_integral = math.hypot( syst_integral, hist.Integral() )

    combined_stat_uncertainty = ecroot.total_combined_stat_uncertainty(
                                                                    mc_ec,
                                                                    distribution)
    stat_integral = 0.
    for ibin in range(combined_stat_uncertainty.GetNbinsX() + 1):
        stat_integral = math.hypot(stat_integral, combined_stat_uncertainty.GetBinContent(ibin))
    total_syst_integral = math.hypot(syst_integral, stat_integral)
    result_dict["syst_integral"] = syst_integral
    result_dict["stat_integral"] = stat_integral
    result_dict["total_syst_integral"] = total_syst_integral

    ## Collect infos for uncertainty summaries
    conf.scale_to_area = True
    try:
        ec_uncert = ECUncertaintyPlot(
                               conf,
                               distribution,
                               mc_ec=mc_ec,
                               data_ec=data_ec,
                               relative = True
                            )

        if not conf.detailed_systematics:
            group_infos = SYSTEMATIC_STYLES
        else:
            group_infos = DETAILED_SYSTEMATIC_STYLES
        systematic_bin_frequencies = {}
        bin_edges = []
        for name, hist in ec_uncert.get_uncert_group_hist( group_infos ):
            bin_edges, sys_freq = ecroot.get_distribution_width_weighted( hist, name, ec_uncert.total_mc_hist )
            systematic_bin_frequencies[name] = sys_freq
        bin_edges, sys_freq = ecroot.get_distribution_width_weighted( ec_uncert.error_total,
                                                                      "total",
                                                                      ec_uncert.total_mc_hist )
        systematic_bin_frequencies[ "total" ] = sys_freq
        result_dict['uncert_spikes'] = []
        for edge, freq in zip(bin_edges, sys_freq):
            if edge > 5.0 and freq > 0.:
                result_dict['uncert_spikes'].append([edge, freq])

        i = 0
        for ed, freq in zip(bin_edges, sys_freq):
            if ed > 1.5:
                continue
            i += 1

    except:
        print("Error!")
        traceback.print_exc()
        bin_edges, systematic_bin_frequencies = [],[]

    result_dict["dist_edges"] = bin_edges
    result_dict["syst_bin_dists"] = systematic_bin_frequencies

    ## Collect scan infos
    result_dict["p-value"] = -1.
    result_dict["p-tilde"] = -1.
    result_dict["nrounds"] = 0
    result_dict["scan_hash"] = None
    result_dict["scan_skipped"] = False
    if conf.scan_db_path and os.path.exists(conf.scan_db_path) and conf.scan_id and (conf.show_p_value or conf.sort_option == "p-value"):
        engine = sqlalchemy.create_engine( 'sqlite:///' + conf.scan_db_path,
                                    echo=False )
        with dbtools.session_scope( engine ) as session:
            search_id = scan_hash if scan_hash else conf.scan_id
            scan = dbtools.match_hash(search_id,
                                      session,
                                      allow_name=True,
                                      scan_type="dataScan",
                                      distribution=distribution)
            result_dict["scan_hash"] = scan.hash
            filters = [dbtools.Result.hash==scan.hash,
                       dbtools.Result.event_class==result_dict["name"]]
            query = session.query( dbtools.Result.score, dbtools.Result.skipped ) \
                           .filter(*filters)
            if query.count():
                result_dict["p-value"] = float(str(query[0][0]))
                result_dict["scan_skipped"] = query[0][1]
                #print("Found p-value {} {}".format(result_dict["p-value"], result_dict["scan_skipped"]))
            query = session.query( dbtools.CorrectedResult ) \
                .filter( dbtools.CorrectedResult.hash==scan.hash, dbtools.CorrectedResult.event_class==result_dict["name"] )
            if query.count():
                result_dict["p-tilde"] = float(query[0].score)
                result_dict["nrounds"] = int(query[0].comp_count)
    # Finalize ecroot.ec_loop function
    #if conf.jobs > 1: mc_ec.Delete()
    #if data_ec and conf.jobs > 1: data_ec.Delete()
    return result_dict
