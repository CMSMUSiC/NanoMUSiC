#!/usr/bin/env python
## @package dbutilscms
# This module provides classes to work with various cms databases
#
# This module provides classes to work with various cms databases
# Currently supported databases: McM, DBS

# Standard library imports
import sys
import os
import logging
import subprocess
import json
# Import requests, but disable SSL warnings because McM has not configured SSL
import requests
import requests.exceptions
from requests.packages.urllib3.exceptions import InsecureRequestWarning
requests.packages.urllib3.disable_warnings(InsecureRequestWarning)

log = logging.getLogger( 'dbutilscms' )

class McMUtilities():
    def __init__(self):
        self.mcm_prefix = "https://cms-pdmv.cern.ch/mcm/public/restapi/requests/produces"
        self.mcm_json = None # requested json
        self.gen_json = None # gen-sim json


    def buildURL(self, datasetpath):
        # avoid leading slashes being interpreted as absolute paths
        if datasetpath[0] == "/":
            return os.path.join(self.mcm_prefix, datasetpath[1:])
        else:
            return os.path.join(self.mcm_prefix)


    def readJSON(self, datasetpath):
        # Try to find the dataset in the McM database; Note that McM has poorly
        # configured SSL certificates, which necessitates ignoring them by
        # setting verify to false.
        try:
            response = requests.get(self.buildURL(datasetpath), verify=False)
        except requests.exceptions.RequestException:
            log.error("Failed to retrieve dataset " + datasetpath)
            return {}

        # Return the result of the HTTP request, parsed as JSON dictionary
        return response.json()['results']


    def readURL(self, mcm_dataset):
        # Load the new McM JSON and reset the GEN-SIM JSON
        self.mcm_json = self.readJSON(mcm_dataset)
        self.gen_json = {}
        
        # If there is no McM JSON, we're done here
        if not self.mcm_json:
            return

        #Print By LOR
        print ("mcm_dataset is: %s/n ", mcm_dataset)
    
        # Try to find the corresponding GEN-SIM dataset from dataset chain!!!! LOR COMMENTED IT OUT IN MAY 21
        if 'pdmv_dataset_list' in self.mcm_json['reqmgr_name'][0]['content'].keys():
            for datasetpath in self.mcm_json['reqmgr_name'][0]['content']['pdmv_dataset_list']:
                if datasetpath.endswith('GEN-SIM'):
                    self.gen_json = self.readJSON(datasetpath)
                    break


    # generator level information wrapper
    def getGenInfo(self, key, default=None):
        # Multiple cases in which the default value is being returned
        # If no GEN-SIM sample JSON was loaded
        if not self.gen_json:
            log.error("Could not find GEN-SIM sample,"
                      " returning {}: {}".format(key, default))
            return default
        # If GEN-SIM sample JSON has no generator parameters
        if len(self.gen_json["generator_parameters"]) == 0:
            log.error("No generator_parameters in GEN-SIM sample,"
                      " returning {}: {}".format(key, default))
            return default
        # If the generator_parameters do not contain the desired key
        if not key in self.gen_json["generator_parameters"][-1]:
            log.error("Key not in generator_parameters of GEN-SIM sample,"
                      " returning {}: {}".format(key, default))
            return default

        return self.gen_json["generator_parameters"][-1][key]


    def getCrossSection(self):
        return self.getGenInfo("cross_section", -1.0)


    def getFilterEfficiency(self):
        return self.getGenInfo("filter_efficiency", 1.0)


    # sample level information wrapper
    def getInfo(self, key, default=None):
        # Multiple cases in which the default value is being returned
        # If no sample JSON was loaded
        if not self.mcm_json:
            log.error("Could not find GEN-SIM sample,"
                      " returning {}: {}".format(key, default))
            return default
        # If the generator_parameters do not contain the desired key
        if not key in self.mcm_json:
            log.error("Key not in sample,"
                      " returning {}: {}".format(key, default))
            return default

        return self.mcm_json[key]


    def getEvents(self):
        return self.getInfo("total_events", 0)


    def getGenerators(self):
        return self.getInfo("generators", [""])[0]


    def getEnergy(self):
        return self.getInfo("energy", 13)


    def getCMSSW(self):
        return self.getInfo("cmssw_release", "")


    def getWorkingGroup(self):
        return self.getInfo("pwg", "")


## Barebones exception for the python wrapper around CMS DAS
class DasClientError(Exception):
    ''' Raised for error related to the DAS output '''


## Wrapper around the command line CMS DAS client
#
# This wrapper can be used to retrieve information from the CMS DAS. It assumes
# all output is returned in the JSON format, which is then translated to a
# python dictionary.
class DasClient():
    ## Default constructor
    #
    # @param self: The object pointer
    def __init__(self):
        self.executable = 'dasgoclient'

    ## Call the DAS client in a subprocess with the JSON output flag set
    #
    # @param self: The object pointer
    # @param arguments:
    # @return A dictionary of the parsed JSON return value, if the call was successful
    def call(self, arguments):
        process = subprocess.Popen([self.executable, '-json'] + arguments,
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = process.communicate()
        if process.returncode != 0 or stderr:
            print stdout
            print stderr
            print ' '.join([self.executable, '-json'] + arguments)
            raise DasClientError('{}Returncode: {}'.format(stderr, process.returncode))
        return json.loads(stdout)

    ## Perform a query returning the result or raise an exception for none
    #
    # @param self: The object pointer
    # @param expression: A (valid) query expression to pass to the CMS DAS
    # @return Results of the search as a list, assuming there are results
    def query(self, expression):
        arguments = ['-query', expression]
        results = self.call(arguments)
        if not len(results) > 0:
            raise ValueError('Query expression yielded no results: {}'.format(expression))
        return results

    ## Search for a dataset in a specific instance and return the result
    #
    # @param self: The object pointer
    # @param dataset: String containing the dataset name
    # @param instance: String specifying the instance where the dataset is stored
    def get_data(self, dataset, instance='global', all_status=False):
        query = 'dataset={} instance=prod/{}'.format(dataset, instance)
        if all_status:
            query = 'dataset ' + query + ' status=*'
        return self.query(query)

    ## Get a list of runs for a datasetpath
    #
    # @param self: The object pointer.
    # @param dataset: String containing the dataset name
    # @param instance: String specifying the instance where the dataset is stored
    # @return List containing the initial and final run number
    def get_runrange(self, dataset, instance='global'):
        '''Get run range for a dataset from das'''
        query = 'run dataset={} instance=prod/{}'.format(dataset, instance)
        result_json = self.query(query)
        runs = []
        for res in result_json:
            if not 'run' in res:
                raise KeyError("Problem with das query 'run' not in result")
            for run_res in res['run']:
                err = "Problem with das query 'run_number' not in run dict "
                if not 'run_number' in run_res:
                    raise KeyError(err)
                runs.append(int(run_res['run_number']))
        return sorted(runs)

    ## Get a dict containing most common dataset infos
    #
    # @param self: The object pointer.
    # @param dataset: String containing the dataset name
    # @param instance: String specifying the instance where the dataset is stored
    # @return A dictionary containing the infos: name, nevents, nfiles, nlumis, nblocks, size (byte)
    def get_dataset_summary(self, dataset, instance='global'):
        # Get results as list of dictionaries
        results = self.get_data(dataset, instance)
        # Try to gather all information in one dictionary
        summary = {}
        for result in results:
            summary.update(result['dataset'][0])
        return summary


if __name__ == '__main__':
    # dc = DasClient()
    # import pprint
    # pprint.pprint(dc.get_dataset_summary('/ZToMuMu_NNPDF30_13TeV-powheg_M_800_1400/RunIISummer16MiniAODv2-PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/MINIAODSIM', 'global'))
    # pprint.pprint(dc.query('dataset=/ZToMuMu_NNPDF30_13TeV-powheg_M_800_1400/RunIISummer16MiniAODv2-PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/MINIAODSIM'))
    # print dc.query('asdb')
    sys.exit(0)
