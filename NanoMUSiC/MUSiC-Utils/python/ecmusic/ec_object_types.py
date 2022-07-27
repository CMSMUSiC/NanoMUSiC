from ectools.type_definitions  import ECObjectType
from ectools.misc import *

class ECObjectTypeMixin(object):
    """Mixin class which contains all object definitions for MUSiC (Ele, Muon...).

    This class defines the available object types and related meta data to represent
    them in various formats. In addition this modules defines object types which
    are required for a class to be considered or rejected.

    """
    @classmethod
    def required_object_types(cls):
        """ Return a list of names for ec object types which are required
            for a class to be considered.

             Returns:
                A list of ECObject names
        """
        return ["EleEB", "EleEE","Muon", "GammaEB", "Ele", "Tau"]

    @classmethod
    def rejected_object_types(cls):
        """ Return a list of names for ec object types which not allowed
            to be included in a class to be considered.

            Returns:
                A list of ECObject names
        """
        return ["GammaEE"]

    @classmethod
    def object_types(cls):
        '''Return a list of physics objects used to define a event class with
           formating innformation.

           Returns:
               A list of ecroot.ECObjectType instances
        '''
        pobs = []
        pobs.append(ECObjectType("Ele", "ele",
                                 latex_tag=r'e',
                                 root_tag=r'e',
                                 mpl_tag=r'e',
                                 html_tag='e',
                                 ))
        #pobs.append(ECObjectType("EleEB", "eleeb",
        #                         latex_tag=r'e',
        #                         root_tag=r'e',
        #                         mpl_tag=r'e',
        #                         html_tag='e',
        #                         ))
        #pobs.append(ECObjectType("EleEE", "eleee",
        #                         latex_tag=r'e_{e}',
        #                         root_tag=r'e_{e}',
        #                         mpl_tag=r'e_{e}',
        #                         html_tag='e_{e}',
        #                         ))
        pobs.append(ECObjectType("Muon", "muon",
                                 latex_tag=r'\mu',
                                 root_tag=r'#mu',
                                 mpl_tag=r'\mu',
                                 html_tag=r'&mu;',
                                 ))
        pobs.append(ECObjectType("Tau", "tau",
                                 latex_tag=r'$\tau$',
                                 root_tag=r'#tau',
                                 mpl_tag=r'\tau',
                                 html_tag=r'&tau;',
                                 ))
        #pobs.append(ECObjectType("Gamma", "gamma",
        #                         latex_tag=r'$\gamma$',
        #                         root_tag=r'#gamma',
        #                         mpl_tag=r'\gamma',
        #                         html_tag=r'&gamma;',
        #                         ))
        pobs.append(ECObjectType("GammaEB", "gammaeb",
                                 latex_tag=r'\gamma',
                                 root_tag=r'#gamma',
                                 mpl_tag=r'\gamma',
                                 html_tag=r'&gamma',
                                 ))
        pobs.append(ECObjectType("GammaEE", "gammaee",
                                 latex_tag=r'\gamma_{e}',
                                 root_tag=r'#gamma_{e}',
                                 mpl_tag=r'\gamma_{e}',
                                 html_tag=r'&gamma;-e',
                                 ))
        pobs.append(ECObjectType("bJet", "bjet",
                                 latex_tag=r'b',
                                 root_tag=r'b',
                                 mpl_tag=r'b',
                                 html_tag=r'b',
                                 pluralize=False,
                                ))
        pobs.append(ECObjectType("Jet", "jet",
                                 latex_tag=r'jet',
                                 root_tag=r'jet',
                                 mpl_tag=r'jet',
                                 html_tag=r'jet',
                                 pluralize=True,
                                 ))
        pobs.append(ECObjectType("MET", "met",
                                 latex_tag=r'MET',
                                 root_tag=r'MET',
                                 mpl_tag=r'MET',
                                 html_tag=r'MET',
                                 countable=False,
                                 ))
        return pobs
