import ROOT
import os

from .stats import ECStatHelper


class ECIOHelper(ECStatHelper):
    """
         Helper functions to load TEventClass data from root files
    """

    @classmethod
    def read_root_files(cls, conf):
        """ Read in root data and mc files defined in conf namespace

            Args:
                conf: A argparse namespace containting the options defined in add_ec_standard_options

            Returns:
                mc_file, data_file as Root TFile objects
        """
        # Open the ROOT files. Cannot use a contextmanager wrapper here because
        # the file objects are supposed to be None.
        if conf.mc:
            mc_file = cls.read_root_file(conf.mc)
        else:
            mc_file = None

        if conf.data:
            data_file = cls.read_root_file(conf.data)
        else:
            data_file = None

        return mc_file, data_file

    @classmethod
    def read_root_file(cls, path):
        """ Read a single root file from a given path

            Args:
                path: Path to the input root file

            Returns:
                An opened ROOT.TFile object
        """
        if not os.path.isfile(path):
            raise IOError("File not found: '%s'" % path)
        return ROOT.TFile.Open(path)

    @classmethod
    def read_ec_objects(cls, mc_file, data_file, name):
        """" Read EventClass objects from root files for given class name

            Args:
                mc_file: A root TFile object containing TEventClass objects for MC
                data_file: A root TFile object containing TEventClass objects for Data
                name: Name of the EventClass to load from both files

            Returns:
                mc_ec, data_ec with TEventClass objects

        """
        # Obtain the MC TEventClass
        mc_ec = cls.read_ec_object(mc_file, name)
        data_ec = cls.read_ec_object(data_file, name)
        return mc_ec, data_ec

    @classmethod
    def read_ec_object(cls, root_file, name):
        """" Read a EventClass objects from a root file for given class name

            Args:
                root_file: A root TFile object containing TEventClass objects
                name: Name of the EventClass to load from both files

            Returns:
                Return a TEventClass object or None if no event class was found

        """
        if root_file:
            key = root_file.GetKey(name)
            if key:
                return key.ReadObj()
        return None

    @classmethod
    def prepare_class_names(cls,
                            conf,
                            mc_file=None,
                            data_file=None,
                            distribution=None,
                            **kwargs):
        """" Prepare a list of names for event class present in either the mc_file or data_file

            This function is intended to be used to determine the full set of classes to consider from
            both MC and data and apply the filter criteria defined in the ec_conf object.

            Args:
                conf: A argparse namespace containting the options defined in add_ec_standard_options
                mc_file: A root TFile object containing TEventClass objects for MC
                data_file: A root TFile object containing TEventClass objects for Data
                name: Name of the EventClass to load from both files
                **kwargs: Arbitrary keyword arguments passed to filter function_class_names
                          function which might contain additional arguments if the filter_class_names
                          function was overwritten in a child class.

            Returns:
                names_to_keep, names_to veto: Two sets with all found class names divide into chosen and vetoed names.

        """
        names, names_vetoed = set(), set()
        all_kwargs = kwargs
        all_kwargs.update(cls.extra_kwargs(conf))
        for root_file in (mc_file, data_file):
            if not root_file:
                continue
            ec_names = cls.get_class_names(root_file)
            filtered_tmp, vetoed_tmp = cls.filter_class_names(ec_names,
                                                              conf,
                                                              distribution=distribution,
                                                              **all_kwargs)
            names.update(filtered_tmp)
            names_vetoed.update(vetoed_tmp)
        return names, names_vetoed

    @classmethod
    def get_class_names(cls, root_file):
        """ Return a set of all event class names found in a root file """
        ec_names = set()
        first = True
        for key in root_file.GetListOfKeys():
            if key.GetClassName() != 'TEventClass':
                continue
            ec_names.add(key.GetName())
            if first:
                ec_temp = key.ReadObj()
                first = False
                ec_temp.Delete()
        return ec_names
