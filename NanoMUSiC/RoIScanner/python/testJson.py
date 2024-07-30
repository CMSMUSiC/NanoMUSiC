import json
import random


def makeMCBins():
    MCbinList = []
    for ibin in range(100):
        mcbindict = {}
        # Add number of events and bin borders
        mcbindict.update({"Nevents": random.uniform(1, 220)})
        mcbindict.update({"lowerEdge": ibin * 10})
        mcbindict.update({"width": 10})
        mcbindict.update({"MCerror": random.uniform(1, 20)})
        uncertDict = {}
        # Add uncertainties
        for systName in ["test1_up", "test1_down", "test2_up", "test2_down"]:
            uncertDict.update({systName: random.uniform(1, 220)})
        mcbindict.update({"systematics": uncertDict})
        unweightDict = {}
        for proc in ["proc1", "proc2", "proc3"]:
            unweightDict.update(
                {proc: {"Nevents": mcbindict["Nevents"] / 6, "xs": 2.0}}
            )
        mcbindict.update({"unweightedEvents": unweightDict})

        MCbinList.append(mcbindict)
    return MCbinList


testDict = {"MCBins": makeMCBins()}
# add seedDict
testDict.update({"Seeds": {"test1": 1222, "test2": 87978675}})
with open("test.json", "w") as outfile:
    json.dump(testDict, outfile)
