from __future__ import print_function

import os.path
import ROOT as ro
import array

from ectools.register import ecroot

from cmsstyle import *
from .general import GeneralPlots
from roothelpers import Color, LineStyle, get_dummy_hist
import dbtools


class ECDistributionPlot(GeneralPlots):
    def __init__(
        self,
        config,
        distribution,
        data_ec=None,
        mc_ec=None,
        signal_ec=None,
        session=None,
    ):
        # Store the arguments in instance-scope.

        ec_name = data_ec.GetName() if data_ec is not None else mc_ec.GetName()
        self.signal_ec = signal_ec

        # Call init of base class
        super(ECDistributionPlot, self).__init__(
            config, distribution, data_ec=data_ec, mc_ec=mc_ec, session=session
        )
        # Store options in instance-scope.
        # The ratio plot is only created if requested AND data and MC is given.
        self.opt_ratio = (
            (data_ec is not None) and (mc_ec is not None) and self.config.ratio
        )

        self.opt_ratio_error = (mc_ec is not None) and self.config.plot_error_ratio
        self.opt_error_band = (mc_ec is not None) and self.config.plot_error

        self.min_yield = config.min_yield

        self._pad1a = None
        self._pad1b = None
        self.mc_stack = None
        self.systematics = None

        self.canvas
        # hists
        self.ratio = None

        self.data_graph = None
        self.error_band = None

        self.roi_line_lower_edge = []
        self.roi_line_upper_edge = []

        set_cms_root_style()

        self.init_text_sizes()

        self.legend = None

        self.skipreason = ""

        # Do all backgrounds have negative contribution?
        if "mc" not in self.plot_case:
            self.all_mc_negative = False
        else:
            # MC exists: start with all_mc_negative = True,
            # will be set to False by GeneralPlots.create_mc_stack()
            # once a positive contribution has been found.
            self.all_mc_negative = True

        self.plot()
        if self.skip:
            return
        if self.opt_ratio:
            self.plot_ratio()

        self.label_axis(self.dummy_hist, ratio=self.opt_ratio)

        if self.opt_ratio:
            self.scale_axis_label_size_of_ratio()

        self.create_legend()
        self.draw_legend()
        self.draw_tex_header()

        self.tex_header_collision()
        self.resize_y_axis_to_legend()

        if self.config.scan_id and not self.p_tilde:
            self.skip = True
            self.skipreason = "No p-tilde"
            return

        self.plot_roi_markers()

    @classmethod
    def get_plot_name(
        cls, config, ec_name="Rec_undefined", distribution="Dist", **kwargs
    ):
        if config.scan_id:
            return ec_name + "_withRoI_" + distribution
        return ec_name + distribution

    @classmethod
    def get_subfolder(cls, config, ec_name="Rec_Empty", distribution="dist"):
        key = distribution
        prefix, objects, class_tag = ecroot.split_ec_name(ec_name)
        ec_base_name = ecroot.join_ec_name(objects, "")
        if config.cumulative:
            key += "-cumulative"
        return os.path.join("EventClass", ec_base_name, key)

    @property
    def canvas(self):
        if self._canvas is None:
            if self.opt_ratio:
                # create canvas
                self._canvas = ro.TCanvas(
                    self.ec_name + self.distribution,
                    "All BG syst" + self.ec_name + self.distribution,
                    self.config.canvas_width,
                    self.config.canvas_height,
                )
                self._canvas.UseCurrentStyle()

                self.pad1a.Draw()
                self.pad1b.Draw()
                self.pad1a.cd()
            else:
                self._canvas = ro.TCanvas(
                    self.ec_name + self.distribution,
                    "All BG syst" + self.ec_name + self.distribution,
                    self.config.canvas_width,
                    int(self.config.canvas_height * 0.8),
                )
                self._canvas.UseCurrentStyle()
        return self._canvas

    @property
    def pad1a(self):
        if self._pad1a is None:
            self._pad1a = ro.TPad("pad1a", "pad1a", 0, 0.25, 1, 1)
            self._pad1a.SetBottomMargin(0.05)
        return self._pad1a

    @property
    def pad1b(self):
        if self._pad1b is None:
            self._pad1b = ro.TPad("pad1b", "pad1b", 0, 0.0, 1, 0.248)
            self._pad1b.SetBottomMargin(0.33)
        return self._pad1b

    def cleanup(self):
        # delete stuff
        if hasattr(self, "_pad1a") and self._pad1a is not None:
            self._pad1a.Close()
        if hasattr(self, "_pad1b") and self._pad1b is not None:
            self._pad1b.Close()
        self.canvas.Close()
        if "mc" in self.plot_case:
            self.mc_stack.Delete()
            self.total_mc_hist.Delete()
            self.total_mc_hist_unscaled.Delete()
            if self.opt_error_band:
                if self.systematics:
                    self.systematics.Delete()
                    self.systematics_unscaled.Delete()
        if "data" in self.plot_case:
            self.data_hist.Delete()
            self.data_hist_unscaled.Delete()
        if self.opt_ratio:
            if self.opt_ratio_error == "band":
                self.ratio_error_band.Delete()
            elif self.opt_ratio_error == "total":
                self.ratio_error_total.Delete()
            self.ratio.Delete()
            self.ratio_den.Delete()
        self._canvas = None

    def plot(self):
        self.canvas.cd()
        if self.opt_ratio:
            self.pad1a.cd()
        self.create_input_layers()
        if self.skip:
            return
        self.dummy_hist = get_dummy_hist(
            [self.mc_stack, self.signal_stack, self.data_hist]
        )
        self.dummy_hist.Draw()
        self.set_log_axis()
        if "mc" in self.plot_case:
            options = "hist same"
            self.mc_stack.Draw(options)
            self.mc_stack.GetXaxis()

            if self.opt_error_band:
                self.create_error_band()
                self.error_band.Draw("2same")
        if "signal" in self.plot_case:
            # options = "L nostack"
            options = "hist nostack"
            options += " same"
            self.signal_stack.Draw(options)
        if "data" in self.plot_case:
            options = "E0P"
            options += " same"
            self.data_graph.Draw(options)

        self.determine_x_axis_range()
        self.adjust_axes_to_histogram()
        self.dummy_hist.Draw("sameaxis")
        return

    def create_error_band(self):
        """Plot the MC systematic uncertainties, centered around the MC values."""
        self.systematics = ecroot.total_combined_uncertainty(
            self.mc_ec,
            self.distribution,
            filter_systematics=self.config.filter_systematics,
            with_stat=True,
        )

        if not self.systematics:
            return
        self.systematics = self.rescale_mc_to_data(self.systematics)
        self.systematics_unscaled = self.systematics.Clone()
        if self.config.scale_to_area:
            self.systematics.Scale(self.min_bin_width, "width")

        # create cumulative plot if option is used
        if self.config.cumulative:
            sum = 0
            for ibin in reversed(xrange(self.systematics.GetNbinsX())):
                sum += (
                    self.systematics.GetBinContent(ibin + 1)
                    * self.systematics.GetBinWidth(ibin + 1)
                    / self.min_bin_width
                )
                self.systematics.SetBinContent(ibin + 1, sum)

        mc_x_arr, mc_y_arr = root_hist2arrays(self.total_mc_hist)
        syst_x_arr, syst_y_arr = root_hist2arrays(self.systematics)

        # set error to half bin width (is extended symmetrically in both directions)
        bin_width_arr = array.array(
            "f",
            [
                self.total_mc_hist.GetBinWidth(ibin + 1) / 2.0
                for ibin in range(self.total_mc_hist.GetNbinsX())
            ],
        )
        self.error_band = ro.TGraphErrors(
            len(mc_x_arr), mc_x_arr, mc_y_arr, bin_width_arr, syst_y_arr
        )

        self.error_band.SetFillColor(35)
        self.error_band.SetFillStyle(3244)  # 3244 = hatched

    def plot_ratio(self):
        self.canvas.cd()
        self.pad1b.cd()

        self.pad1b.SetLogy(self.config.ratio_scale)

        # error band
        # optional rebinning
        if self.config.ratio_rebin:
            # find bin borders where cobined numer of mc adds up to one event
            bin_borders = array.array("d")
            bin_borders.append(self.total_mc_hist.GetXaxis().GetBinLowEdge(1))
            local_bin_sum = 0.0
            first_bin = True
            for ibin in range(1, self.total_mc_hist_unscaled.GetNbinsX() + 1):
                # Begin one bin at lower part of plotting range
                if (
                    self.total_mc_hist_unscaled.GetXaxis().GetBinUpEdge(ibin)
                    > self.xlow
                    and first_bin
                ):
                    if self.xlow > 0:
                        bin_borders.append(
                            self.total_mc_hist_unscaled.GetXaxis().GetBinLowEdge(ibin)
                        )
                    first_bin = False

                local_bin_sum += self.total_mc_hist_unscaled.GetBinContent(ibin)
                if (
                    local_bin_sum > 1.0
                    and self.data_hist_unscaled.GetBinContent(ibin) > 0
                ):
                    bin_borders.append(
                        self.total_mc_hist_unscaled.GetXaxis().GetBinUpEdge(ibin)
                    )
                    append_last_bin = False
                    local_bin_sum = 0.0
                else:
                    append_last_bin = True

                # cutoff rebinning at upper edge of plotting range
                if (
                    self.total_mc_hist_unscaled.GetXaxis().GetBinUpEdge(ibin)
                    == self.xup
                ):
                    if not append_last_bin:
                        bin_borders[
                            -1
                        ] = self.total_mc_hist_unscaled.GetXaxis().GetBinUpEdge(ibin)
                    else:
                        bin_borders.append(
                            self.total_mc_hist_unscaled.GetXaxis().GetBinUpEdge(ibin)
                        )
                    break

            bin_borders.append(
                self.total_mc_hist_unscaled.GetXaxis().GetBinUpEdge(
                    self.total_mc_hist_unscaled.GetXaxis().GetLast()
                )
            )

            self.ratio = self.data_hist_unscaled.Rebin(
                len(bin_borders) - 1, "nominator", bin_borders
            )
            self.ratio_den = self.total_mc_hist_unscaled.Rebin(
                len(bin_borders) - 1, "denominator", bin_borders
            )

            shift_hist = self.systematics_unscaled.Rebin(
                len(bin_borders) - 1, "denominator_syst_shift", bin_borders
            )
        else:  # Just clone histos if we do not want to rebin.
            self.ratio = self.data_hist_unscaled.Clone()
            self.ratio_den = self.total_mc_hist_unscaled.Clone()
            shift_hist = self.systematics_unscaled.Clone()

        ratio_x, ratio_y, ratio_x_err, ratio_y_err = (
            array.array("f"),
            array.array("f"),
            array.array("f"),
            array.array("f"),
        )
        for ibin in range(1, self.ratio.GetNbinsX() + 1):
            ratio_x.append(self.ratio.GetBinCenter(ibin))
            ratio_y.append(1.0)
            ratio_x_err.append(self.ratio.GetBinWidth(ibin) / 2)

        if self.opt_ratio_error == "band":
            for ibin in range(1, self.ratio.GetNbinsX() + 1):
                try:
                    ratio_y_err.append(
                        shift_hist.GetBinContent(ibin)
                        / self.ratio_den.GetBinContent(ibin)
                    )
                    self.ratio.SetBinContent(
                        ibin,
                        self.ratio.GetBinContent(ibin)
                        / self.ratio_den.GetBinContent(ibin),
                    )
                    self.ratio.SetBinError(
                        ibin,
                        self.ratio.GetBinError(ibin)
                        / self.ratio_den.GetBinContent(ibin),
                    )
                except ZeroDivisionError:
                    ratio_y_err.append(0.0)
                    self.ratio.SetBinContent(ibin, -1.0)
                    self.ratio.SetBinError(ibin, 0.0)

            self.ratio_error_band = ro.TGraphErrors(
                len(ratio_x), ratio_x, ratio_y, ratio_x_err, ratio_y_err
            )
            self.ratio_error_band.SetFillColor(35)
            self.ratio_error_band.SetFillStyle(3244)

        elif self.opt_ratio_error == "total":
            self.ratio_error_total = ro.TGraphAsymmErrors()
            ratio_syst = self.ratio_den.Clone()
            for ibin in range(1, self.ratio_den.GetNbinsX() + 1):
                ratio_syst.SetBinError(ibin, shift_hist.GetBinError(ibin))
            self.ratio_error_total.Divide(self.ratio, ratio_syst, "pois midp")
            ratio_syst.Delete()

        # find y plotting range
        y_min = 999.0
        y_max = 0.0

        if self.opt_ratio_error == "band":
            if self.xup != self.cme:
                max_range_axis = self.ratio.GetNbinsX() - 1
            else:
                max_range_axis = self.ratio.GetNbinsX()
            if self.xup != 0:
                min_range_axis = 1
            else:
                min_range_axis = 0

            # data points
            for ibin in range(min_range_axis, max_range_axis):
                ct = self.ratio.GetBinContent(ibin + 1)
                er = self.ratio.GetBinError(ibin + 1)
                if (ct + er) > y_max:
                    y_max = ct + er
                if (ct - er) < y_min and ct >= 0:
                    y_min = ct - er

        if y_min > 0.9:
            y_min = 0.9
        if y_max < 1.1:
            y_max = 1.1

        self.ratio.UseCurrentStyle()
        if self.opt_ratio_error == "band":
            self.ratio.Draw("PE")
            self.ratio_error_band.Draw("2same")
            self.ratio.Draw("PEsame")
            self.ratio.GetXaxis().SetRangeUser(self.xlow, self.xup)
            self.ratio.GetYaxis().SetTitle("Data / MC")
        elif self.opt_ratio_error == "total":
            self.ratio_error_total.Draw("APE")
            self.ratio_error_total.GetXaxis().SetRangeUser(self.xlow, self.xup)
            self.ratio_error_total.GetYaxis().SetTitle("Data / MC")

        self.pad1b.Update()

        if self.opt_ratio_error == "total":
            ratio_plot = self.ratio_error_total
            nbins = len(ratio_y)
        else:
            self.ratio.SetMaximum(y_max * 1.08)
            self.ratio.SetMinimum(y_min * 0.95)
            ratio_plot = self.ratio
            nbins = self.ratio.GetNbinsX()

        linemin = linemax = 0
        for ibin in range(1, nbins + 1):
            if (
                ratio_plot.GetXaxis().GetBinLowEdge(ibin) <= self.xlow
                and ratio_plot.GetXaxis().GetBinUpEdge(ibin) >= self.xlow
            ):
                linemin = ratio_plot.GetXaxis().GetBinLowEdge(ibin)
            if (
                ratio_plot.GetXaxis().GetBinUpEdge(ibin) >= self.xup
                and ratio_plot.GetXaxis().GetBinLowEdge(ibin) < self.xup
            ):
                linemax = ratio_plot.GetXaxis().GetBinUpEdge(ibin)

        ratio_plot.GetYaxis().SetNdivisions(505, True)
        ratio_plot.SetLineWidth(1)

        self.line = ro.TLine(linemin, 1, linemax, 1)
        self.line.Draw()

        shift_hist.Delete()

    def create_input_layers(self):
        self.skip = True

        if "data" in self.plot_case:
            self.create_data_graph()
            if self.total_n_data > 0.0:
                self.skip = False
        if "mc" in self.plot_case:
            self.create_mc_stack("mc")
            name, integral = ecroot.get_class_yields(
                self.config,
                distribution=self.distribution,
                min_yield=self.min_yield,
                mc_ec=self.mc_ec,
            )
            if integral > self.min_yield:
                self.skip = False
        if "signal" in self.plot_case:
            self.create_mc_stack("signal")
            if self.total_signal_hist.Integral() > self.min_yield:
                self.skip = False
        if self.skip:
            self.skipreason += "min yield"

    def create_data_graph(self):
        if self.data_hist is None:
            self.create_data_hist()

        self.data_graph = ro.TGraphAsymmErrors(self.data_hist)
        self.data_graph.UseCurrentStyle()
        self.data_graph.SetLineWidth(1)
        # following recipe from https://twiki.cern.ch/twiki/bin/view/CMS/PoissonErrorBars
        for ibin in range(self.data_graph.GetN()):
            if self.config.scale_to_area:
                scalefac = self.data_hist.GetBinWidth(ibin + 1) / self.min_bin_width
            else:
                scalefac = 1.0
            N = int(round(self.data_hist.GetBinContent(ibin + 1) * scalefac, 0))
            ci_up = ro.Math.gamma_quantile_c(self.config.data_err_alpha / 2, N + 1, 1)
            if N < 1:
                ci_low = 0
                ci_up = N  # this is in principle other than recommended
            else:
                ci_low = ro.Math.gamma_quantile(self.config.data_err_alpha / 2, N, 1.0)

            self.data_graph.SetPointEYlow(ibin, (N - ci_low) / scalefac)
            self.data_graph.SetPointEYhigh(ibin, (ci_up - N) / scalefac)

    # Dont set logy if all background contributions are negative
    def set_log_axis(self):
        if self.all_mc_negative:
            # Set position of exponent
            ro.TGaxis.SetExponentOffset(-0.07, 0.00, "y")
            if self.opt_ratio:
                self.pad1a.SetLogy(0)
            else:
                self.canvas.SetLogy(0)
        else:
            if self.opt_ratio:
                self.pad1a.SetLogy(1)
            else:
                self.canvas.SetLogy(1)

    def adjust_axes_to_histogram(self):
        # determine and set x range first, this allows to determine
        # the y range in the x range easily with root
        if "mc" in self.plot_case:
            self.mc_stack.GetXaxis().SetRangeUser(self.xlow, self.xup)

        if "data" in self.plot_case:
            self.data_hist.GetXaxis().SetRangeUser(self.xlow, self.xup)
            self.data_graph.GetXaxis().SetRangeUser(self.xlow, self.xup)

        self.dummy_hist.GetXaxis().SetRangeUser(self.xlow, self.xup)
        if self.config.ymin:
            ymin = self.config.ymin
        else:
            ymin = 1e22

            if "data" in self.plot_case:
                x_vals = self.data_graph.GetX()
                y_vals = self.data_graph.GetY()
                data_vals = []
                for i in range(self.data_graph.GetN()):
                    if x_vals[i] < self.xlow:
                        continue
                    if y_vals[i] <= 0:
                        continue
                    if x_vals[i] > self.xup:
                        break
                    data_vals.append(y_vals[i] - self.data_graph.GetErrorYlow(i))
                data_min = min(data_vals)
                if self.total_mc_hist.Integral() < 1:
                    ymin = min(1e-3, ymin)
                ymin = min(ymin, data_min)

            if "mc" in self.plot_case:
                ymin_bin = self.total_mc_hist.GetMinimumBin()
                mc_min = self.total_mc_hist.GetBinContent(ymin_bin)

                ymax_bin = self.total_mc_hist.GetMaximumBin()
                mc_max = self.total_mc_hist.GetBinContent(ymax_bin)

                ymin = min(ymin, mc_min, 0.01 * mc_max)

            if not self.all_mc_negative:
                if ymin <= 0:
                    ymin = min(0.01 * mc_max, 0.01)

        if self.config.ymax:
            ymax = self.config.ymax
        else:
            ymax = 1.05 * self.max_y

        self.dummy_hist.SetMinimum(ymin)
        self.dummy_hist.SetMaximum(ymax)

    def resize_y_axis_to_legend(self):
        if self.config.ymax is None:
            self.dummy_hist.SetMaximum(self.max_y * 1.05)

    def scale_axis_label_size_of_ratio(self):
        # Get sizes of both pads and scale font size by size_pad_1 / size_pad_2
        size_pad_1 = self.pad1a.GetWh() * self.pad1a.GetAbsHNDC()
        size_pad_2 = self.pad1b.GetWh() * self.pad1b.GetAbsHNDC()

        if self.opt_ratio_error == "total":
            self.ratio_error_total.GetXaxis().SetTitleSize(
                self.ratio_error_total.GetXaxis().GetTitleSize()
                * size_pad_1
                / size_pad_2
            )
            self.ratio_error_total.GetYaxis().SetTitleOffset(
                self.ratio_error_total.GetYaxis().GetTitleOffset()
                / (size_pad_1 / size_pad_2)
            )
            self.ratio_error_total.GetYaxis().SetTitleSize(
                self.ratio_error_total.GetYaxis().GetTitleSize()
                * size_pad_1
                / size_pad_2
            )
            self.ratio_error_total.GetXaxis().SetLabelSize(
                self.ratio_error_total.GetXaxis().GetLabelSize()
                * size_pad_1
                / size_pad_2
            )
            self.ratio_error_total.GetYaxis().SetLabelSize(
                self.ratio_error_total.GetYaxis().GetLabelSize()
                * size_pad_1
                / size_pad_2
            )
            self.ratio_error_total.GetXaxis().SetTickLength(
                self.ratio_error_total.GetXaxis().GetTickLength()
                * size_pad_1
                / size_pad_2
            )
        else:
            self.ratio.GetXaxis().SetTitleSize(
                self.ratio.GetXaxis().GetTitleSize() * size_pad_1 / size_pad_2
            )
            self.ratio.GetYaxis().SetTitleOffset(
                self.ratio.GetYaxis().GetTitleOffset() / (size_pad_1 / size_pad_2)
            )
            self.ratio.GetYaxis().SetTitleSize(
                self.ratio.GetYaxis().GetTitleSize() * size_pad_1 / size_pad_2
            )
            self.ratio.GetXaxis().SetLabelSize(
                self.ratio.GetXaxis().GetLabelSize() * size_pad_1 / size_pad_2
            )
            self.ratio.GetYaxis().SetLabelSize(
                self.ratio.GetYaxis().GetLabelSize() * size_pad_1 / size_pad_2
            )
            self.ratio.GetXaxis().SetTickLength(
                self.ratio.GetXaxis().GetTickLength() * size_pad_1 / size_pad_2
            )

    def legend_collision(self):
        box = ro.TBox(
            self.xlow_legend, self.ylow_legend, self.xup_legend, self.yup_legend
        )
        if self.signal_stack:
            signal_hists = [hist for hist in self.signal_stack.GetHists()]
        else:
            signal_hists = []
        return ecroot.generic_collision(
            self.the_pad,
            box,
            [self.total_mc_hist, self.error_band, self.data_graph] + signal_hists,
        )

    def plot_roi_markers(self):
        if not self.mc_ec or not self.config.show_roi or self.data_scan_result is None:
            return

        # Get RoI egdes
        roi_lower_edge = self.data_scan_result.roi_lower_edge
        roi_upper_edge = (
            self.data_scan_result.roi_lower_edge + self.data_scan_result.roi_width
        )

        self.roi_lines = []
        if self.opt_ratio:
            pads = [self.pad1a, self.pad1b]
        else:
            pads = [self.canvas]
        for loc in (roi_lower_edge, roi_upper_edge):
            lines, _ = ecroot.create_line_markers(self, loc)
            if len(lines) != len(pads):
                raise ValueError("Not the same number of lines and pads")
            for line, pad in zip(lines, pads):
                pad.cd()
                pad.Update()
                pad.Modified()
                line.SetNDC(False)
                line.SetLineStyle(2)
                line.SetLineColor(633)
                line.SetLineWidth(3)
                if line.GetX1() < self.xup:
                    line.Draw()
            self.roi_lines.extend(lines)
            # TODO: test if this works with a ratio plot
            # No it did not! But now it does.

    @classmethod
    def add_commandline_options(cls, parser):
        super(ECDistributionPlot, cls).add_commandline_options(parser)
        cls.add_distribution_cli_option(parser)
        cls.add_label_cli_options(parser)
        cls.add_legend_cli_options(parser)
        group = parser.add_argument_group(title="ECDistributionPlot plotting options")

        group.add_argument(
            "--data-err-alpha",
            help="Quantile for confidence interval used to evaluate errors on data.",
            default=(1.0 - 0.6827),
            type=float,
        )

        group.add_argument(
            "--ratio", help="Include ratio plot.", default=True, type=bool
        )
        group.add_argument(
            "--ratio-rebin",
            help="Rebin ratio plot until at least one MC event is expected.",
            default=True,
            type=bool,
        )
        group.add_argument(
            "--ratio-scale",
            help="Scale to use for ratio plot: 0 - linear, 1 - log.",
            default=0,
            type=int,
        )

        group.add_argument(
            "--plot-error-ratio",
            help="Show error in ratio: 'band' - band for mc uncert and data points with data uncert, 'total' - datapoints with total uncert.",
            default="band",
            choices=("band", "total"),
            type=str,
        )
        group.add_argument(
            "--plot-error", help="Show systematic error bands.", default=True, type=bool
        )
