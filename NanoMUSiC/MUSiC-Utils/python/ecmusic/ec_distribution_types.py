from ectools.type_definitions  import ECDistributionType

class ECDistributionTypeMixin(object):
    """Mixin class which contains all distribution type definitions for MUSiC (SumPt, InvMass, MET).

    This class defines the available event class distributions and related meta data to represent
    them in various formats.

    """

    @classmethod
    def distribution_types(cls):
        '''Return a list of distribution with formating innformation and required_object_types infos.

            Returns:
                A list of ecroot.ECDistributionType instances
        '''
        dists = []
        dists.append(ECDistributionType("SumPt", "SumPt",
                                        latex_tag="$\Sigma \mathrm{p_{T}}$",
                                        root_tag="#Sigma p_{T}"))
        dists.append(ECDistributionType("InvMass", "InvMass",
                                        latex_tag="$\mathrm{M_{inv}}$",
                                        root_tag="M_{inv}"))
        dists.append(ECDistributionType("MET", "MET",
                                        required_object_types=["MET"],
                                        latex_tag="MET",
                                        root_tag="MET"))

        return dists
