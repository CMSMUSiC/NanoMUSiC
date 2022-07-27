import json

import luigi

class SystSeedsTarget(luigi.target.Target):

    def __init__(self,
                 path,
                 syst_names,
                 nrounds):
        self.path = path
        self.syst_names = syst_names
        self.nrounds = nrounds

    def exists(self):
        with open(self.path, 'r') as jf:
            seeds = json.load(jf)
        if not self.syst_names:
            raise AttributeError('No syst names defined for SystSeedsTarget')
        # check if all systs are included in dict
        if not set(self.syst_names).issubset(set(seeds.keys)):
            return False
        #check if enough rounds are includes
        if len(seeds[seeds.keys()[0]]) < self.nrounds:
            return False
        return True
