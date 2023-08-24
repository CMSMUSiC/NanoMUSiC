import os
import ROOT as ro
import numpy as np

from cmsstyle import exponent_notation, convert_tlatex_to_tbox
from .collector_base import ECCollectorConsumerPlot
from ectools.register import ecroot

from ecresults.util import ClassCountMixin


class ClassYieldsPlot(ECCollectorConsumerPlot, ClassCountMixin):
    """A Plot to visualize the number of classes dependent on their total class yield"""

    def __init__(
        self,
        ec_records,
        class_type,
        distribution=None,
        config=None,
        object_group_tag=None,
        min_yield=None,
    ):

        # call parent constructor
        super(ClassYieldsPlot, self).__init__(
            ec_records,
            config=config,
            object_group_tag=object_group_tag,
            distribution=distribution,
            class_type=class_type,
        )
        self.distribution = distribution
        # check if we show p values and therefore need to differentiate between skipped scans
        if (
            os.path.exists(self.config.scan_db_path)
            and self.config.scan_id
            and (self.config.show_p_value or self.config.sort_option == "p-value")
        ):
            self.ec_records = self.ec_records.filter_skipped(
                invert=self.config.show_skipped_classes
            )

        # check if we use a requirement for the minimal p-value
        if self.config.min_p_value:
            self.ec_records = self.ec_records.filter_gt(
                "p-value", self.config.min_p_value, allow_equal=True
            )
        # order records by yield
        self.ec_records_by_mc_yield = list(
            reversed(self.ec_records.sort_records("integral"))
        )
        self.ec_records_by_data_yield = list(
            reversed(self.ec_records.sort_records("data_integral"))
        )

        self.min_yield = min_yield
        self._legend = None
        self._yield_mc_hist = None
        self._yield_data_hist = None

    def set_cache_dirty(self):
        """Reset all cached properties of this class"""
        super(ClassYieldsPlot, self).set_cache_dirty()
        self._yield_mc_hist = None
        self._yield_data_hist = None
        self._legend = None

    @property
    def legend(self):
        """Property to create a cached root TLegend object"""
        if self._legend is None:
            self._legend = ro.TLegend(0.36, 0.65, 0.93, 0.91)
            self._legend.UseCurrentStyle()
            self._legend.SetNColumns(2)
            self._legend.SetFillStyle(0)
        return self._legend

    @property
    def max_yield(self):
        """Property to get the largest yield from either data or mc"""
        return max(
            self.ec_records_by_mc_yield[0]["integral"],
            self.ec_records_by_data_yield[0]["data_integral"],
        )

    @property
    def bin_edges(self):
        """Property for plot bin edges.

        The x-axis spans several orders of magnitude and the bin edges are
        created to provide 10 bins per magnitude.
        """
        bin_edges = np.array([], dtype="float64")
        by_yield = self.ec_records.sort_records("integral")
        bin_edges = np.linspace(
            min([r["integral"] for r in self.ec_records]), 1, 20, endpoint=False
        )
        for i in range(1, int(np.log10(by_yield[0]["integral"])) + 1):
            bin_edges = np.concatenate(
                [bin_edges, np.linspace(10 ** (i - 1), 10**i, 9, endpoint=False)]
            )
        return bin_edges

    @property
    def yield_mc_hist(self):
        """Property for cached TH1F with nclass vs. class yield for MC"""
        if self._yield_mc_hist is None:

            self._yield_mc_hist = ro.TH1F(
                "yield_mc",
                "yield_mc;N_{Events} > x;number of classes",
                len(self.bin_edges) - 1,
                self.bin_edges,
            )
            self._yield_mc_hist.UseCurrentStyle()
            self._yield_mc_hist.SetLineColor(2)
            self._yield_mc_hist.SetLineWidth(3)
            for ibin in range(1, self._yield_mc_hist.GetNbinsX() + 1):
                for i, record in enumerate(self.ec_records_by_mc_yield):
                    if (
                        self._yield_mc_hist.GetBinLowEdge(ibin)
                        <= self.ec_records_by_mc_yield[i]["integral"]
                    ):
                        self._yield_mc_hist.SetBinContent(
                            ibin, len(self.ec_records_by_mc_yield) - i
                        )
                        found = True
                        break
        return self._yield_mc_hist

    @property
    def yield_data_hist(self):
        """Property for cached TH1F with nclass vs. class yield for Data"""
        if self._yield_data_hist is None:

            self._yield_data_hist = ro.TH1F(
                "yield_data",
                "yield_data;N_{Events} > x;number of classes",
                len(self.bin_edges) - 1,
                self.bin_edges,
            )
            self._yield_data_hist.UseCurrentStyle()
            self._yield_data_hist.SetLineColor(ro.TColor.GetColor("#0c3468"))
            self._yield_data_hist.SetLineWidth(3)

            for ibin in range(1, self._yield_data_hist.GetNbinsX() + 1):
                for i, record in enumerate(self.ec_records_by_data_yield):
                    if (
                        self._yield_data_hist.GetBinLowEdge(ibin)
                        <= self.ec_records_by_data_yield[i]["data_integral"]
                        and self.ec_records_by_data_yield[i]["data_integral"] > 0
                    ):
                        self._yield_data_hist.SetBinContent(
                            ibin, len(self.ec_records_by_data_yield) - i
                        )
                        break
        return self._yield_data_hist

    @property
    def canvas(self):
        """Property for cached TH1F with nclass vs. class yield for Data"""
        if self._canvas is None:
            self._canvas = ro.TCanvas(self.plot_name)
            self._canvas.UseCurrentStyle()
            self._canvas.SetLogy(0)
            self._canvas.SetLogx(1)
        return self._canvas

    def plot(self):
        """Plot all requested objects to the canvas"""
        self.canvas.cd()
        self.yield_mc_hist.Draw("hist")
        self.yield_data_hist.Draw("hist same")
        self.legend.AddEntry(
            self.yield_mc_hist, "{} MC classes".format(self.class_type), "L"
        )
        self.legend.AddEntry(
            self.yield_data_hist, "{} Data classes".format(self.class_type), "L"
        )
        self.legend.Draw()

        self._header_obs = ecroot.create_tex_header(self.canvas, header_outside=True)
        for obs in self._header_obs:
            obs.Draw()

    def draw_min_yield_line(self):
        if self.min_yield is None:
            return
        lines, _ = ecroot.create_line_markers(self, self.min_yield)
        for line in lines:
            self.canvas.cd()
            self.canvas.Update()
            self.canvas.Modified()
            line.SetNDC(False)
            line.SetLineStyle(2)
            line.SetLineColor(633)
            line.SetLineWidth(3)
            line.Draw()

    @property
    def plot_name(self):
        """Property to call get_class_name with settings from the current plot instance"""
        return self.get_plot_name(
            self.config, self.distribution, class_type=self.class_type
        )

    @classmethod
    def get_plot_name(
        cls, config, distribution, class_type="class_type", object_group_tag=None
    ):
        """Construct the output name for this plot

        Args:
            config: A conf argparse nanmespace produced with the plots CLI options
            distribution: An ECDistributionType name
            class_type: An ECClassType name
            object_group_tag: An ECObjectGroup tag
        """
        kwargs = {"class_type": class_type, "suffix": cls.get_suffix(distribution)}
        if object_group_tag:
            return "Class_Yields_{class_type}_{object_group_tag}{suffix}".format(
                object_group_tag=object_group_tag, **kwargs
            )
        return "Class_Yields_{class_type}{suffix}".format(**kwargs)

    @classmethod
    def get_subfolder(cls, config, object_group_tag=None, **kwargs):
        """Return the subfolder where the current plot should be saved"""
        if object_group_tag:
            return "ClassYields/{}".format(object_group_tag)
        return "ClassYields"
