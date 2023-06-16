#!/usr/bin/env python
# Copyright [2015] <III Phys. Inst A, RWTH Aachen University>

# standard library imports
import sys
import os
import logging
import yaml

# setup logging
log = logging.getLogger(__name__)

# 3a imports
import aachen3adb

## Class managing and writing a progress visualisation in the console
#
# Different visualisation modes are available:
# - percentage
# - absolute
# - bar
class Progress():
    ## The constructor
    #
    # @param self Object pointer
    # @param current Current value of the progress
    # @param maximum Maximum value of the progress
    # @param text Text to write in the same line - default is none
    # @param mode Visualisation mode - Options: percentage, absolute, bar
    def __init__(self, current=0.0, maximum=1.0, text='', mode='percentage+absolute'):
        self.fill_current = current
        self.fill_maximum = maximum
        self.text = text
        self.mode = mode.split('+')
        self.draw()


    ## Advance the progress to the provided value
    #
    # @param self Object pointer
    # @param value Value to which to progress
    def advance_to(self, value):
        self.fill_current = float(value)
        self.draw()


    ## Advance the progress by the provided value
    #
    # @param self Object pointer
    # @param value Value by which to progress
    def advance_by(self, value):
        self.fill_current += float(value)
        self.draw()

    ## Write the percentage to the standard out
    #
    # @param self Object pointer
    def draw_percentage(self):
        if self.fill_current != self.fill_maximum:
            percentage = self.fill_current / self.fill_maximum * 100.0
        else:
            percentage = 100.0
        sys.stdout.write(' {0:3.0f} %'.format(percentage))


    ## Write the absolute to the standard out
    #
    # @param self Object pointer
    def draw_absolute(self):
        # this only makes sense for integer values
        sys.stdout.write(' ({0:g}/{1:g})'.format(self.fill_current, self.fill_maximum))


    ## Draw a progress bar in the standard out
    #
    # @param self Object pointer
    def draw_bar(self):
        sys.stdout.write(' [')
        # determine fill percentage
        if self.fill_current < self.fill_maximum:
            percentage = self.fill_current / self.fill_maximum
        else:
            percentage = 1.0

        # draw symbols
        filled = int(percentage * 50)
        empty = 50 - filled
        for i in range(0, filled):
            sys.stdout.write('-')
        for i in range(0, empty):
            sys.stdout.write(' ')
        sys.stdout.write(']')

    ## Determine which type of visualisation to draw
    #
    # Aside from determining the visualisation mode and calling its respective
    # method, the draw() method will also return the cursor the start of the
    # line.
    #
    # @param self Object pointer
    def draw(self):
        # return to beginning of line
        sys.stdout.write('\r')
        sys.stdout.write(self.text)
        for mode in self.mode:
            if mode == 'bar':
                self.draw_bar()
            elif mode == 'percentage':
                self.draw_percentage()
            elif mode == 'absolute':
                self.draw_absolute()
            else:
                raise Exception('Mode \'' + mode + '\' does not exist. Available modes: percentage, absolute, bar')
        sys.stdout.flush()

    ## End the visualisation
    #
    # Ending the visualisation adds a small statement followed by a new line
    # character.
    #
    # @param self Object pointer
    def end(self):
        sys.stdout.write(', done.\n')
        sys.stdout.flush()



## Class containing information regarding analysis samples
#
# Stores skim and sample as well as other minor informations
class AnalysisSample():
    ## The constructor
    #
    # @param self Object pointer
    # @param sampletype Type of the sample: data, mc
    # @param samplegroup Group of the sample specific in the config, e.g. Background
    # @param identifier String or int used to identify sample in the database
    # @param samplearguments Specific command line arguements for this sample
    def __init__(self, dblink, sampletype, samplegroup, identifier, samplearguments):
        # store values
        self.dblink = dblink
        self.identifier = identifier
        self.sampletype = sampletype
        self.samplegroup = samplegroup
        self.samplearguments = samplearguments

        # skim and sample; to be retrieved via self.query_db()
        self.skim = None
        self.sample = None



    ## Class representation as a string
    #
    # @param self Object pointer
    def __str__(self):
        s = ('<AnalysisSample instance>\n'
             'Sample name: ' + self.sample.name + '\n'
             'Custom sample arguments: ' + self.samplearguments + '\n'
             'Number of files: ' + str(len(self.get_files())) + '\n')
        return s


    ## Load sample from database
    #
    # @param filters Preformatted query to use in database query
    def load_db_entry(self, sample_criteria, skim_criteria):
        # Add skim ID to skim criteria
        if isinstance(self.identifier, int):
            skim_criteria['id'] = self.identifier
        elif isinstance(self.identifier, basestring):
            # Add dataset path to skim criteria
            if self.identifier.startswith('/'):
                skim_criteria['datasetpath'] = self.identifier
            # Add sample name to sample criteria
            else:
                sample_criteria['name'] = self.identifier
        else:
            raise ValueError('Identifier {} is neither skim ID, dataset path '
                             'or sample name'.format(self.identifier))
        # Get properly formatted query
        formatted_query = self.dblink.get_criteria_query(
            sample_criteria, skim_criteria,
            True if self.sampletype == 'data' else False
        )
        # Perform query, returning False if there is no result
        result = self.dblink.query_objects(formatted_query, latest=True)
        if not result:
            return False

        # Set sample and skim
        self.sample = result[0]
        self.skim = self.sample.skims[0]
        # Signal success
        return True


    ## Get files to be analyzed for this sample
    #
    # @param self Object pointer
    def get_files(self):
        if hasattr(self.skim, 'files'):
            return [entry.path for entry in self.skim.files]
        else:
            return []


    ## Build a name for the task out of the sample and skim id
    #
    # @param self Object pointer
    def get_taskname(self):
        return self.sample.name + '-skimid' + str(self.skim.id)



## Returns list of AnalysisSamples
#
# @param settings_config Config yaml document with ram settings
# @param sample_config Config yaml document from which to read the samples
# @param testmode If true, only returns one sample per samplegroup
def get_samples(settings_config, sample_config, testmode=False):
    # create one link to the database for all upcoming queries
    dbcon = aachen3adb.ObjectDatabaseConnection()
    # get database query criteria
    skim_criteria, sample_criteria = get_selection_criteria(settings_config)

    # load every sample of each samplegroup
    samplelist = []
    print 'Reading samples from database'
    for samplegroup in sample_config:
        # record progress
        progress = Progress(maximum=len(sample_config[samplegroup]),
                            text=samplegroup + ':',
                            mode='percentage+absolute')

        # determine type of samples and build filter criteria
        sampletype = 'data' if 'data' in samplegroup.lower() else 'mc'
        for sample in sample_config[samplegroup]:
            analysis_sample = AnalysisSample(
                dbcon,  # central link to database
                sampletype,  # data or mc
                samplegroup,  # title of group
                sample,  # identifier
                sample_config[samplegroup][sample]  # sample arguments
            )
            if analysis_sample.load_db_entry(sample_criteria, skim_criteria):
                samplelist.append(analysis_sample)
            else:
                log.error(
                    'No entries for {}: {} are matching the criteria; skipping.'.format(
                        samplegroup, sample
                    )
                )

            # increase progress
            progress.advance_by(1)

            # only add one sample per sample group if in testmode
            if testmode:
                break
        # end progress for this sample group
        progress.end()

    return samplelist


## Load information on settings and samples used from the configuration file
#
# @param configfile The submission configuration file
def load_config(configfile):
    settings = None
    samplecfg = None
    with open(configfile, 'r') as config:
        for document in yaml.load_all(config):
            # first part of the config should be the settings
            if not settings:
                settings = document
            # second part the samples
            elif not samplecfg:
                samplecfg = document
    return settings, samplecfg


## Return skim and sample selection criteria from ram config
#
# @param dblink Link to aix3adb
# @param config A yaml document containing query options for aix3adb
def get_selection_criteria(config):
    # extract selection criteria, if provided
    skim_criteria = config['skim_criteria'] if 'skim_criteria' in config else {}
    sample_criteria = config['sample_criteria'] if 'sample_criteria' in config else {}

    # add defaults for latest unless keys already specified
    default_list = [('deprecated', None)]
    if 'ignore_unfinished' not in config or ('ignore_unfinished' in config and not config['ignore_unfinished']):
        default_list.append(('finished__isnot', None))
    for key, value in default_list:
        if key not in skim_criteria:
            skim_criteria[key] = value

    # sanity checks for skim / sample identifier
    if 'id' in skim_criteria:
        log.error('Skim ID identifies a single skim, remove from skim_criteria.')
        sys.exit(1)
    if 'datasetpath' in skim_criteria:
        log.error('Dataset path identifies a single skim, remove from skim_criteria.')
        sys.exit(1)
    if 'name' in sample_criteria:
        log.error('Sample name identifies a single sample, remove from sample_criteria.')
        sys.exit(1)

    return skim_criteria, sample_criteria


def main():
    print 'Testing progress visualisations'
    # progress test
    print 'Progress bar'
    # empty bar
    preg = Progress(current=0, maximum=70, text='progressblub', mode='bar+percentage+absolute')
    preg.end()

    # filling bar
    prog = Progress(current=0, maximum=70, text='progresstest', mode='bar+percentage+absolute')
    import time
    for i in range(0, 80, 10):
        prog.advance_to(i)
        time.sleep(0.5)
    prog.end()

    return 0


if __name__=='__main__':
    sys.exit(main())
