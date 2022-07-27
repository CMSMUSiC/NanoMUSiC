#!/bin/env python
import sys
import os
import argparse
import luigi
import time
from multiprocessing import cpu_count
import select

from shellhelpers import expand_path

#3A Tools
import gridlib

from ectools.register import ecstyle

from luigi_music.tasks import ECPlotTask, FullClassificationTask, \
                              ClassificationDistributionPlotsTask,\
                              ClassificationAllClasstypePlotsTask, \
                              ClassificationPerClasstypePlotsTask, FullScanTask,\
                              PtildePlotTask, ScanTask, SingleBinScanTask, \
                              AnalysisNoteTask, ECScanJob, ScanFinaliaztionTask, \
                              ScanWriteBack, MCClassificationTask



def command_line_options():
    main_parser = argparse.ArgumentParser("Run MUSiC workflows with luigi")
    parsers = main_parser.add_subparsers( help="workflow option help", dest="workflow" )

    default_distributions = [d.name for d in ecstyle.distribution_types()]
    default_class_types = [d.name for d in ecstyle.class_types()]
    default_output_formats = ["png", "pdf", "root"]

    general_parser = argparse.ArgumentParser(add_help=False)
    general_group = general_parser.add_argument_group(description ="General options")
    general_group.add_argument("--min-yield",  default=0.1, type=float,
        help="Minimum MC yield for classes to be considered in scans / plots. Default:%(default)s",)
    general_group.add_argument("--campaign-id",
        help="Id of the current workflow campaign", required=True)
    general_group.add_argument("--lumi",
        help="Luminosity analyzed in this run", default=35922.0)
    general_group.add_argument("--remote-dir", default="/net/scratch_cms3a/pook/MUSiC/MUSiCout/",
        help="Folder where results should be proccessed")
    general_group.add_argument("--site", default="T2_DE_RWTH",
        help="Grid site to use for workflows")
    general_group.add_argument("--distributions", nargs="*",
        help="Distributions which should be proccessed. Default:{}".format(",".join(default_distributions)))
    general_group.add_argument("--class-types", nargs="*",
        help="Class types which should be scaned in RoI scan. Default:{}".format(",".join(default_class_types)))
    general_group.add_argument("--filter-systematics", nargs="+",
        help="Systematics which should be omitted (uses contains!)", default=[])
    general_group.add_argument("--formats", nargs="*",
        help="Requested output formats for plots. Default:{}".format(",".join(default_output_formats)))
    general_group.add_argument("--filter", nargs="+",
        help="A list of filter for ec names. ")
    general_group.add_argument("--veto", nargs="+",
        help="A list of filter for ec names. ")


    classification_parser = argparse.ArgumentParser(add_help=False)
    classification_group = classification_parser.add_argument_group(description ="Classification options")
    classification_group.add_argument("--workers-classification", default=min(8,cpu_count()-1), type=int)
    classification_group.add_argument("--myskims-path", default=expand_path("$MUSIC_CONFIG/MC/myskims_ram.yaml"),
        help="path to the MC myskims file. Default:$MUSIC_CONFIG/MC/myskims_ram.yaml")
    classification_group.add_argument("--myskims-path-data", default=expand_path("$MUSIC_CONFIG/Data/myskims_ram.yaml"),
        help="path to the Data myskims file. Default:$MUSIC_CONFIG/Data/myskims_ram.yaml")

    no_classification_parser = argparse.ArgumentParser(add_help=False)
    no_classification_group = no_classification_parser.add_argument_group(description ="Classification requirement options")
    no_classification_group.add_argument("--mc", default="bg.root", help="path to mc classification file")
    no_classification_group.add_argument("--data", default="data.root", help="path to data classification file")
    no_classification_group.add_argument("--signal", default='', help="path to signal classification file")

    available_classification_plots = ["distribution",
                                      "uncert",
                                      "simpsons",
                                      "uncertainty-map",
                                      "pdist",
                                      "class-yield",
                                      "dominant-process"]

    available_classification_tex = ["class-count", "object-group"]

    classification_plots_parser = argparse.ArgumentParser(add_help=False)
    classification_plots_group = classification_plots_parser.add_argument_group(description ="Classification plots options")
    classification_plots_group.add_argument("--workers-classification-plots", default=min(20,cpu_count()-1), type=int)
    classification_plots_group.add_argument("--requested-plots", nargs="+", default=available_classification_plots,
        help="List of plots to produce, produces all by default. Available options: " + ",".join(available_classification_plots))
    classification_plots_group.add_argument("--requested-tex", nargs="+", default=available_classification_tex)

    scan_parser = argparse.ArgumentParser(add_help=False)
    scan_parser_group = scan_parser.add_argument_group(description ="Classification plots options")
    scan_parser_group.add_argument("--workers-roi-scan", default=min(40,cpu_count()-1), type=int)
    scan_parser_group.add_argument("--nrounds", default=10000, type=int,
        help="Number of pseudo round in scan default: %(default)s")
    scan_parser_group.add_argument("--regionYieldThreshold", default=1e-6, type=float,
        help="Minimum yield for a single region to be considered in a RoI scan: %(default)s")
    scan_parser_group.add_argument("--mcStatUncertScaleFactor", default=1.0, type=float,
        help="Factor by which MC statistics uncertainties should be scaled in the scanner: %(default)s")
    scan_parser_group.add_argument("--dicedMCUncertScaleFactor", default=1.0, type=float,
        help="Factor by which MC uncertainties should be scaled only in the pseudo data generation for MC compared to the expected variations: %(default)s")
    scan_parser_group.add_argument("--dicedSignalUncertScaleFactor", default=1.0, type=float,
        help="Factor by which MC uncertainties should be scaled only in the pseudo data generation for Signal compared to the expected variations: %(default)s")
    scan_parser_group.add_argument("--integral-scan", action="store_true",
        help="Perform an integral scan instead of full RoI scan: %(default)s")
    scan_parser_group.add_argument("--nrounds-signal",
                                   default=200,
                                   type=int,
                                   help="Number of psudo round in signal scan default: %(default)s")
    scan_parser_group.add_argument("--skip-integral_check", action='store_true',
        help="Do not check for too big / small most significant deviations in integral scan")
    # Add options to fix scan hashs for all class_type distribution combination
    for class_type in default_class_types:
        for distribution in default_distributions:
            scan_parser_group.add_argument("--forced-data-scan-hash-{}-{}".format(class_type, distribution),
                help="Use a fixed data scan hash for {} {} scans".format(class_type, distribution))
            scan_parser_group.add_argument("--forced-mc-scan-hash-{}-{}".format(class_type, distribution),
                help="Use a fixed mc scan hash for {} {} scans".format(class_type, distribution))
            scan_parser_group.add_argument("--forced-signal-scan-hash-{}-{}".format(class_type, distribution),
                help="Use a fixed signal scan hash for {} {} scans".format(class_type, distribution))
    scan_parser_group.add_argument("--allow-hash-ambigouty", action="store_true",
        help="Allow several matching hashes for one requsted scan_id (campaign_id)."\
             "This is expected to happen if you want to combine scans from different states of the" \
             "MUSiC repositories")

    single_scan_parser = argparse.ArgumentParser(add_help=False)
    single_scan_parser.add_argument("--class-name", required=True)
    single_scan_parser.add_argument("--distribution", default="SumPt")

    parsers.add_parser("classify", parents=[general_parser, classification_parser])
    parsers.add_parser("classify-mc", parents=[general_parser, classification_parser])
    parsers.add_parser("classify-plot", parents=[general_parser, no_classification_group, classification_parser, classification_plots_parser])
    parsers.add_parser("classify-all", parents=[general_parser, classification_parser, classification_plots_parser])
    parsers.add_parser("scan", parents=[general_parser, no_classification_group, scan_parser])
    parsers.add_parser("scan-plot", parents=[general_parser, no_classification_group, scan_parser])
    parsers.add_parser("scan-class", parents=[general_parser, no_classification_group, scan_parser, single_scan_parser])
    parsers.add_parser("note", parents=[general_parser, classification_parser, no_classification_parser, classification_plots_parser, scan_parser])
    parsers.add_parser("all", parents=[general_parser, classification_parser, classification_plots_parser, scan_parser])

    args = main_parser.parse_args()
    # validate args
    if not args.class_types:
        args.class_types = default_class_types
    if not args.distributions:
        args.distributions = default_distributions
    if not args.filter:
        args.filter = []
    if not args.veto:
        args.veto = []
    if not args.filter_systematics:
        args.filter_systematics += ["*/*qcdWeightGamma*", "*/*qcdWeightQCD*", "*/*xs*NLO"]
    return args


def run_luigi(tasks, nworkers=1, retry=0):
    try:
        return luigi.build(tasks, workers=nworkers)
    # the luigi workflows are prone to IO issues with the low level select library
    # catch those error and retry a few times
    except select.error:
        if retry < 3:
            time.sleep(2)
            retry += 1
            return run_luigi(tasks, nworkers, retry=retry)
        return False

def main():

    conf = command_line_options()

    base_args = {"campaign_id" : conf.campaign_id,
                 "remote_dir" : conf.remote_dir}

    # check if jobs request workflows with a proxy delegation id.
    # This should be created once and shared among grid jobs. Since it needs
    # to be available during the creation of the dependency tree (requires functions)
    # we need to create it beforehand to prevent several concurrent calls to get an
    # delegation ID.
    #delegation_id = '' #Yannik commented it out (debug)
    if conf.workflow in ("classify", "classify-all", "classify-mc", "scan", "all", "note"):
        #print ("DEBUG LINE 182")
        se = gridlib.se.get_se_instance( conf.site )
        #print ("DEBUG LINE 184")
        proxy = gridlib.ce.ProxyDelegator( se )
        #print ("DEBUG LINE 186")
        #delegation_id = proxy.get_delegation_id() #Yannik commented it out (debug)
	#print ("DEBUG LINE 188")

    # run workflows for classification
    if conf.workflow in ("classify", "classify-all", "all"):
        kwargs = base_args.copy()
        kwargs["myskims_path"] = conf.myskims_path
        kwargs["myskims_path_data"] = conf.myskims_path_data
        #kwargs["delegation_id"] = delegation_id #Yannik commented it out (debug)
        classfication_task = FullClassificationTask(**kwargs)
        success = run_luigi([classfication_task], conf.workers_classification)
        conf.mc = classfication_task.output()['BGMergeTask'].path
        conf.data =  classfication_task.output()['DataMergeTask'].path
        if not success:
            raise RuntimeError("classify workflow step failed ending run")

    # for running only in MC samples for clasification
    if conf.workflow in ("classify-mc"):
        kwargs = base_args.copy()
        kwargs["myskims_path"] = conf.myskims_path
        print conf.myskims_path
        #kwargs["delegation_id"] = delegation_id #Yannik commented it out (debug)
        classfication_task = MCClassificationTask(**kwargs)
        success = run_luigi([classfication_task], conf.workers_classification)
        conf.mc = classfication_task.output()['BGMergeTask'].path
        if not success:
            raise RuntimeError("classify workflow step failed ending run")

    # we need min_yield for the following steps
    base_args["min_yield"] = conf.min_yield

    #validation of arguments
    if conf.workflow not in ("classify-mc"):
        if (conf.signal and conf.data):
            conf.data = ""
            print ("NOTE: Signal and Data can not be used together, removing data. Signal is : ", conf.signal)

    # run workflow for plot creation from classification
    if conf.workflow in ("classify-plot", "classify-all", "all"):
        # Create plots on a per distribution level for single event classes
        distribution_plot_task = ClassificationDistributionPlotsTask(mc=conf.mc,
                                            data=conf.data,
                                            signal=conf.signal,
                                            class_types=conf.class_types,
                                            distributions=conf.distributions,
                                            requested_plots=conf.requested_plots,
                                            requested_tex=conf.requested_tex,
                                            filter_systematics=conf.filter_systematics,
                                            ecfilter=conf.filter,
                                            ecveto=conf.veto,
                                            **base_args)
        success = run_luigi([distribution_plot_task], conf.workers_classification_plots)

        # Create plots which are aggregate classes by distribution and class type
        for class_type in conf.class_types:
            for distribution in conf.distributions:
                finalization_task = ClassificationPerClasstypePlotsTask(mc=conf.mc,
                                                                   data=conf.data,
                                                                   signal=conf.signal,
                                                                   class_type=class_type,
                                                                   distribution=distribution,
                                                                   requested_plots=conf.requested_plots,
                                                                   requested_tex=conf.requested_tex,
                                                                   filter_systematics=conf.filter_systematics,
                                                                   ecfilter=conf.filter,
                                                                   ecveto=conf.veto,
                                                                   **base_args)
                success =run_luigi([finalization_task], conf.workers_classification_plots)

        # Create plots which are aggregate classes by distribution over all class type
        all_classtype_plot_task = ClassificationAllClasstypePlotsTask(mc=conf.mc,
                                                                      data=conf.data,
                                                                      signal=conf.signal,
                                                                      class_types=conf.class_types,
                                                                      distributions=conf.distributions,
                                                                      requested_plots=conf.requested_plots,
                                                                      requested_tex=conf.requested_tex,
                                                                      filter_systematics=conf.filter_systematics,
                                                                      ecfilter=conf.filter,
                                                                      ecveto=conf.veto,
                                                                     **base_args)
        success = run_luigi([all_classtype_plot_task], conf.workers_classification_plots)
    base_args["lumi"] = conf.lumi
    # run RoI scan workflow
    if conf.workflow in ("scan", "all"):
        for class_type in conf.class_types:
            for distribution in conf.distributions:
                tasks = []
                scan_args = base_args.copy()
                #scan_args["delegation_id"] = delegation_id #Yannik commented it out (debug)
                scan_args["regionYieldThreshold"] = conf.regionYieldThreshold
                scan_args["mcStatUncertScaleFactor"] = conf.mcStatUncertScaleFactor
                scan_args["dicedMCUncertScaleFactor"] = conf.dicedMCUncertScaleFactor
                scan_args["dicedSignalUncertScaleFactor"] = conf.dicedSignalUncertScaleFactor
                add_scan_workflow(FullScanTask,
                                  tasks,
                                  conf,
                                  class_type,
                                  distribution,
                                  scan_args)
                success = run_luigi(tasks, conf.workers_roi_scan)

    # run RoI scan workflow
    if conf.workflow in ("scan-plot", "all"):
        for class_type in conf.class_types:
            for distribution in conf.distributions:
                tasks = []
                scan_args = base_args.copy()
                #scan_args["delegation_id"] = delegation_id #Yannik commented it out (debug)
                scan_args["regionYieldThreshold"] = conf.regionYieldThreshold
                scan_args["mcStatUncertScaleFactor"] = conf.mcStatUncertScaleFactor
                scan_args["dicedMCUncertScaleFactor"] = conf.dicedMCUncertScaleFactor
                scan_args["dicedSignalUncertScaleFactor"] = conf.dicedSignalUncertScaleFactor
                add_scan_workflow(ScanFinaliaztionTask,
                                  tasks,
                                  conf,
                                  class_type,
                                  distribution,
                                  scan_args)
                success = run_luigi(tasks, conf.workers_roi_scan)

    if conf.workflow in ("scan-class",):
        from luigi_music.tasks.scan import ScanDBInitTask
        kwargs = {'forced_class_names' : [conf.class_name],
                  'mc' : conf.mc,
                  'data' : conf.data,
                  'distribution' : conf.distribution,
                  'nrounds' : conf.nrounds,
                  'nrounds_signal' : conf.nrounds_signal,
                  'integralScan' : conf.integral_scan,
                  'filter_systematics': conf.filter_systematics,
                  'regionYieldThreshold' : conf.regionYieldThreshold,
                  'mcStatUncertScaleFactor' : conf.mcStatUncertScaleFactor}
        tasks = [ScanTask(**kwargs)]
        success = run_luigi(tasks)

    # run workflow for analysis note outputs
    if conf.workflow in ("note"):
        kwargs = base_args.copy()
        kwargs["mc"] = conf.mc
        kwargs["data"] = conf.data
        kwargs["nrounds"] = conf.nrounds
        kwargs["nrounds_signal"] = conf.nrounds_signal

        kwargs["class_types"] = conf.class_types
        kwargs["distributions"] = conf.distributions
        kwargs['filter_systematics'] = conf.filter_systematics
        kwargs['regionYieldThreshold'] = conf.regionYieldThreshold
        kwargs['dicedMCUncertScaleFactor'] = conf.dicedMCUncertScaleFactor
        kwargs['dicedSignalUncertScaleFactor'] = conf.dicedSignalUncertScaleFactor
        result_task = AnalysisNoteTask(**kwargs)
        success = run_luigi([result_task], nworkers=conf.workers_classification)
        sys.exit(int(success))


def add_scan_workflow(task_class,
                      tasks,
                      conf,
                      class_type,
                      distribution,
                      base_args):
    ''' Add a scan task based on scan_class to a tasks lists '''

    # needed because argparse sets "-" in class type / distribution type names to "_"
    dist_key = distribution.replace("-", "_")
    class_key = class_type.replace("-", "_")
    tasks.append(task_class(class_type=class_type,
                            distribution=distribution,
                            mc=conf.mc,
                            data=conf.data,
                            signal=conf.signal,
                            nrounds=conf.nrounds,
                            nrounds_signal=conf.nrounds_signal,
                            integral_scan=conf.integral_scan,
                            forced_data_scan_hash=getattr(conf, "forced_data_scan_hash_{}_{}".format(class_key, dist_key)),
                            forced_mc_scan_hash=getattr(conf, "forced_mc_scan_hash_{}_{}".format(class_key, dist_key)),
                            forced_signal_scan_hash=getattr(conf, "forced_signal_scan_hash_{}_{}".format(class_key, dist_key)),
                            filter_systematics=conf.filter_systematics,
                            ecfilter=conf.filter,
                            ecveto=conf.veto,
                            **base_args))

if __name__=="__main__":
    main()
