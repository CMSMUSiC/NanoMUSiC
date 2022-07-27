#!/usr/bin/python
#
# Copyright [2015] - <Markus Radziej>

## @package dbconfigmanager
# Tool to manage config files of samples stored in a database
#
# This is a tool to manage cross sections, number of events and weights for
# various samples, which are stored in the aix3adb database. Upon creating the
# DBConfigManager object, one sets a working directory in which the xsv123.cfg
# files are stored. To sync a list of samples with the database, one has to pass
# it to update_config function. The tool then takes the provided config file or
# the latest revision it can find in the working directory and compares its
# entries with the ones stored in the database.
#
# Example usage
#
# Create the object and pass the working directory as an argument
# import dbconfigmanager
# cfgmgr = dbconfigmanager.DBConfigManager("../cfg/xs/")
#
# Read a specific config file
# cfgmgr.read_config("../cfg/xs/xsv300.cfg")
#
# Update the loaded or a specific config with a list of samples
# update_succesful = cfgmgr.update_config(["blub"])
# update_succesful = cfgmgr.update_config(["blub"], "../cfg/xs/xsv300.cfg")
#
# Instead of frequently querying the database, one can use auto_update. This
# will only perform a query, if the latest revision is older than the second
# argument (24 hours in this example).
# cfgmgr.auto_update(["blub"], 24)
#
# Retrieving the read python config
# cfgmgr.get_config()


import sys
import os

import logging
import ConfigParser
import datetime

import aix3adb

## Logger object
log = logging.getLogger("dbconfigmanager")

## Database config manager class
#
# The database config manager is a tool to compare sample entries in a database
# with a local config file. It can update the contents of said file and write
# them to a new revision, meaning that no information is lost.
class DBConfigManager:
    ## Initialization method
    #
    # Can be used to set up the working directory by passing it as an argument.
    #
    # @param self The object pointer.
    # @param directory_path The path to the working directory.
    def __init__(self, directory_path="../cfg/xs/"):
        # set config file directory
        ## @var directory
        # Working directory of the config manager
        self.directory = os.path.abspath(directory_path)
        ## @var config
        # Config object that is being worked on after reading
        self.config = ConfigParser.SafeConfigParser()
        ## @var aix3adb
        # Aix3adb database object
        self.aix3adb = aix3adb.aix3adb()

    ## Set the working directory of the DBConfigManager
    #
    # @param self The object pointer.
    # @param directory_path The path to the working directory.
    def set_directory(self, directory_path):
        """Set the working directory of the config manager."""

        self.directory = directory_path

    ## Returns the latest revision of the config files named 'xsv123.cfg'
    #
    # Checks the working directory for the xsv123.cfg file with the highest
    # number and returns that number
    #
    # @return Latest config file revision number
    def latest_revision(self):
        """Returns the latest revision of the config files named 'xsv123.cfg'."""

        # get all _files_ in directory
        files = []
         for (dirpath, dirnames, filenames) in os.walk(self.directory):
            files.extend(filenames)
            break

        # reduce the file names to the numeric portion
        file_numbers = []
        for f in files:
            f = f.rsplit(".")[0]
            file_numbers.append(int(f.replace("xsv", "")))

        # return last element / latest revision
        return sorted(file_numbers)[-1] if file_numbers else None


    ## Returns the file name of the latest revision of config files named 'xsv123.cfg'
    #
    # @param self The object pointer
    # @return File name of the latest revision of config files
    def latest_file(self):
        return "xsv" + str(self.latest_revision()) + ".cfg"


    ## Returns the path to the file corresponding to latest_revision()
    #
    # @param self The object pointer
    # @return Absolute path to latest revision of config file
    def latest_abs_file(self):
        return self.directory + "/" + self.latest_file()


    ## Compares sample values with database entry
    #
    # Returns true if sample values are up to date, else false Returns the
    # absolut path to the latest revision of config files
    #
    # @param self The object pointer
    # @param sample Sample which is being compared
    # @return True if sample is up to date, else False
    def compare_sample(self, sample):
        try:
            skimid = self.config.getint(sample, "id") if self.config.has_option(sample, "id") else None
            db_skim, db_sample = self.aix3adb.getSkimAndSample("mc", skimd=skimid, samplename=sample)
        except aix3adb.Aix3adbException:
            log.error("Sample '" + sample + "' could not be found in the database. It has been removed from the config.")
            # if sample is not in the database and not in the config,
            # the config is considered up to date
            return not self.config.remove_section(sample)

        up_to_date = True
        # update section
        if not self.config.has_section(sample):
            log.info("Created new section for sample '" + sample + "'.")
            up_to_date = False
            self.config.add_section(sample)
            self.config.set(sample, "nevents", "0")
            self.config.set(sample, "weight", "0.0")
            self.config.set(sample, "crosssection", "0.0")


        # update crosssection
        crosssection = self.config.getfloat(sample, "crosssection")
        db_crosssection = float(db_sample.crosssection)
        if not crosssection  == db_crosssection:
            log.info(sample
                     + " [Crosssection] Old " + str(crosssection)
                     + " -- >>NEW<< " + str(db_crosssection))
            up_to_date = False
            self.config.set(sample, "crosssection", str(db_crosssection))

        # update weight
        # CAREFUL: composition of k-factor and filter efficiency
        weight = self.config.getfloat(sample, "weight")
        db_weight = float(db_sample.kfactor) * float(db_sample.filterefficiency)
        if not weight == db_weight:
            log.info(sample
                     + " [Weight] Old " + str(weight)
                     + " -- >>NEW<< " + str(db_weight))
            up_to_date = False
            self.config.set(sample, "weight", str(db_weight))

        # update nevents
        nevents = self.config.getint(sample, "nevents")
        db_nevents = int(db_skim.nevents)
        if not nevents == db_nevents:
            log.info(sample +
                     " [NEvents] Old " + str(nevents)
                     + " -- >>NEW<< " + str(db_nevents))
            up_to_date = False
            self.config.set(sample, "nevents", str(db_nevents))

        return up_to_date

    ## Reads the given config_file into the class variable self.config
    #
    # If no config_file is given, the latest revision of config files is being
    # read.
    #
    # @param self The object pointer
    # @param config_file Path to a config file to be read. Optional.
    def read_config(self, config_file = None):
        # if no config_file is given, try to fill default value
        if config_file is None:
            # replace with latest revision (if there is one)
            rev = self.latest_revision()
            if not rev is None:
                name = "xsv" + str(rev) + ".cfg"
                log.info("Working with latest config file revision '" + name + "'.")
                config_file = os.path.abspath(self.directory + "/" + name)
            else:
                # if no latest revision exists, create a new config
                log.info("No latest revision found, working with new/empty config.")

        # try reading the config file
        if not config_file is None:
            self.config.readfp(open(config_file))


    ## Returns the python config object from the class variable self.config
    #
    # Useful when you want to compare config values in the plotting tool to the
    # ones in the database
    #
    # @param self The object pointer
    # @return Python config object
    def get_config(self):
        return self.config


    ## Updates config_file for a given sample_list
    #
    # Searches for entries from sample_list in self.config and compares with
    # potential database entries. If differences are found, a new config file
    # revision with updated entires is created. Default value for config_file is
    # the latest revision in the working directory. Return value indicates if
    # the update was succesful.
    #
    # @param self The object pointer
    # @param sample_list List of samples to update in the config file
    # @param config_file Config file to update. Default is the latest revision
    # of config files
    # @return True if the update is succesful or unnecessary, else False.
    def update_config(self, sample_list, config_file=None):
        # read config if one is given
        if not config_file is None:
            self.read_config(config_file)

        # loop over sample list and update the values
        log.info("Updating config...")
        up_to_date = True
        for sample in sample_list:
            up_to_date = self.compare_sample(sample) and up_to_date

        # write revision if necessary
        if up_to_date:
            log.info("Config file is up to date. Nothing to be done.")
            return True
        else:
            # set timestamp
            if not self.config.has_section("misc"):
                self.config.add_section("misc")
            self.config.set("misc", "timestamp", datetime.datetime.strftime(datetime.datetime.now(),
                                                                            "%Y-%m-%d %H:%M:%S"))
            # create and write file
            rev = self.latest_revision()
            file_name = "xsv" + str(rev + 1 if not rev is None else 1) + ".cfg"
            with open(self.directory + "/" + file_name, "w") as f:

                self.config.write(f)
            log.info("Succesfully updated config file. Latest revision is now '" + file_name + "'.")
            return True


    ## Update the latest revision of config files if older than age_in_hours
    #
    # Calls update_config(sample_list) if the timestamp is older than
    # age_in_hours.
    #
    # @param self The object pointer
    # @param sample_list List of samples to update in the config file
    # @param age_in_hours Age of the latest revision in hours after which it
    # should be updated
    # @return True if the update is succesful or unnecessary, else False.
    def auto_update(self, sample_list, age_in_hours=24):
        # read the default/latest config
        self.read_config()
        # compare timestamp to current time
        if self.config.has_option("misc", "timestamp"):
            timestamp = datetime.datetime.strptime(self.config.get("misc", "timestamp"),
                                                   "%Y-%m-%d %H:%M:%S")
            delta = datetime.datetime.now() - timestamp
            if delta < datetime.timedelta(hours=age_in_hours):
                log.info("Latest revision is newer than " + str(age_in_hours) + "h. Skipping update.")
                return True

        return self.update_config(sample_list)

## main() method for testing purposes
def main():
    logging.basicConfig(level=logging.DEBUG)
    log.info("This is meant to be used as a library.")


if __name__ == "__main__":
    sys.exit(main())
