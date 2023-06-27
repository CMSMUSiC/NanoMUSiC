# This file contains the ECRootHelper which inherits from ECStatHelper
# and adds functionality for TEventClass root objects
#( no need to have root installed with ECStyleHelper or ECStatHelper funtions)

import os
import math
import ROOT
import fnmatch
import warnings
import multiprocessing
from collections import namedtuple
import subprocess
import hashlib
import numpy as np
import ROOT as ro

from .io import ECIOHelper
from .misc import rremove

import processgroup

from roothelpers import root_hist2arrays, root_get_error_hist, \
    convert_ndc_axis_to_pad, convert_ndc_pad_to_axis, convert_user_to_ndc, \
    get_canvas_size, root_setup, root_sum_hist, root_absolute_difference, \
    root_sqsum_hist, TextAlign, Font

from shellhelpers import tmp_chdir, expand_path

class ECRootHelper(ECIOHelper):
    ''' This class provides a faascade of classmethods to access the TEventClass root object'''

    @classmethod
    def rescale_to_luminosity(cls, hist, initial_lumi, target_lumi):
        factor = target_lumi / initial_lumi
        hist.Scale(factor)
        return hist

    @classmethod
    def total_event_yield(cls, event_class, distribution):
        """Calculate a ROOT TH1 histogram containing all data contributions
        added up.

        The distribution is obtained from the parameter distribution.

        Note:
            The created histogram has to be deleted externally.
        """

        # No manual deletion of the list necessary since the objects
        # are still owned by the Event Class.
        hists = list(event_class.getHistoPointer(p, distribution) for p
                     in list(event_class.getProcessList()))

        # Filter out null-pointers
        hists_filtered = filter(bool, hists)
        hists = list(hists_filtered)
        if len(hists) < 1:
            return None
        else:
            return root_sum_hist(hists)

    @classmethod
    def event_yield_by_process_group(cls,
                                     event_class,
                                     distribution,
                                     aggregation_level="medium"):
                                     #aggregation_level="none"):#LOR CHANGE TO DEBUG. default is "medium" as above
        """Group MC contributions by process group. Returns a dictionary with
        the process group name as keys and ROOT TH1 as values.

        Note:
            The created histograms have to be deleted externally.

        Args:
            event_class: A TEventClass object.
            distribution: name of the ECDistributionType which should be returned.
            aggregation_level: A key for a previously defined aggregation level.

        Returns:
            A dictionary with groupname => TH1F as key => value.
        """

        process_groups = {}

        for process in list(event_class.ProcessList()):
            hist = event_class.getHistoPointer(process, distribution)
            # Skip null-pointers
            if not hist:
                continue

            group = cls.get_plotting_group(process,
                                           aggregation_level)

            # If there is already one histogram, use ROOT's TH1::Add(),
            # otherwise add a new process group.
            if group in process_groups:
                process_groups[group].Add(hist)
            else:
                process_groups[group] = hist.Clone(group)

            # hist must not be deleted, it's owned by the Event Class.
        return process_groups

    @classmethod
    def _skip_systematic(cls, process, systematic, filter_systematics):
        name = "%s/%s" % ( process, systematic )
        for pattern in filter_systematics:
            if fnmatch.fnmatch( name, pattern ):
                return True
        return False

    @classmethod
    def combined_uncertainty_by_systematic_name(cls,
                                                event_class,
                                                distribution,
                                                relative=False,
                                                symmetrize=True,
                                                filter_systematics=[],
                                                with_stat=False):
        """Gather systematic uncertainties, grouped by systematic name. This
        combines the systematics over all processes and process groups.

        Within a systematic, the contributions from processes are added
        normally (not in quadrature).

        Note:
            The meaning of "systematic" changes here.
            Initially, systematics in the TEventClass are described as bin counts
            with shifted quantities. Here, the output "systematic" describes the
            absolute difference between the shifted quantity and the bin count.

            If shifts _up and _down are present, the error is symmetrized as

                uncertainty = |_up - count|/2 + |_down - count|/2

        Note:
            The created histograms have to be deleted externally.

        Args:
            event_class: A TEventClass object.
            distribution: name of the ECDistributionType which should be returned.
            relative: Relative uncertainties are returned if set to true.
            symmtrize: Symmetrize uncertainties if set to true.
            filter_systematics: A list of systematics which will be omitted.
            with_stat: Statistic uncertainties will be included if set to true.

        Returns:
            A dictionary with the uncertainty name as the key and a TH1
            histogram containing the uncertainty (one-side only, has to be
            multiplied by 2 to get the total error band width.)
        """

        combined_systematics = {}
        total = None
        import processgroup
        # Loop over all processes to combine systematics.
        for process in list(event_class.ProcessList()):
            hist = event_class.getHistoPointer(process, distribution)

            # Skip null-pointers
            if not hist:
                continue

            if not total:
                total = hist.Clone()
            else:
                total.Add(hist)

            # Loop over all possible systematic names, even if they are not
            # present in this particular process.
            for systematic in list(event_class.getSystematicNames()):
                # Skip  _down systematics (explanation see below for _up).
                # Note that this does not check the presence of
                # an _up systematic, thus the code is not failure-proof here.
                if symmetrize and systematic.endswith('Down'):
                    continue

                if filter_systematics and cls._skip_systematic( process, systematic, filter_systematics ):
                    continue

                hist_systematic = event_class.getSystHistoPointer(process,
                                                                  distribution,
                                                                  systematic)

                # Filter out systematics that are not present.
                if not hist_systematic:
                    continue
                group = processgroup.get_process_group(process)
                if systematic.startswith("qcdWeight") and not systematic.startswith("qcdWeight%s_" % group):
                    # consider qcd weights only for processes of the specific group
                    continue

                # Calculate the absolute difference:
                # uncertainty = |shifted - count|
                hist_syst_diff = root_absolute_difference(hist,
                                                          hist_systematic,
                                                          sign_if_first_negative=True)

                # If the systematic name ends with Up, there must be a
                # systematic with the same name, just ending with Down.
                # Those two are symmetrized.
                if symmetrize and systematic.endswith('Up'):
                    systematic = systematic[:-len('Up')]

                    hist_syst_down = event_class.getSystHistoPointer(process,
                                distribution, systematic + 'Down')

                    if not hist_syst_down:
                        msg = "Found %sUp systematic without" % systematic
                        msg += "corresponding %sDown systematic." % systematic
                        raise ValueError(msg)

                    hist_syst_diff_down = root_absolute_difference(hist,
                        hist_syst_down, sign_if_first_negative=True)

                    hist_syst_diff_up = hist_syst_diff

                    # Symmetrization performed by adding the two histograms
                    # with weights of 0.5.
                    hist_syst_diff = hist_syst_diff.Clone()

                    hist_syst_diff.Add(hist_syst_diff_up,
                                       hist_syst_diff_down,
                                       0.5,
                                       0.5)

                    # Cleaning up of temporary histograms.
                    # hist_syst_diff_up.Delete()
                    # hist_syst_diff_down.Delete()

                # Add to existing systematic entry or clone the histogram
                # and store it.
                # Treat systematics as *fully* correlated -> linear sum
                if systematic in combined_systematics:
                    nbins = combined_systematics[systematic].GetNbinsX()
                    if nbins != hist_syst_diff.GetNbinsX():
                        print("adding syst %s %s" % (systematic, process))

                    combined_systematics[systematic].Add(hist_syst_diff)
                else:
                    combined_systematics[systematic] = hist_syst_diff.Clone()

                # Clean up ROOT histograms.
                #hist_syst_diff.Delete()

                # hist_systematic must not be deleted, it's owned by the
                # Event Class.
        if relative:
            for syst in combined_systematics:
                combined_systematics[syst].Divide(total)
        # hist must not be deleted, it's owned by the Event Class.
        # if total:
        #     total.Delete()
        if with_stat:
            combined_systematics["MCstat"] = cls.total_combined_stat_uncertainty(
                                                    event_class,
                                                    distribution)
        return combined_systematics

    @classmethod
    def systematic_by_process_group(cls,
                                    event_class,
                                    systematic,
                                    distribution,
                                    aggregation_level="none"):
        """Get grouped contributions for a systematic by process group.

        Note:
            The created histograms have to be deleted externally.

        Args:
            event_class: A TEventClass object.
            systematic: The name of the systematic which should be returned.
            distribution: name of the ECDistributionType which should be returned.
            aggregation_level: A key for a previously defined aggregation level.

        Returns:
            A dictionary with the process group name as keys and ROOT TH1 as values.
        """
        systematics_by_process_group = {}
        for process in list(event_class.ProcessList()):

            group = cls.get_plotting_group(process,
                                           aggregation_level)

            hist_systematic = event_class.getSystHistoPointer(process,
                                                  distribution,
                                                  systematic)
            if hist_systematic:
                if group not in systematics_by_process_group:
                    systematics_by_process_group[group] = hist_systematic.Clone()
                else:
                    systematics_by_process_group[group].Add(hist_systematic)
        return systematics_by_process_group

    @classmethod
    def total_combined_stat_uncertainty(cls, event_class, distribution):
        """Get total contributions from statistic uncertainites.

        Note:
            The created histogram has to be deleted externally.

        Args:
            event_class: A TEventClass object.
            distribution: name of the ECDistributionType which should be returned.

        Returns:
            A histogram with the total combined stat uncert as a ROOT TH1.
        """
        process_hists = cls.event_yield_by_process_group(
            event_class,
            distribution,
            aggregation_level="none")

        stat_error_hist = root_get_error_hist(process_hists.values())
        #for key, hist in process_hists.items():
        #    hist.Delete()
        return stat_error_hist

    @classmethod
    def total_combined_uncertainty(cls,
                                   event_class,
                                   distribution,
                                   filter_systematics=[],
                                   with_stat=True):
        """Calculate combined systematic MC uncertainties, one side only,
        thus has to be multiplied by 2 when plotting the total uncertainty
        band.

        Args:
            event_class: A TEventClass object.
            distribution: name of the ECDistributionType which should be returned.
            filter_systematics: A list of systematics which will be omitted.
            with_stat: Statistic uncertainties will be included if set to true.

        Returns:
            A histogram (TH1F) containing the total combined uncertainty
        """

        combined_systematics = cls.combined_uncertainty_by_systematic_name(
            event_class,
            distribution,
            filter_systematics=filter_systematics)

        if not combined_systematics:
            warnings.warn("Warning: no systematics found in MC data set.")
            return

        if with_stat:
            combined_systematics["MCstat"] = cls.total_combined_stat_uncertainty(
                                                    event_class,
                                                    distribution)

        # Add up all uncertainties (given as TH1s) in quadrature (as TH1).
        retval = root_sqsum_hist(combined_systematics.values())
        #for hist in combined_systematics.values():
        #    hist.Delete()
        return retval

    # ==============================
    # Helpers for LateX and plotting
    # ==============================

    @classmethod
    def get_plotting_group(cls, process, aggregation_level=""):
        """Get the plotting group for a process in an event_class based on the selected aggregation level.


        Args:
            process: Name of the process for which the process group should be determined.
            aggregation_level: A key for a previously defined aggregation level.

        Returns:
            The process group as a string
        """
        if aggregation_level == "total":
            return "MC"
        group = processgroup.get_process_group(process)
        if not group or aggregation_level=="process":
            group = process
        if not group or aggregation_level=="customDY" or aggregation_level=="customWJ":
            group = process
        grouping_dict = cls.get_aggregation_groups().get(aggregation_level, {})
        for group_name, aggregated_group_name in grouping_dict.items():
            if fnmatch.fnmatchcase(group, group_name):
                return aggregated_group_name
        return group

    @classmethod
    def get_ec_distribution_types(cls, event_class):
        """ Helper function to remain backwards compatible for event_classes
            which lack a function to return distribution types.

            Args:
                event_class: A TEventClass object.

            Returns:
                A list of available distribution type names for this event class
        """
        try:
            vec = ROOT.std.vector(str)(event_class.getDistTypes())
            new_vec = []
            for i in vec:
                new_vec.append(str(i))
            return new_vec
        except:
            return []

    @classmethod
    def create_line_markers(cls,
                            plot_object,
                            x_pos,
                            description="",
                            y_pos=0.92):
        """ Create a line marker for plotObject at given x positions.

            Args:
                plot_object: The root object in which the lines should be placed
                x_pos: x position for the line on the plot_object
                description: A description text to be added next to the line.
                y_pos: Upper edge for line in y.

            Returns:
             tuple of line_elements, text_elements
            where each is a list, to account for multiple pads.
        """

        xlow_canvas = ROOT.Double(0.)
        ylow_canvas = ROOT.Double(0.)
        xup_canvas = ROOT.Double(0.)
        yup_canvas = ROOT.Double(0.)

        # First test if pads are found or only one exists
        no_pad_found = True
        for pad in plot_object.canvas.GetListOfPrimitives():
            # If Object is no pad continue
            if pad.ClassName() == "TPad":
                no_pad_found = False
        if no_pad_found:
            pads = [plot_object.canvas.GetPad(0)]
        else:
            pads = plot_object.canvas.GetListOfPrimitives()

        lines = []
        labels = []

        # Loop over all pads
        for pad in pads:
            # If Object is no pad continue
            if pad.ClassName() not in ["TPad", "TCanvas"]:
                continue

            # Get size of canvas
            pad.Modified()
            pad.Update()
            pad.GetRangeAxis(xlow_canvas, ylow_canvas, xup_canvas, yup_canvas)

            # Create line
            if pad.GetLogy():
                y_low = pow(10, ylow_canvas)
                y_up = pow(10, yup_canvas)
            else:
                y_low = ylow_canvas
                y_up = yup_canvas

            line = ROOT.TLine( x_pos, y_low, x_pos, y_up )
            line.SetNDC( False )
            lines.append( line )

            if pad.GetLogy():
                y_text = (yup_canvas - ylow_canvas) * y_pos + ylow_canvas
                y_text = pow(10, y_text)
            else:
                y_text = (yup_canvas - ylow_canvas) * y_pos + ylow_canvas

            label = ROOT.TLatex(
                    x_pos + 0.02 * (xup_canvas - xlow_canvas),
                    y_text,
                    description )
            label.SetNDC( False )
            labels.append( label )

        return lines, labels



############################################
# Collision of histograms with other objects

    @classmethod
    def generic_collision(cls, pad, box, objects):
        """ Caluclates minimal distance from new object to all other objects
            box needs to be of type TBox (e.g. TLatex, TLegend)
            objects need to be TGraph(Asymm)Errors or TH1
        """
        Coordinates = namedtuple("Coordinates", ("xlow", "xup", "ylow", "yup"))
        coordinates_canvas = get_canvas_size(pad)
        left_margin = pad.GetLeftMargin()
        right_margin = pad.GetRightMargin()
        try:
            coordinates_box = Coordinates(box.GetX1(),
                                          box.GetX2(),
                                          box.GetY1(),
                                          box.GetY2())
        except AttributeError:
            raise AttributeError("Object seems not to of type TBox")

        maximum = 1e-6
        for object in objects:
            if object is None:
                continue
            # This is sadly neccessary
            if "TGraphErrors" in object.ClassName():
                n_points = object.GetN()
                tmp = object.GetX()
                x_values = [tmp[i] for i in range(n_points)]
                tmp = object.GetEX()
                x_errors = [tmp[i] for i in range(n_points)]
                tmp = object.GetY()
                y_values = [tmp[i] for i in range(n_points)]
                tmp = object.GetEY()
                y_errors = [tmp[i] for i in range(n_points)]
            elif "TGraphAsymmErrors" in object.ClassName():
                n_points = object.GetN()
                tmp = object.GetX()
                x_values = [tmp[i] for i in range(n_points)]
                tmp = object.GetEXhigh()
                x_errors = [tmp[i] for i in range(n_points)]
                tmp = object.GetY()
                y_values = [tmp[i] for i in range(n_points)]
                tmp = object.GetEYhigh()
                y_errors = [tmp[i] for i in range(n_points)]
            elif "TH1" in object.ClassName():
                x_values = [object.GetBinCenter(ibin + 1) for ibin in range(object.GetNbinsX())]
                x_errors = [object.GetBinWidth(ibin + 1) / 2 for ibin in range(object.GetNbinsX())]
                y_values = [object.GetBinContent(ibin + 1) for ibin in range(object.GetNbinsX())]
                y_errors = [0. for ibin in range(object.GetNbinsX())]
                n_points = object.GetNbinsX()
            else:
                raise AttributeError("Object is not of type TGraph(Asymm)Errors or TH1")

            # Get Lower and Upper intersection bin
            for i in range(0, n_points):
                ndc_coord = convert_user_to_ndc(x_values[i] + x_errors[i],
                                                coordinates_canvas.xlow,
                                                coordinates_canvas.xup)
                converted_ndc = convert_ndc_axis_to_pad(ndc_coord,
                                                        left_margin,
                                                        right_margin)
                if converted_ndc > coordinates_box.xlow:
                    lower_bin = i
                    break
            for i in reversed(range(0, n_points)):
                ndc_coord = convert_user_to_ndc(x_values[i] - x_errors[i],
                                                coordinates_canvas.xlow,
                                                coordinates_canvas.xup)
                converted_ndc = convert_ndc_axis_to_pad(ndc_coord,
                                                        left_margin,
                                                        right_margin)
                if converted_ndc < coordinates_box.xup:
                    upper_bin = i
                    break

            # Get maximum value in intersection region
            for i in range(lower_bin, upper_bin + 1):
                maximum = max(maximum, y_values[i] + y_errors[i])

        is_logy = pad.GetLogy()
        tmp = maximum
        if is_logy:
            tmp = math.log10(maximum)
        else:
            tmp = maximum
        ymax_ndc_axis = (tmp - coordinates_canvas.ylow) / (coordinates_canvas.yup - coordinates_canvas.ylow)

        # Converts NDC coordinates from NDC with respect to the axis to NDC with respect to the pad
        bot_margin = pad.GetBottomMargin()
        top_margin = pad.GetTopMargin()
        ymax_ndc_pad = convert_ndc_axis_to_pad(ymax_ndc_axis,
                                               bot_margin,
                                               top_margin)
        return (ymax_ndc_pad - coordinates_box.ylow, maximum)


    @classmethod
    def get_run_hash_from_file(cls, mc_file=None, data_file=None):
        """ Return the run hash from either a mc_file or data file

            The run_hash is intended to be fixed for a set of running conditions
            and may be used to compare if two EventClass objects were created with
            the same version of the classification and ectools repository.

            Returns:
                The run_hash for the data_file or the mc_file if no tata_file was passed
        """
        the_file = data_file if data_file is not None else mc_file
        assert the_file

        empty_key = the_file.GetKey("Rec_Empty+X")
        empty_class = empty_key.ReadObj()
        run_hash = empty_class.getRunHash()
        #empty_class.Delete()
        return run_hash

    @classmethod
    def calc_run_hash(cls):
        """ Calcultate the current run hash from git repo states

            The run_hash is intended to be fixed for a set of running conditions
            and may be used to compare if two EventClass objects were created with
            the same version of the classification and ectools repository.
            This function reads the current commit hash for
            the repos MUSIC_UTILS, PXLANA and the EventClassFactory and combines them into
            one hash.

            Returns:
                The run_hash for the data_file or the mc_file if no tata_file was passed
        """
        def _get_repo_hash(repopath):
            with tmp_chdir( repopath ):
                cmd = ['git', 'symbolic-ref', 'HEAD', '--short']
                return subprocess.check_output(cmd).split('\n')[0]

        repos = (expand_path("$MUSIC_UTILS"),
                 expand_path("$PXLANA"),
                 expand_path("$PXLANA/$MYPXLANA"))

        hash_components = []
        for repo in repos:
            hash_components.append(_get_repo_hash(repo))
        return hashlib.sha1(''.join(hash_components)).hexdigest()

    @classmethod
    def calculate_new_canvas_size(cls, pad, ylow_object, maximum):
        """ Calculate the new ymax of the canvas due to collisions with objects

            Args:
                pad: The pad or canvas for which the ymax should be determined
                ylow_object: The ylow coordinated of the object to check
                maximum: The maximum y value
            Returns:
                The new ymax value
        """
        coordinates_canvas = get_canvas_size(pad)
        bot_margin = pad.GetBottomMargin()
        top_margin = pad.GetTopMargin()
        is_log = pad.GetLogy()

        # Adjust y using a smart formula (hist_ndc_axis = legend_ndc_axis)
        if is_log:
            return pow(10, (math.log10(maximum) - coordinates_canvas.ylow) / convert_ndc_pad_to_axis(ylow_object, bot_margin, top_margin) + coordinates_canvas.ylow)
        else:
            return (maximum - coordinates_canvas.ylow) / convert_ndc_pad_to_axis(ylow_object, bot_margin, top_margin) + coordinates_canvas.ylow

    @classmethod
    def ec_loop(cls,
                conf,
                func,
                skip_empty=False,
                setup_root=True,
                distribution=None,
                **kwargs):
        """ Run a generic loop over all (possibly filtered) classes in a root file containing TEventClass objects and call the passed function with func.

            This function runs a generic loop as needed by many CLI tools and workflows
            to run actions defined in a function on a per event class basis on all event classes
            in a set of mc, data and possibly signal files. This loop performes a set of standard
            preparation steps for the class names and provides an option to parallize the loop.

            The list of classes is filtered using the prepare_class_names function

            Args:
                conf: An ec_conf namespace object containing at least the arguments defines in add_ec_standard_options
                func: A python function which should be called on any class.
                skip_empty: Do not run the passed function on the Rec_Empty+X class
                setup_root: Load ROOT with TEventClass shared object libraries before the loop if true
                **kwargs: Arbitrary keyword arguments which are passed to func

            Example:
                Suppose you want to collect a list of class names from a mc_file passed via the ec_standard_options
                To do so define a funtion which acts on a single class and call the ec_loop function
                blocks::

                    def get_class_name(conf, mc_ec=None):
                        return mc_ec.GetName()

                    names = ecroot.ec_loop(conf, get_class_name)


            Note:
                The ecout option to write TEventClass which were altered in the passed function can not be combined with more than one parallel job
        """
        # User has to provide at least one of MC or data.
        if not conf.mc and not conf.data:
            raise ValueError("One of MC or data has to be provided.")

        # Load the eventclass library into nthe ROOT interpreter.
        if setup_root:
            root_setup()

        # strip output ec file name
        if conf.ecout:
            conf.ecout = rremove(conf.ecout, ".root")

        mc_file, data_file = cls.read_root_files(conf)

        signal_file = None
        if conf.signal:
            signal_file = cls.read_root_file(conf.signal)

        if conf.ecout and conf.jobs > 1:
            conf.jobs = 1
            warn = "Unable to use ecout option and multiprocessing."
            warn += "Switching to single core mode."
            print(warn)

        if conf.ecout and mc_file:
            mc_outfile = ROOT.TFile(conf.ecout + "_mc.root", 'RECREATE')

        if conf.ecout and data_file:
            data_outfile = ROOT.TFile(conf.ecout + "_data.root", 'RECREATE')

        results = []
        jobs = []
        if conf.jobs > 1:
            pool = multiprocessing.Pool(conf.jobs)

        # try ... finally closes the files on error.
        try:
            # get default lumi from data using Rec_Empty+X (always filled)
            the_file = data_file if data_file is not None else mc_file
            assert the_file

            empty_key = the_file.GetKey("Rec_Empty+X")
            empty_class = empty_key.ReadObj()

            if conf.ecout and mc_file:
                if data_file:
                    empty_key_mc = mc_file.GetKey("Rec_Empty+X")
                    empty_class_mc = empty_key_mc.ReadObj()
                    mc_outfile.cd()
                    empty_class_mc.Write()
                else:
                    mc_outfile.cd()
                    empty_class.Write()

            if conf.ecout and data_file:
                data_outfile.cd()
                empty_class.Write()

            default_lumi = empty_class.getLumi()
            # First, gather all possible event class names from both files.
            names, names_vetoed = cls.prepare_class_names(conf,
                                                          mc_file=mc_file,
                                                          data_file=data_file,
                                                          distribution=distribution)

            # function should have access to total number of ECs
            kwargs["N"] = len(names)
            # Loop through all possible event class names.
            for i, name in enumerate(sorted(names)):

                # Skip the '...Empty' distribution, it's not physical.
                if not (skip_empty and 'Empty' in name):
                    # Update iterator count and event classes
                    kwargs["i"] = i
                    kwargs["default_lumi"] = default_lumi
                    kwargs["distribution"] = distribution
                    # call chosen function
                    if conf.jobs > 1:
                        args = (conf, func, name)
                        jobs.append(pool.apply_async(_ec_loop_multi,
                                                     args,
                                                     kwargs.copy()))
                    else:
                        mc_ec, data_ec = cls.read_ec_objects(mc_file,
                                                             data_file,
                                                             name)
                        if signal_file:
                            signal_ec = cls.read_ec_object(signal_file, name)
                            kwargs["signal_ec"] = signal_ec
                        kwargs["data_ec"] = data_ec
                        kwargs["mc_ec"] = mc_ec
                        results.append(func(conf, **kwargs))

                if conf.jobs > 1:
                    continue
                # Delete event classes because ROOT doesn't do it itself.
                if data_ec:
                    if conf.ecout:
                        data_outfile.cd()
                        data_ec.Write()
                    #data_ec.Delete()
                if mc_ec:
                    if conf.ecout:
                        mc_outfile.cd()
                        mc_ec.Write()
                    #mc_ec.Delete()

            if conf.keep_vetoed:
                for i, name in enumerate(sorted(names_vetoed)):
                    mc_ec, data_ec = cls.read_ec_objects(mc_file,
                                                         data_file,
                                                         name)
                    if data_ec:
                        if conf.ecout:
                            data_outfile.cd()
                            data_ec.Write()
                        #data_ec.Delete()
                    if mc_ec:
                        if conf.ecout:
                            mc_outfile.cd()
                            mc_ec.Write()
                        #mc_ec.Delete()

        finally:
            # try to get results from pool if multiporcessing is used
            if conf.jobs > 1:
                for job in jobs:
                    try:
                        res = job.get(1200)
                        results.append(res)
                    except multiprocessing.TimeoutError:
                        print("Timeout for one class")
            # In any case, close those ROOT files!
            if mc_file:
                mc_file.Close()
            if data_file:
                data_file.Close()
        return results

    @classmethod
    def get_distribution_width_weighted(cls, hist, systname, yield_histo=None, bin_min_yield=0.0001):
        ''' Returns bin edges and bin width weighted frequencies.

            This is e.g. used to get the distribution of relative uncertainties from a
            histogram of relative uncertaintes with variable binning.
            the yield_histo can be used to make sure that only bins with a certain
            bin_min_yield in the reference histogram are considered

            Args:
                hist: The systematic histogram containing relative uncertainties
                systname: string contauning the name of the systematic
                yield_histo: A histogram with the same binning as the hist argument containing total yield to determine if a bin should be skipped based on the min_yield argument
                bin_min_yield: Minimum yield for bins in the yield_histo for a bin in hist to be considered for the uncertainty distribution

            Returns:
                bin_edges, frequencies as numpy array where frequecies contains the frequency with which the uncert is observered to have a relative value within the bin_edges
        '''

        max_rel_deviation = 10.
        tmp_dist = ro.TH1F("sys uncert dist %s" %systname, "dist of %s uncerts;uncert;freq" % systname,
                           int(max_rel_deviation * 100),
                           0.0,
                           max_rel_deviation)
        for ibin in range(hist.GetNbinsX()):
            # Fill each bin with width as weight to estimate part of x-axis
            if hist.GetBinContent( ibin+1 ) > 0.001:
                if yield_histo.GetBinContent(ibin + 1) > bin_min_yield:
                    tmp_dist.Fill( hist.GetBinContent(ibin + 1), hist.GetBinWidth(ibin + 1))
        # normalize distributions to unity
        if tmp_dist.Integral() > 0.:
            tmp_dist.Scale(1. / tmp_dist.Integral())
            bin_edges, frequencies = root_hist2arrays( tmp_dist, bin_center=True )
        else:
            bin_edges, frequencies = [], []
        return np.array(bin_edges), np.array(frequencies)

    @classmethod
    def ec_root_class_type_loop(cls,
                           func,
                           conf,
                           mc_file=None,
                           data_file=None,
                           **kwargs):
        """ Run a loop splitted by ec class type

            Extracts class names from data and mc file using the prepare_class_names functions and
            processes each found class with the passed function grouped by their class type.

            Args:
                func: calls func(conf, class_type_name, class_names, **kwargs)
                conf: An ec_conf namespace object containing at least the arguments defines in add_ec_standard_options
                mc_file: A root TFile object containing TEventClass objects for MC
                data_file: A root TFile object containing TEventClass objects for Data
                **kwargs: Arbitrary keyword arguments which are passed to func

            Returns:
                A dictionary with the class type name as key and the output of the passed func for classes with this class type.
        """
        class_type_results = {}
        ec_class_types = []
        # Check if configuration filters for one classs type
        for class_type in cls.class_types():
            if getattr(conf, "filter_" + class_type.name.replace("-", "_")):
                ec_class_types = [class_type]
                break
            ec_class_types.append(class_type)
        all_kwargs = cls.extra_kwargs(conf)
        all_kwargs.update(kwargs)
        for class_type in ec_class_types:
            class_type_option = "filter_" + class_type.name.replace("-", "_")
            setattr(conf, class_type_option, True)
            class_names, dropped = cls.prepare_class_names(conf,
                                                  mc_file=mc_file,
                                                  data_file=data_file,
                                                  **all_kwargs)
            class_type_results[class_type.name] = func(conf,
                                                       class_type.name,
                                                       class_names,
                                                       **all_kwargs)
            setattr(conf, class_type_option, False)
        return class_type_results

    @classmethod
    def get_class_yields(cls,
                         conf,
                         distribution='SumPt',
                         min_yield=0.,
                         mc_ec=None,
                         **kwargs):
        """ Get the name and total yield for an event class

            Extracts class names and total yields from a mc file for a given distribution.
            Returns an empty tuple if class contains none of the required or a rejected ec object type.

            Args:
                conf: An ec_conf namespace object containing at least the arguments defines in add_ec_standard_options
                distribution: name of the ECDistributionType for which the yield should be determined.
                mc_ec: A TEventClass objects for MC
                min_yield: return an empty '', 0 tuple if the yield is less than the given value

            Returns:
               A tuple with (class_name, total_yield)
        """
        try:
            total_hist = cls.total_event_yield(mc_ec, distribution)
        except:
            return '' , 0
        name = mc_ec.GetName()
        if any(n in name for n in cls.rejected_object_types()):
            #total_hist.Delete()
            return '', 0
        if not any(n in name for n in cls.required_object_types()):
            #total_hist.Delete()
            return '', 0
        integral = total_hist.Integral()
        #total_hist.Delete()
        return name, integral

    @classmethod
    def add_ec_standard_options(cls,
                                parser,
                                add_ecout_option=True,
                                add_parent=True,
                                **kwargs):
        """ Add standard options for programs running over TEventClass containing root files

            Args:
                parser: A argparse ArgumentParser instance.
                add_ecout_option: Add options to save changed eventclasses back to new output file.
                add_parent: Add the options from the parent class tho this helper.
        """
        kwargs["add_ecout_option"] = add_ecout_option
        kwargs["add_parent"] = add_parent
        if add_parent:
            parent = super(ECRootHelper, cls)
            parent.add_ec_standard_options(parser, **kwargs)

        h = "ROOT file containing TEventClasses from Monte-Carlo simulation."
        parser.add_argument("--mc",
                            help=h,
                            type=str,
                            default=None)
        h = "ROOT file containing a TEventClass, filled with Data events."
        parser.add_argument("--data",
                            help=h,
                            type=str,
                            default=None)
        h = "ROOT file containing a TEventClass, filled with Signal events."
        parser.add_argument("--signal",
                            help=h,
                            type=str,
                            default=None)
        h = "Number of parallel jobs used while processing event classes.."
        parser.add_argument("-j", "--jobs",
                            help=h,
                            type=int,
                            default=1)

        if add_ecout_option:
            h = "Save EventClasses in new file after working on it."
            h += "A suffix for mc / data is added automatically."
            h += "The .root extension can be omitted."
            parser.add_argument("--ecout",
                                help=h,
                                type=str,
                                default=None,)

    ############

    # TeX Header
    @staticmethod
    def create_tex_header(canvas,
                        header_title="CMS",
                        header_subtitle="Work in Progress",
                        header_outside=False,
                        text_size_header=0.05,
                        text_size_info=0.04,
                        lumi=35900,
                        cme=13000,
                        dataset_info_outside=True,
                        extra_info_text="",
                        ):
        """ Create a standard TeX header on a root canvas

            Args:
                canvas: A TCanvas root object.
                header_tilte: The main title to display.
                header_subtitle: The header subtitle
                text_size_header: Float with size parameter passed to TLatex.SetTextSize
                text_size_info: Float with size parameter passed to TLatex.SetTextSize
                lumi: The luminosity to disply
                cme: The center of mass energy
                dataset_info_outside: Flag to control if lumi and cme should be displayed inside the plot
                extra_info_text: A string containing an additional info string to display next / below the main header and subtitle

            Returns:
                A list of Root TLatex objects which need to be drwan to diplay all header elements.
        """
        canvas.cd()
        header_text = ""
        subheader_text = ""
        if header_title:
            header_text += "#bf{%s}" % header_title
        if header_subtitle:
            if header_text and header_outside:
                header_text += " "
            if header_outside:
                header_text += "#it{%s}" % header_subtitle
            else:
                subheader_text += "#it{%s}" % header_subtitle
        text_position_y_info = 1 - text_size_info - 0.001
        margin_left = canvas.GetLeftMargin()
        margin_top = canvas.GetTopMargin()

        margin_right = canvas.GetRightMargin()

        used_above = 0
        objs = []
        if header_text:
            if header_outside:
                x, y = margin_left, text_position_y_info
                align = TextAlign.BOTTOM_LEFT
            else:
                x, y = margin_left + 0.04, 1 - margin_top - 0.04
                align = TextAlign.TOP_LEFT
                used_above = text_size_header

            assert(0 <= x <= 1)
            assert(0 <= y <= 1)

            title_element = ro.TLatex(x, y, header_text)
            title_element.SetTextAlign(align)
            title_element.SetNDC()
            title_element.SetTextSize(text_size_header)
            title_element.SetName("title_element")
            title_element.SetTextFont(Font.SANS)
            objs.append(title_element)

        if subheader_text and not header_outside:
            x, y = margin_left + 0.04, 1 - margin_top - 0.04 - used_above
            align = TextAlign.TOP_LEFT
            used_above += text_size_header - 0.015
            assert(0 <= x <= 1)
            assert(0 <= y <= 1)

            subtitle_element = ro.TLatex(x, y, subheader_text)
            subtitle_element.SetTextAlign(align)
            subtitle_element.SetNDC()
            subtitle_element.SetTextSize(text_size_header - 0.01)
            subtitle_element.SetName("subtitle_element")
            subtitle_element.SetTextFont(Font.SANS)
            objs.append(subtitle_element)

        # Lumi, sqrt(s)
        if dataset_info_outside:
            x, y = 1 - margin_right, text_position_y_info
            align = TextAlign.BOTTOM_RIGHT
        else:
            x, y = margin_left + 0.04, 1 - margin_top - 0.05 - used_above
            align = TextAlign.TOP_LEFT
            used_above += text_size_info + 0.015

        dataset_info_text = "%.1f#kern[0.05]{fb^{-1}} (%d TeV)" % (lumi * 0.001, int(cme / 1000.)) ##sqrt{s} =
        dataset_info = ro.TLatex(x, y, dataset_info_text)
        dataset_info.SetNDC()
        dataset_info.SetTextSize(text_size_info)
        dataset_info.SetTextAlign(align)
        dataset_info.SetTextFont(Font.SANS)
        dataset_info.SetName("dataset_info")
        objs.append(dataset_info)

        # extry info
        if extra_info_text:
            if dataset_info_outside:
                x, y = 1 - margin_right * 4, text_position_y_info
                align = TextAlign.BOTTOM_RIGHT
            else:
                x, y = margin_left + 0.04, 1 - margin_top - 0.04 - used_above
                align = TextAlign.TOP_LEFT

            extra_info = ro.TLatex(x, y, extra_info_text)
            extra_info.SetNDC()
            extra_info.SetTextSize(text_size_info)
            extra_info.SetTextAlign(align)
            extra_info.SetTextFont(Font.SANS)
            extra_info.SetName("extra_info")
            objs.append(extra_info)

        return objs

def _ec_loop_multi(conf, func, name, **kwargs):
    ''' Init function for parallelized ec_loop '''
    mc_file, data_file = ECRootHelper.read_root_files(conf)
    mc_ec, data_ec = ECRootHelper.read_ec_objects(mc_file, data_file, name)
    if conf.signal:
        signal_file = ECRootHelper.read_root_file(conf.signal)
        signal_ec = ECRootHelper.read_ec_object(signal_file, name)
        kwargs["signal_ec"] = signal_ec
    kwargs["data_ec"] = data_ec
    kwargs["mc_ec"] = mc_ec


    return func(conf, **kwargs)


def ec_result_meta_dump(rdict):
    m = {"event_classes" : {},
         "eventclasses_distributions" : {}}
    return m
