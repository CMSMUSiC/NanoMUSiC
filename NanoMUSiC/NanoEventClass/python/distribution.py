from event_class import EventClass, Histogram


class ProcessGroup:
    def __init__(self, histograms: list[Histogram], process_group_name: str):
        self.name = process_group_name
        self.Nominal = Histogram.merge_histograms(
            filter(lambda h: h.syst == "Nominal", histograms)
        )
        self.PU_Up = Histogram.merge_histograms(
            filter(lambda h: h.shift == "PU_Up", histograms)
        )
        self.PU_Down = Histogram.merge_histograms(
            filter(lambda h: h.shift == "PU_Down", histograms)
        )
        self.Fakes_Up = Histogram.merge_histograms(
            filter(lambda h: h.shift == "Fakes_Up", histograms)
        )
        self.Fakes_Down = Histogram.merge_histograms(
            filter(lambda h: h.shift == "Fakes_Down", histograms)
        )
        self.PDF_As_Up = Histogram.merge_histograms(
            filter(lambda h: h.shift == "PDF_As_Up", histograms)
        )
        self.PDF_As_Down = Histogram.merge_histograms(
            filter(lambda h: h.shift == "PDF_As_Down", histograms)
        )
        self.ScaleFactor_Up = Histogram.merge_histograms(
            filter(lambda h: h.shift == "ScaleFactor_Up", histograms)
        )
        self.ScaleFactor_Down = Histogram.merge_histograms(
            filter(lambda h: h.shift == "ScaleFactor_Down", histograms)
        )
        self.PreFiring_Up = Histogram.merge_histograms(
            filter(lambda h: h.shift == "PreFiring_Up", histograms)
        )
        self.PreFiring_Down = Histogram.merge_histograms(
            filter(lambda h: h.shift == "PreFiring_Down", histograms)
        )
        self.ElectronResolution_Up = Histogram.merge_histograms(
            filter(lambda h: h.shift == "ElectronResolution_Up", histograms)
        )
        self.ElectronResolution_Down = Histogram.merge_histograms(
            filter(lambda h: h.shift == "ElectronResolution_Down", histograms)
        )
        self.ElectronScale_Up = Histogram.merge_histograms(
            filter(lambda h: h.shift == "ElectronScale_Up", histograms)
        )
        self.ElectronScale_Down = Histogram.merge_histograms(
            filter(lambda h: h.shift == "ElectronScale_Down", histograms)
        )
        self.PhotonResolution_Up = Histogram.merge_histograms(
            filter(lambda h: h.shift == "PhotonResolution_Up", histograms)
        )
        self.PhotonResolution_Down = Histogram.merge_histograms(
            filter(lambda h: h.shift == "PhotonResolution_Down", histograms)
        )
        self.PhotonScale_Up = Histogram.merge_histograms(
            filter(lambda h: h.shift == "PhotonScale_Up", histograms)
        )
        self.PhotonScale_Down = Histogram.merge_histograms(
            filter(lambda h: h.shift == "PhotonScale_Down", histograms)
        )
        self.JetResolution_Up = Histogram.merge_histograms(
            filter(lambda h: h.shift == "JetResolution_Up", histograms)
        )
        self.JetResolution_Down = Histogram.merge_histograms(
            filter(lambda h: h.shift == "JetResolution_Down", histograms)
        )
        self.JetScale_Up = Histogram.merge_histograms(
            filter(lambda h: h.shift == "JetScale_Up", histograms)
        )
        self.JetScale_Down = Histogram.merge_histograms(
            filter(lambda h: h.shift == "JetScale_Down", histograms)
        )
        self.UnclusteredEnergy_Up = Histogram.merge_histograms(
            filter(lambda h: h.shift == "UnclusteredEnergy_Up", histograms)
        )
        self.UnclusteredEnergy_Down = Histogram.merge_histograms(
            filter(lambda h: h.shift == "UnclusteredEnergy_Down", histograms)
        )
        self.TauEnergy_Up = Histogram.merge_histograms(
            filter(lambda h: h.shift == "TauEnergy_Up", histograms)
        )
        self.TauEnergy_Down = Histogram.merge_histograms(
            filter(lambda h: h.shift == "TauEnergy_Down", histograms)
        )


class Distribution:
    def __init__(self, event_class: EventClass, distribution_name: str):
        self.event_class = event_class
        self.distribution_name = distribution_name
        self.event_class_name = self.event_class.name


def main():
    print("Hello, distribution.")


if __name__ == "__main__":
    main()
