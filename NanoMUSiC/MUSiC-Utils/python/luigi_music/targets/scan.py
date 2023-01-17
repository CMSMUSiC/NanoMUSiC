import logging
import json
import os

import luigi

logger = logging.getLogger('luigi-interface')

class SystSeedsTarget(luigi.target.Target):

    def __init__(self,
                 path,
                 syst_names,
                 nrounds):
        self.path = path
        self.syst_names = syst_names
        self.nrounds = nrounds

    def exists(self):

        if not self.syst_names:
            raise AttributeError('No syst names defined for SystSeedsTarget')
        if not os.path.exists(self.path):
            return False
        with open(self.path, 'r') as jf:
            try:
                seeds = json.load(jf)
            except ValueError as e:
                logger.error("Unable to load " + self.path)
                raise e
        # check if all systs are included in dict
        if not set(self.syst_names).issubset(set(seeds.keys())):
            return False
        #check if enough rounds are includes
        if len(seeds[seeds.keys()[0]]) < self.nrounds:
            return False
        return True

class LocalScanTarget(luigi.LocalTarget):

    @property
    def files(self):
        return [self.path]
