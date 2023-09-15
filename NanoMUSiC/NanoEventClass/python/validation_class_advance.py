from typing import Union
from collections import defaultdict
from tqdm import tqdm
import sys
import copy
from fnmatch import fnmatch
from array import array

import ROOT

ROOT.gROOT.SetBatch(True)


class Histogram:
    def __init__(
        self,
        root_file: ROOT.TFile,
        class_name: str,
        process_group: str,
        xs_order: str,
        sample: str,
        year: str,
        shift: str,
        histo_name: str,
    ):
        self.class_name = class_name
        self.process_group = process_group
        self.xs_order = xs_order
        self.sample = sample
        self.year = year
        self.shift = shift
        self.histo_name = histo_name
        self.full_histo_name = f"[{self.class_name}]_[{self.process_group}]_[{self.xs_order}]_[{self.sample}]_[{self.year}]_[{self.shift}]_[{self.histo_name}]"
        self.histo = root_file.Get(self.full_histo_name)

        self.is_data = process_group == "Data"

    def __str__(self):
        return self.full_histo_name

    def get_bin_content(self, bin):
        return self.histo.GetBinContent(bin)

    def integral(self):
        try:
            return self.histo.Integral()
        except AttributeError:
            print(f"ERROR: Could not get integral of {self}.")
            exit(-1)

    def clone(self):
        try:
            new_histo = copy.deepcopy(self)
            new_histo.histo = self.histo.Clone()
            return new_histo
        except AttributeError:
            print(f"ERROR: Could not clone {self}.")
            exit(-1)

    def reset(self):
        try:
            self.histo.Reset()
        except AttributeError:
            print(f"ERROR: Could not reset {self}.")
            exit(-1)

    def add(self, other):
        try:
            self.histo.Add(other.histo)
        except:
            print(f"ERROR: Could not add {self} to {other}.")
            exit(-1)

    def __add__(self, other):
        self.add(other)
        return self

    def scale(self, c):
        try:
            self.histo.Scale(c)
        except:
            print(f"ERROR: Could not scale {self}.")
            exit(-1)

    def __mul__(self, c):
        self.scale(c)
        return self

    def scale_for_plot(self, c=10.0):
        try:
            return self.histo.Scale(c, "width")
        except:
            print(f"ERROR: Could not scale {self} for plotting.")
            exit(-1)

    def get_limits_bins(self):
        min_bin = 0
        max_bin = 0
        for idx in range(1, self.histo.GetNbinsX() + 1, 1):
            if self.histo.GetBinContent(idx) > 0:
                min_bin = idx
                break
        for idx in range(self.histo.GetNbinsX(), 0, -1):
            if self.histo.GetBinContent(idx) > 0:
                max_bin = idx
                break

        assert min_bin <= max_bin

        if min_bin == max_bin:
            max_bin += 1

        return min_bin, max_bin

    def get_limits(self):
        limits = self.get_limits_bins()
        if limits is None:
            return None
        min_bin, max_bin = limits
        return self.histo.GetBinLowEdge(min_bin), self.histo.GetBinLowEdge(max_bin + 1)

    @staticmethod
    def parse_histo_name(histo_name: str):
        parsed_histogram = histo_name.split("]_[")
        parsed_histogram[0] = parsed_histogram[0][1:]
        parsed_histogram[-1] = parsed_histogram[-1][:-1]
        return parsed_histogram

    @staticmethod
    def from_full_name(hist: str, root_file):
        (
            class_name,
            process_group,
            xs_order,
            sample,
            year,
            shift,
            histo_name,
        ) = Histogram.parse_histo_name(hist)

        return Histogram(
            root_file,
            class_name,
            process_group,
            xs_order,
            sample,
            year,
            shift,
            histo_name,
        )


class EventClass:
    def __init__(self, name: str, histos):
        self.name = name

        self.histos = histos
        # print(f"SELF HISTOS: {self.histos['invariant_mass']}")

        self.has_met = False
        if "MET" in self.name:
            self.has_met = True

        self.data_count = 0
        self.mc_count = 0
        # print(type(self.histos))
        # print(type(histos))
        # histo = [key for key in self.histos.keys()]
        # print(histo)
        # print(f"HISTOS: {histos}")
        # print(len(histos[0]))

        for count in histos[name]:
            if count.shift == "Nominal":
                if count.is_data:
                    self.data_count += count.get_bin_content(1)
                else:
                    self.mc_count += count.get_bin_content(1)

        self.is_valid = self.data_count >= 1 and self.mc_count >= 0.1

    def get_data_count(self) -> float:
        return self.data_count

    def get_mc_count(self) -> float:
        return self.mc_count

    def __str__(self):
        n_data = len(
            list(filter(lambda h: (h.process_group == "Data"), self.histos["sum_pt"]))
        )
        n_mc = len(self.histos[0]) - n_data
        return f"Event Class: {self.name}\nData: {self.get_data_count()} events - {n_data} histograms\nMC: {self.get_mc_count()} events - {n_mc} histograms\n"

    def get_data_histogram(self, histo_name) -> Histogram:
        if histo_name not in self.histos:
            print(
                f"ERROR: Could not get Data histogram. Invalid histogram name ({histo_name})."
            )
            sys.exit(-1)

        data_hist = None
        for h in self.histos[histo_name]:
            # print(f"h in loop:{h}")
            if h.is_data:
                if data_hist is None:
                    data_hist = h.clone()
                else:
                    data_hist.add(h)

        return data_hist

    def get_data_th1(self, histo_name, scale=False) -> ROOT.TH1F:
        h = self.get_data_histogram(histo_name)
        if scale:
            h.scale_for_plot()

        return h

    def get_y_low(self, histo_name) -> float:
        if histo_name not in self.histos:
            print(
                f"ERROR: Could not get Data histogram. Invalid histogram name ({histo_name})."
            )
            sys.exit(-1)

        total_mc_hist = None
        for h in self.histos[histo_name]:
            if total_mc_hist is None:
                total_mc_hist = h.clone()
            else:
                total_mc_hist.add(h)

        lowest_bin_count = sys.float_info.max
        for ibin in range(1, total_mc_hist.histo.GetNbinsX() + 1):
            if (
                total_mc_hist.histo.GetBinContent(ibin) < lowest_bin_count
                and total_mc_hist.histo.GetBinContent(ibin) > 0
            ):
                lowest_bin_count = total_mc_hist.histo.GetBinContent(ibin)

        return lowest_bin_count

    def get_mc_histograms_per_process_group(self, histo_name) -> Histogram:
        if histo_name not in self.histos:
            print(
                f"ERROR: Could not get Data histogram. Invalid histogram name ({histo_name})."
            )
            sys.exit(-1)

        mc_hists = {}
        for h in self.histos[histo_name]:
            if not h.is_data:
                if h.process_group not in mc_hists:
                    mc_hists[h.process_group] = h.clone()
                else:
                    mc_hists[h.process_group].add(h)

        return mc_hists

    def get_ratio_histogram(self, histo_name) -> Histogram:
        if histo_name not in self.histos:
            print(
                f"ERROR: Could not get Data histogram. Invalid histogram name ({histo_name})."
            )
            sys.exit(-1)

        data_hist = None
        total_mc_hist = None
        for h in self.histos[histo_name]:
            if h.is_data:
                if data_hist is None:
                    data_hist = h.clone()
                else:
                    data_hist.add(h)
            else:
                if total_mc_hist is None:
                    total_mc_hist = h.clone()
                else:
                    total_mc_hist.add(h)

        xlow, xup = total_mc_hist.get_limits()
        assert xlow <= xup

        data_hist = data_hist.histo
        total_mc_hist = total_mc_hist.histo

        bin_borders = array("d")
        bin_borders.append(total_mc_hist.GetXaxis().GetBinLowEdge(1))
        local_bin_sum = 0.0
        first_bin = True
        for ibin in range(1, total_mc_hist.GetNbinsX() + 1):
            # Begin one bin at lower part of plotting range
            if total_mc_hist.GetXaxis().GetBinUpEdge(ibin) > xlow and first_bin:
                if xlow > 0:
                    bin_borders.append(total_mc_hist.GetXaxis().GetBinLowEdge(ibin))
                first_bin = False

            local_bin_sum += total_mc_hist.GetBinContent(ibin)
            if local_bin_sum > 1.0 and data_hist.GetBinContent(ibin) > 0:
                bin_borders.append(total_mc_hist.GetXaxis().GetBinUpEdge(ibin))
                append_last_bin = False
                local_bin_sum = 0.0
            else:
                append_last_bin = True

            # cutoff rebinning at upper edge of plotting range
            if total_mc_hist.GetXaxis().GetBinUpEdge(ibin) == xup:
                if not append_last_bin:
                    bin_borders[-1] = total_mc_hist.GetXaxis().GetBinUpEdge(ibin)
                else:
                    bin_borders.append(total_mc_hist.GetXaxis().GetBinUpEdge(ibin))
                break

        return data_hist.Rebin(
            len(bin_borders) - 1, "data", bin_borders
        ), total_mc_hist.Rebin(len(bin_borders) - 1, "mc", bin_borders)


class EventClassCollection:
    def __init__(self, source_files, event_class_pattern, histograms_to_plot, hist_to_validate):
        self.classes: dict[EventClass] = {}
        self.root_files = []
        histograms_per_class = {key: defaultdict(list) for key in event_class_pattern}
        print(f"Hist to validate:{hist_to_validate}")
        print(f"Histograms per class: {histograms_per_class}")
        for f in tqdm(source_files):
            root_file = ROOT.TFile.Open(f)
            self.root_files.append(root_file)
            count_of_histos = 0
            for key in root_file.GetListOfKeys():
                class_name = Histogram.parse_histo_name(key.GetName())[0]
                for p in event_class_pattern:
                    if fnmatch(class_name, p):
                        for histogram in histograms_to_plot:
                            if histogram in key.GetName():
                                count_of_histos += 1
                                histo = Histogram.from_full_name(
                                    key.GetName(), root_file
                                )
                                if histogram in hist_to_validate[class_name]:
                                    if root_file.Get(key.GetName()).ClassName() != "TH2F":
                                        histograms_per_class[histo.class_name][
                                            histogram
                                        ].append(histo)

            if count_of_histos == 0:
                print(
                    f"ERROR: Could not build Event Class collection. Histograms missing found for {f} ."
                )
                sys.exit(-1)

        for event_class in event_class_pattern:
            hist_in_class = {}
            hist_in_class = histograms_per_class[event_class]
            ec = EventClass(
                event_class,
                hist_in_class,
            )
            self.classes[event_class] = ec

    def __iter__(self):
        self.list_event_classes = list(self.classes.keys())
        self.index = 0
        return self

    def __next__(self):
        if self.index < len(self.list_event_classes):
            this_ec = self.classes[self.list_event_classes[self.index]]
            self.index += 1
            return this_ec
        else:
            raise StopIteration

    def __getitem__(self, event_class_name):
        return self.classes[event_class_name]

    def __len__(self):
        return len(list(self.classes.keys()))

    def remove(self, event_class_name):
        self.classes.pop(event_class_name)

    def add(self, event_class: EventClass):
        self.classes[event_class.name] = event_class

    def has(self, event_class_name):
        return event_class_name in self.classes.keys()

    def is_in(self, event_class_name):
        return self.has(event_class_name)

    def get_all_classes_names(self):
        return list(self.classes.keys())


if __name__ == "__main__":
    import glob

    source_files = list(
        filter(
            lambda f: ("cutflow" not in f),
            glob.glob(
                "/net/scratch_cms3a/silva/classification_outputs_2023_08_21/*/*/*.root"
            ),
        )
    )

    with EventClassCollection(
        # source_files, ["EC_2Muon", "EC_2Muon+*"]
        source_files,
        "EC_2Muon",
    ) as test_event_classes:
        for ec in test_event_classes:
            print(ec)

        # print(test_event_classes["EC_2Muon+X"])
