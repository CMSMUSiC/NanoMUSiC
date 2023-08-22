from typing import Union
from collections import defaultdict

import ROOT


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
        self.histo_ptr: ROOT.TH1F = root_file.Get(
            f"[{self.class_name}]_[{self.process_group}]_[{self.xs_order}]_[{self.sample}]_[{self.year}]_[{self.shift}]_[{self.histo_name}]"
        )

    @staticmethod
    def parse_histo_name(histo_name: str):
        parsed_histogram = histo_name.split("]_[")
        parsed_histogram[0] = parsed_histogram[0][1:]
        parsed_histogram[-1] = parsed_histogram[0][:-1]
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
    def __init__(
        self,
        name: str,
        counts: list[Histogram],
        invariant_mass: list[Histogram],
        sum_pt: list[Histogram],
        met: Union[list[Histogram], None] = None,
    ):
        self.name = name

        self.counts = counts
        self.invariant_mass = invariant_mass
        self.sum_pt = sum_pt

        self.met = None
        if "MET" in self.name:
            self.met = met
            if self.met == None:
                raise Exception(
                    f"ERROR: Could not create EventClass ({self.name}). Expected MET histograms were not provided."
                )

    def __str__(self):
        n_data = len(list(filter(lambda h: (h.process_group == "Data"), self.counts)))
        n_mc = len(self.counts) - n_data
        return f"Class Name: {self.name}\nData histograms: {n_data}\nMC histograms: {n_mc}\n"


class EventClassCollection:
    def __init__(self, source_files):
        self.classes: dict[EventClass] = {}

        h_counts_per_class = defaultdict(list)
        h_invariant_mass_per_class = defaultdict(list)
        h_sum_pt_per_class = defaultdict(list)
        h_met_per_class = defaultdict(list)
        for f in source_files:
            root_file = ROOT.TFile.Open(f)
            for key in root_file.GetListOfKeys():
                if key.GetName().startswith(f"[EC_"):
                    if "h_counts" in key.GetName():
                        histo = Histogram.from_full_name(key.GetName(), root_file)
                        h_counts_per_class[histo.class_name].append(histo)
                    if "h_invariant_mass" in key.GetName():
                        histo = Histogram.from_full_name(key.GetName(), root_file)
                        h_invariant_mass_per_class[histo.class_name].append(histo)
                    if "h_sum_pt" in key.GetName():
                        histo = Histogram.from_full_name(key.GetName(), root_file)
                        h_sum_pt_per_class[histo.class_name].append(histo)
                    if "h_met" in key.GetName():
                        histo = Histogram.from_full_name(key.GetName(), root_file)
                        h_met_per_class[histo.class_name].append(histo)

        for event_class in h_counts_per_class:
            if (
                event_class in list(h_invariant_mass_per_class.keys())
                and event_class in list(h_sum_pt_per_class.keys())
                # and event_class in list(h_met_per_class.keys())
            ):
                self.classes[event_class] = EventClass(
                    event_class,
                    h_counts_per_class[event_class],
                    h_invariant_mass_per_class[event_class],
                    h_sum_pt_per_class[event_class],
                    h_met_per_class[event_class],
                )

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

    def add(self, event_class: EventClass):
        self.classes[event_class.name] = event_class

    def has(self, event_class_name):
        return event_class_name in self.classes.keys()

    def get_all_classes_names(self):
        return list(self.classes.keys())


if __name__ == "__main__":
    import glob

    source_files = list(
        filter(
            lambda f: ("cutflow" not in f),
            glob.glob(
                "/.automount/home/home__home1/institut_3a/silva/projects/music/NanoMUSiC/build/classification_outputs/*/*/*.root"
            ),
        )
    )

    test_event_classes = EventClassCollection(source_files)

    for ec in test_event_classes:
        print(ec)

    print(test_event_classes["EC_2Muon+X"])
