import itertools

from ectools.type_definitions  import ECGroupType
from ectools.misc import *

class ECObjectGroupMixin(object):
    """Mixin class which contains all object group definitions for MUSiC.

    This class defines the available object group types and related meta data to represent
    them in various formats. In addition this module defines filter functions to
    filter for a object type in sets of event class names.

    """

    @classmethod
    def object_groups(cls):
        '''Return a list of object groups with formating information and filter functions.

            Returns:
                A list of ecroot.ECGroupType instances
        '''
        groups = []
        groups.append(ECGroupType("single electron", "1Ele",
                                  filter_func=cls.single_ele_only,
                                  mpl_tag=r'\ 1e',
                                  root_tag=r' 1e',
                                  latex_tag=r'1e',
                                  html_tag=r' 1e',
                                  description="Exactly one electron and an arbitrary number of b-jets and jets",
                                  description_latex="$1e \,+\mathrm{X} $ b-jets or jets"
                                  ))

        groups.append(ECGroupType("double electron", "2Ele",
                                  filter_func=cls.double_ele_only,
                                  mpl_tag=r'\ 2e',
                                  root_tag=r' 2e',
                                  latex_tag=r' 2e',
                                  html_tag=r' 2e',
                                  description="Exactly two electrons and any number of b-jets and jets",
                                  description_latex = "$2e \,+\mathrm{X} $ b-jets or jets"
                                  )),

        groups.append(ECGroupType("single muon", "1Muon",
                                  filter_func=cls.single_muon_only,
                                  latex_tag=r'1$\mu$',
                                  root_tag=r'1#mu',
                                  mpl_tag=r'1\mu',
                                  html_tag=r'1&mu;',
                                  description="Exactly one muon and any number of b-jets and jets",
                                  description_latex="$1\mu \,+\mathrm{X} $ b-jets or jets",
                                  ))

        groups.append(ECGroupType("double muon", "2Muon",
                                  filter_func=cls.double_muon_only,
                                  latex_tag=r'2$\mu$',
                                  root_tag=r'2#mu',
                                  mpl_tag=r'2\mu',
                                  html_tag=r'2&mu;',
                                  description="Exactly two muons and any number of b-jets and jets",
                                  description_latex="$2\mu \, +\mathrm{X} $ b-jets or jets"
                                  )),

        groups.append(ECGroupType("single tau", "1Tau",
                                  filter_func=cls.single_tau_only,
                                  latex_tag=r'1$\tau$',
                                  root_tag=r'1#tau',
                                  mpl_tag=r'1\tau',
                                  html_tag=r'1&tau;',
                                  description="Exactly one tau and any number of b-jets and jets",
                                  description_latex="$1\tau \,+\mathrm{X} $ b-jets or jets",
                                  ))

        groups.append(ECGroupType("double tau", "2Tau",
                                  filter_func=cls.double_tau_only,
                                  latex_tag=r'2$\tau$',
                                  root_tag=r'2#tau',
                                  mpl_tag=r'2\tau',
                                  html_tag=r'2&tau;',
                                  description="Exactly two taus and any number of b-jets and jets",
                                  description_latex="$2\tau \, +\mathrm{X} $ b-jets or jets"
                                  )),

        #~ groups.append(ECGroupType("single gamma", "1GammaEB",
                                  #~ filter_func=cls.single_gamma_only,
                                  #~ latex_tag=r'1$\gamma$',
                                  #~ root_tag=r'1#gamma',
                                  #~ mpl_tag=r'1\gamma',
                                  #~ html_tag=r'1&gamma',
                                  #~ description="Exactly one photon and any number of b-jets and jets",
                                  #~ description_latex="$1\gamma \,+\mathrm{X} $ b-jets or jets"
                                  #~ ))

        groups.append(ECGroupType("double gamma", "2GammaEB",
                                  filter_func=cls.double_gamma_only,
                                  latex_tag=r'2$\gamma$',
                                  root_tag=r'2#gamma',
                                  mpl_tag=r'2\gamma',
                                  html_tag=r'2&gamma',
                                  description="Exactly two photons and any number of b-jets and jets",
                                  description_latex="$2\gamma \,+\mathrm{X} $ b-jets or jets",
                                  ))

        groups.append(ECGroupType("muon + electron", "1Ele_1Muon",
                                  filter_func=cls.ele_muon_only,
                                  latex_tag=r'1e + 1\mu$',
                                  root_tag=r'1e + 1#mu',
                                  mpl_tag=r'1e + 1\mu',
                                  html_tag=r'1e + 1\mu',
                                  description="Exactly one muon and one ele and any number of b-jets and jets",
                                  description_latex="$2\gamma \,+\mathrm{X} $ b-jets or jets",
                                  ))
        groups.append(ECGroupType("electron + tau", "1Ele_1Tau",
                                  filter_func=cls.ele_tau_only,
                                  latex_tag=r'1e + 1$\tau$',
                                  root_tag=r'1e + 1#tau',
                                  mpl_tag=r'1e + 1\tau',
                                  html_tag=r'1e + 1\tau',
                                  description="Exactly one ele and one tau and any number of b-jets and jets",
                                  description_latex="$2\gamma\,+\mathrm{X} $ b-jets or jets",
                                  ))
        groups.append(ECGroupType("muon + tau", "1Muon_1Tau",
                                  filter_func=cls.muon_tau_only,
                                  latex_tag=r'1\tau$ + 1\mu$',
                                  root_tag=r'1#tau + 1#mu',
                                  mpl_tag=r'1\tau + 1\mu',
                                  html_tag=r'1\tau + 1\mu',
                                  description="Exactly one muon and one tau and any number of b-jets and jets",
                                  description_latex="$2\gamma\,+\mathrm{X} $ b-jets or jets",
                                  ))


        groups.append(ECGroupType("triple lepton SF", "triplelepton_sf",
                                  filter_func=cls.triple_lepton_sf_only,
                                  latex_tag=r'triple lepton SF',
                                  root_tag=r'triple lepton SF',
                                  mpl_tag=r'triple lepton SF',
                                  html_tag=r'triple lepton SF',
                                  description="Exactly three electrons or three muons and any number of b-jets and jets",
                                  description_latex="$(3)e/\mu\(SF),+\mathrm{X} $ b-jets or jets"
                                  ))

        groups.append(ECGroupType("triple lepton OF", "triplelepton_of",
                                  filter_func=cls.triple_lepton_of_only,
                                  latex_tag=r'triple lepton OF',
                                  root_tag=r'triple lepton OF',
                                  mpl_tag=r'triple lepton OF',
                                  html_tag=r'triple lepton OF',
                                  description="A total of three leptons with at least one muon and one electron and any number of b-jets and jets",
                                  description_latex="$(3)e/\mu\(OF),+\mathrm{X} $ b-jets or jets"
                                  ))

        groups.append(ECGroupType("multi lepton", "multilepton",
                                  filter_func=cls.multi_lepton_only,
                                  latex_tag=r'multi lepton',
                                  root_tag=r'multi lepton',
                                  mpl_tag=r'multi lepton',
                                  html_tag=r'multi lepton',
                                  description="Four or more leptons in any combination of electrons and muons and any number of b-jets and jets",
                                  description_latex="$(4+)e/\mu\,+\mathrm{X} $ b-jets or jets"
                                  ))

        groups.append(ECGroupType("single muon \& photons", "1Muon_photons",
                                  filter_func=cls.single_muon_photons,
                                  latex_tag=r"$1\mu$ + $\gamma$'s",
                                  root_tag=r"1#mu + #gamma's",
                                  mpl_tag=r"1\mu + \gamma's",
                                  html_tag=r"1&mu + &gamma's",
                                  description="Exactly one muon, at least one photon and any number of b-jets and jets",
                                  description_latex="$1\mu\, (1+)\gamma\,+\mathrm{X} $ b-jets or jets"
                                  ))

        groups.append(ECGroupType("single tau \& photons", "1Tau_photons",
                                  filter_func=cls.single_tau_photons,
                                  latex_tag=r"$1\tau$ + $\gamma$'s",
                                  root_tag=r"1#tau + #gamma's",
                                  mpl_tag=r"1\tau + \gamma's",
                                  html_tag=r"1&tau + &gamma's",
                                  description="Exactly one tau, at least one photon and any number of b-jets and jets",
                                  description_latex="$1\tau\, (1+)\gamma\,+\mathrm{X} $ b-jets or jets"
                                  ))

        groups.append(ECGroupType("single electron \& photons", "1Ele_photons",
                                  filter_func=cls.single_ele_photons,
                                  latex_tag=r"1e + $\gamma$'s",
                                  root_tag=r"1e+ #gamma's",
                                  mpl_tag=r"1e + \gamma's",
                                  html_tag=r"1e + &gamma's",
                                  description="Exactly one electron, at least one photon and any number of b-jets and jets",
                                  description_latex="$1e\, (1+)\gamma\,  +\mathrm{X} $ b-jets or jets"
                                  ))

        groups.append(ECGroupType("double muons \& photons", "2Muon_photons",
                                  filter_func=cls.double_muon_photons,
                                  latex_tag=r"$2\mu$ + $\gamma$'s",
                                  root_tag=r"2#mu + #gamma's",
                                  mpl_tag=r"2\mu + \gamma's",
                                  html_tag=r"2&mu + &gamma's",
                                  description="Exactly two muons, at least one photon and any number of b-jets and jets",
                                  description_latex="$2\mu\, (1+)\gamma\, +\mathrm{X} $ b-jets or jets",
                                  ))

        groups.append(ECGroupType("double taus \& photons", "2Tau_photons",
                                  filter_func=cls.double_tau_photons,
                                  latex_tag=r"$2\tau$ + $\gamma$'s",
                                  root_tag=r"2#tau + #gamma's",
                                  mpl_tag=r"2\tau + \gamma's",
                                  html_tag=r"2&tau + &gamma's",
                                  description="Exactly two taus, at least one photon and any number of b-jets and jets",
                                  description_latex="$2\tat\, (1+)\gamma\, +\mathrm{X} $ b-jets or jets",
                                  ))

        groups.append(ECGroupType("double electron \& photons", "2Ele_photons",
                                  filter_func=cls.double_ele_photons,
                                  latex_tag=r'2e + photons',
                                  root_tag=r'2e + photons',
                                  mpl_tag=r'2e + photons',
                                  html_tag=r'2e + photons',
                                  description="Exactly two electrons, at least one photon and any number of b-jets and jets",
                                  description_latex="$2e\, (1+)\gamma\, +\mathrm{X} $ b-jets or jets",
                                  ))

        groups.append(ECGroupType("ele + muon \& photons", "1Ele_1Muon_photons",
                                  filter_func=cls.ele_muon_photons,
                                  latex_tag=r"1e + 1$\mu$ + $\gamma$'s",
                                  root_tag=r"1e + 1#mu+ #gamma's",
                                  mpl_tag=r"1e + 1\mu + \gamma's",
                                  html_tag=r"1e + 1\mu + \gamma's",
                                  description="Exactly one electron, one muon, any number of photons and any number of b-jets and jets",
                                  description_latex="$1e \, 1\mu\, (1+)\gamma\, +\mathrm{X} $ b-jets or jets"
                                  ))

        groups.append(ECGroupType("ele + tau \& photons", "1Ele_1Tau_photons",
                                  filter_func=cls.ele_tau_photons,
                                  latex_tag=r"1e + 1$\tau$ + $\gamma$'s",
                                  root_tag=r"1e + 1#tau+ #gamma's",
                                  mpl_tag=r"1e + 1\tau + \gamma's",
                                  html_tag=r"1e + 1\tau + \gamma's",
                                  description="Exactly one electron, one tau, any number of photons and any number of b-jets and jets",
                                  description_latex="$1e \, 1\tau\, (1+)\gamma\, +\mathrm{X} $ b-jets or jets"
                                  ))


        groups.append(ECGroupType("muon + tau \& photons", "1Muon_1Tau_photons",
                                  filter_func=cls.muon_tau_photons,
                                  latex_tag=r"1$\mu$ + 1$\tau$ + $\gamma$'s",
                                  root_tag=r"1#mu + 1#tau+ #gamma's",
                                  mpl_tag=r"1\mu + 1\tau + \gamma's",
                                  html_tag=r"1\mu + 1\tau + \gamma's",
                                  description="Exactly one muon, one tau, any number of photons and any number of b-jets and jets",
                                  description_latex="$1\mu \, 1\tau\, (1+)\gamma\, +\mathrm{X} $ b-jets or jets"
                                  ))


        groups.append(ECGroupType("triple lepton \& photons", "triple_photons",
                                  filter_func=cls.triple_lepton_photons,
                                  latex_tag=r"triple lepton + \gamma's",
                                  root_tag=r"triple lepton + #gamma's",
                                  mpl_tag=r"triple lepton + \gamma's",
                                  html_tag=r"triple lepton + \gamma's",
                                  description="Exactly 3 electrons or muons in any combination of electrons and muons, at least one photon and any of b-jets and jets",
                                  description_latex="$(3)e/\mu \,(1+)\gamma\, +\mathrm{X} $ b-jets or jets",
                                  ))

        #~ groups.append(ECGroupType("multi lepton \& photons", "multilepton_photons",
                                  #~ filter_func=cls.multi_lepton_photons,
                                  #~ latex_tag=r"multi lepton + \gamma's",
                                  #~ root_tag=r"multi lepton + #gamma's",
                                  #~ mpl_tag=r"multi lepton + \gamma's",
                                  #~ html_tag=r"multi lepton + \gamma's",
                                  #~ description="At least 4 electrons or muons in any combination, at least one photon and any of b-jets and jets",
                                  #~ description_latex="$(4+)e/\mu \,(1+)\gamma\, +\mathrm{X} $ b-jets or jets",
                                  #~ ))

        groups.append(ECGroupType("single electron + MET", "singleelectron_met",
                                  filter_func=cls.single_ele_met,
                                  latex_tag=r"1e + MET",
                                  root_tag=r"1e + MET",
                                  mpl_tag=r"1e + MET",
                                  html_tag=r"1e + MET",
                                  description="One electron, a significant amount of MET, any number of b-jets and jets",
                                  description_latex="$1\mathrm{e}\, ,>100\GeV \,\met\, +\mathrm{X} $ b-jets or jets",
                                  ))

        groups.append(ECGroupType("single muon + MET", "singlemuon_met",
                                  filter_func=cls.single_muon_met,
                                  latex_tag=r"1\mu + MET",
                                  root_tag=r"1\mu + MET",
                                  mpl_tag=r"1\mu + MET",
                                  html_tag=r"1\mu + MET",
                                  description="One muon, a significant amount of MET, any number of b-jets and jets",
                                  description_latex="$1\mathrm{\mu}\, ,>100\GeV \,\met\, +\mathrm{X} $ b-jets or jets",
                                  ))

        groups.append(ECGroupType("single tau + MET", "singletau_met",
                                  filter_func=cls.single_tau_met,
                                  latex_tag=r"1\tau + MET",
                                  root_tag=r"1\tau + MET",
                                  mpl_tag=r"1\tau + MET",
                                  html_tag=r"1\tau + MET",
                                  description="One tau, a significant amount of MET, any number of b-jets and jets",
                                  description_latex="$1\mathrm{\tau}\, ,>100\GeV \,\met\, +\mathrm{X} $ b-jets or jets",
                                  ))


        groups.append(ECGroupType("double lepton + MET", "doublelepton_met",
                                  filter_func=cls.double_lepton_met,
                                  latex_tag=r"double lepton + MET",
                                  root_tag=r"double lepton + MET",
                                  mpl_tag=r"double lepton + MET",
                                  html_tag=r"double lepton + MET",
                                  description="Two same flavor electrons or muons, a significant amount of MET, and an arbitrary number of b-jets and jets",
                                  description_latex="$2\mathrm{l}\,>100\GeV \,\met\,+ \mathrm{X} $ b-jets or jets",
                                  ))

        groups.append(ECGroupType("triple lepton (SF) + MET", "triplelepton_sf_met",
                                  filter_func=cls.triple_lepton_sf_met,
                                  latex_tag=r"triple lepton SF + MET",
                                  root_tag=r"triple lepton SF + MET",
                                  mpl_tag=r"triple lepton SF + MET",
                                  html_tag=r"triple lepton SF + MET",
                                  description="Exactly three electrons or three muons, a significant amount of MET, and an arbitrary number number of b-jets and jets",
                                  description_latex="$(3)\mathrm{l}(SF)\,>100\GeV \,\met\,+ \mathrm{X} $ b-jets or jets",
                                  ))

        groups.append(ECGroupType("triple lepton (OF)+ MET", "triplelepton_of_met",
                                  filter_func=cls.triple_lepton_of_met,
                                  latex_tag=r"triple lepton OF + MET",
                                  root_tag=r"triple lepton OF + MET",
                                  mpl_tag=r"triple lepton OF + MET",
                                  html_tag=r"triple lepton OF + MET",
                                  description="A total of three leptons with at least one muon and one electron, a significant amount of MET, and an arbitrary number number of b-jets and jets",
                                  description_latex="$(3)\mathrm{l}(SF)\,>100\GeV \,\met\,+ \mathrm{X} $ b-jets or jets",
                                  ))

        groups.append(ECGroupType("multi lepton + MET", "multilepton_met",
                                  filter_func=cls.multi_lepton_met,
                                  latex_tag=r"multi lepton + MET",
                                  root_tag=r"multi lepton + MET",
                                  mpl_tag=r"multi lepton + MET",
                                  html_tag=r"multi lepton + MET",
                                  description="At least three electrons or muons in any combination, a significant amount of MET, and an arbitrary number number of b-jets and jets",
                                  description_latex="$(3+)\mathrm{l}\,>100\GeV \,\met\,+ \mathrm{X} $ b-jets or jets",
                                  ))

        groups.append(ECGroupType("single electron \& photons + MET", "singleelectron_photons_met",
                                  filter_func=cls.single_ele_photons_met,
                                  latex_tag=r"1e + \gamma's + MET",
                                  root_tag=r"1e + #gamma's + MET",
                                  mpl_tag=r"1e + \gamma's + MET",
                                  html_tag=r"1e + \gamma's + MET",
                                  description="Exactly one electron, at least one photon, a significant amount of MET, and an arbitrary number of b-jets and jets",
                                  description_latex="$1\mathrm{e}\, (1+)\gamma\, >100\GeV \met\, +\mathrm{X} $ b-jets or jets",
                                  ))

        groups.append(ECGroupType("single muon \& photons + MET", "singlemuon_photons_met",
                                  filter_func=cls.single_muon_photons_met,
                                  latex_tag=r"1\mu + \gamma's + MET",
                                  root_tag=r"1\mu + #gamma's + MET",
                                  mpl_tag=r"1\mu + \gamma's + MET",
                                  html_tag=r"1\mu + \gamma's + MET",
                                  description="Exactly one muon, at least one photon, a significant amount of MET, and an arbitrary number of b-jets and jets",
                                  description_latex="$1\mathrm{\mu}\, (1+)\gamma\, >100\GeV \met\, +\mathrm{X} $ b-jets or jets",
                                  ))

        groups.append(ECGroupType("single tau \& photons + MET", "singletau_photons_met",
                                  filter_func=cls.single_tau_photons_met,
                                  latex_tag=r"1\tau + \gamma's + MET",
                                  root_tag=r"1\tau + #gamma's + MET",
                                  mpl_tag=r"1\tau + \gamma's + MET",
                                  html_tag=r"1\tau + \gamma's + MET",
                                  description="Exactly one tau, at least one photon, a significant amount of MET, and an arbitrary number of b-jets and jets",
                                  description_latex="$1\mathrm{\tau}\, (1+)\gamma\, >100\GeV \met\, +\mathrm{X} $ b-jets or jets",
                                  ))

        groups.append(ECGroupType("double lepton \& photons + MET", "doublelepton_photons_met",
                                  filter_func=cls.double_lepton_photons_met,
                                  latex_tag=r"double lepton + \gamma's + MET",
                                  root_tag=r"double lepton + #gamma's + MET",
                                  mpl_tag=r"double lepton + \gamma's + MET",
                                  html_tag=r"double lepton + \gamma's + MET",
                                  description="Exactly two electrons or muons, at least one photon, a significant amount of MET, and an arbitrary number of b-jets and jets",
                                  description_latex="$2\mathrm{l}\, \,(1+)\gamma\, >100\GeV \,\met\,+ \mathrm{X}$",
                                  ))

        #~ groups.append(ECGroupType("multi lepton & photons + MET", "multilepton_photons_met",
                                  #~ filter_func=cls.multi_lepton_photons_met,
                                  #~ latex_tag=r"multi lepton + \gamma's + MET",
                                  #~ root_tag=r"multi lepton + #gamma's + MET",
                                  #~ mpl_tag=r"multi lepton + \gamma's + MET",
                                  #~ html_tag=r"multi lepton + \gamma's + MET",
                                  #~ description="At least three electrons or muons in any combination, at least one photon, a significant amount of MET, and an arbitrary number of b-jets and jets",
                                  #~ description_latex="$(3+)\mathrm{l}\, \,(1+)\gamma\, >100\GeV \,\met\,+ \mathrm{X} $ b-jets or jets",
                                  #~ ))
        return groups

    @classmethod
    # Some functions to filter classes based on their group
    def filter_objects_only(cls,
                            ec_names,
                            objects,
                            filter_dict=None,
                            cumulative_dict=None):
        ''' Filter class names based on objects given in counnt dicts

            Args:
                ec_names: A list or set of ec_names.
                objects: A list of used ECObjectType names (e.g. Ele).
                filter_dict: A dictionary containing obj => ncount pairs for an exact match with the count.
                cumulative_dict: A dictionary contains obj=>ncount pairs where at least ncount objects need to be present in a class name.

            Returns:
                A set of filtered class names
        '''
        if filter_dict is None:
            filter_dict = {}
        if cumulative_dict is None:
            cumulative_dict = {}
        obj_filter_chunks = []
        object_veto_chunks = []
        mixed_cumulative_object_veto_chunks = []
        general_object_vetos = []
        with_met = False
        # preserve order by using object types instead of orderless countdict
        obj_list_index = 0
        for obj_type in cls.object_types():
            object_cumulative = False
            veto_chunk_added = False
            # First add for cumulative countdict all combinations where
            # the cumulative count is lower than requested to the veto list

            for obj,count in cumulative_dict.items():
                if obj != obj_type.name:
                    continue
                object_cumulative = True
                veto_count = count
                if count > 1:
                    veto_chunk_added = True
                    object_veto_chunks.append([])
                for i in range(1, count):
                    object_veto_chunks[-1].append("{count}{obj}".format(count=i, obj=obj))
                # we also need to add all other cumulative filter options up to their count
                # i.e. for a cumulative dict of {Ele:2, Muon:2 } one needs to also filter
                # 1_Ele_2Muon
            if object_cumulative:
                other_obj_list_index = 0
                for other_obj_type in cls.object_types():
                    if obj_type.name == other_obj_type.name:
                        other_obj_list_index+=1
                        continue
                    for other_obj,other_count in cumulative_dict.items():
                        if other_obj != other_obj_type.name:
                            continue
                        for j in range(1, other_count):
                            if obj_list_index < other_obj_list_index:
                                mixed_cumulative_object_veto_chunks.append("*{count}{obj}*{other_count}{other_obj}*".format(count=count,
                                                                                                                            obj=obj_type.name,
                                                                                                                            other_count=j,
                                                                                                                            other_obj=other_obj))
                            else:
                                mixed_cumulative_object_veto_chunks.append("*{other_count}{other_obj}*{count}{obj}*".format(other_count=j,
                                                                                                                            other_obj=other_obj,
                                                                                                                            count=cumulative_dict[obj_type.name],
                                                                                                                            obj=obj_type.name))
                    other_obj_list_index+=1
            obj_list_index+=1

            # if the object was already used in the cumulative countdict it can't
            # be used for the exclusive filtering with an exact count
            # in this case allow any number for this object as unwanted combinations
            # are filtered.
            if object_cumulative:
                obj_filter_chunks += ["*?{obj}".format(obj=obj_type.name)]
            else:
                # we need to filter all objects which are not requested except for
                # jets and bjets
                if obj_type.name not in filter_dict and not veto_chunk_added and obj_type.name not in ['Jet','bJet']:
                    general_object_vetos.append("Rec_*?{obj}*".format(obj=obj_type.name))
                # add filters for exact requested counts
                for obj, count in filter_dict.items():
                    if obj != obj_type.name:
                        continue
                    # if MET is requested we need to append it
                    # after the differen Jet count distributions
                    if obj == "MET":
                        with_met = True
                        continue

                    else:
                        obj_filter_chunks += ["{count}{obj}".format(count=count, obj=obj)]

        all_filters = []
        if obj_filter_chunks:
            obj_filter = "Rec_" + "_".join(obj_filter_chunks)
            re_filter = [obj_filter,
                         obj_filter + "_?bJet_?Jet",
                         obj_filter + "_?Jet",
                         obj_filter + "_?bJet",
                          ]
            if with_met:
                re_filter = [r + "_1MET" for r in re_filter]

            # allow for all class types
            for class_type_info in cls.class_types():
                for f in re_filter:
                    if class_type_info.name_tag:
                        all_filters.append(f + "+" + class_type_info.name_tag)
            all_filters += re_filter
        else:
            all_filters.append("*")
        all_vetos = general_object_vetos
        if cumulative_dict and object_veto_chunks:
            for combination in itertools.product(*object_veto_chunks):
                veto = "Rec_*" + "*_".join(combination)
                if not veto.endswith("*"):
                    veto += "*"
                all_vetos.append(veto)
            all_vetos += mixed_cumulative_object_veto_chunks

        #print all_filters
        #print all_vetos
        cleaned = super_filter(ec_names, all_filters, all_vetos)
        return cleaned

    @classmethod
    def single_ele_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Ele" : 1}
        return cls.filter_objects_only(ec_names, objects, countdict)

    @classmethod
    def double_ele_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Ele" : 2}
        return cls.filter_objects_only(ec_names, objects, countdict)

    @classmethod
    def single_muon_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Muon" : 1}
        return cls.filter_objects_only(ec_names, objects, countdict)

    @classmethod
    def double_muon_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Muon" : 2}
        return cls.filter_objects_only(ec_names, objects, countdict)

    @classmethod
    def single_tau_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Tau" : 1}
        return cls.filter_objects_only(ec_names, objects, countdict)

    @classmethod
    def double_tau_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Tau" : 2}
        return cls.filter_objects_only(ec_names, objects, countdict)


    @classmethod
    def single_gamma_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"GammaEB" : 1}
        return cls.filter_objects_only(ec_names, objects, countdict)

    @classmethod
    def double_gamma_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"GammaEB" : 2}
        return cls.filter_objects_only(ec_names, objects, countdict)

    @classmethod
    def ele_muon_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Muon" : 1, "Ele" : 1}
        return cls.filter_objects_only(ec_names, objects, countdict)

    @classmethod
    def ele_tau_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Tau" : 1, "Ele" : 1}
        return cls.filter_objects_only(ec_names, objects, countdict)

    @classmethod
    def muon_tau_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Muon" : 1, "Tau" : 1}
        return cls.filter_objects_only(ec_names, objects, countdict)


    @classmethod
    def triple_lepton_sf_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Muon" : 3}
        names = cls.filter_objects_only(ec_names,
                                        objects,
                                        filter_dict=countdict)
        countdict = {"Ele" : 3}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict)
        countdict = {"Tau" : 3}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict)
        return names

    @classmethod
    def triple_lepton_of_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        cumulative_countdict = {"Ele" : 1, "Muon" : 2}
        names = cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=cumulative_countdict)
        cumulative_countdict = {"Ele" : 2, "Muon" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=cumulative_countdict)
        cumulative_countdict = {"Ele" : 2, "Tau" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=cumulative_countdict)
        cumulative_countdict = {"Ele" : 1, "Tau" : 2}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=cumulative_countdict)
        cumulative_countdict = {"Tau" : 2, "Muon" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=cumulative_countdict)
        cumulative_countdict = {"Tau" : 1, "Muon" : 2}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=cumulative_countdict)

        return names

    @classmethod
    def multi_lepton_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        cumulative_countdict = {"Muon" : 4}#x
        names = cls.filter_objects_only(ec_names,
                                        objects,
                                        cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Ele" : 4}#x
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Tau" : 4}#x
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         cumulative_dict=cumulative_countdict)

        filter_countdict = {}#
        cumulative_countdict = {"Ele" : 1, "Muon" : 3}#
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Ele" : 2, "Muon" : 2}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Ele" : 2, "Muon" : 1, "Tau" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Muon" : 2, "Ele" : 1, "Tau" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Tau" : 2, "Ele" : 1, "Muon" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Ele" : 3, "Muon" : 1}#x
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         cumulative_dict=cumulative_countdict,
                                         filter_dict=filter_countdict)
        cumulative_countdict = {"Ele" : 1, "Tau" : 3}#
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Ele" : 2, "Tau" : 2}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Ele" : 3, "Tau" : 1}#x
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         cumulative_dict=cumulative_countdict,
                                         filter_dict=filter_countdict)

        cumulative_countdict = {"Muon" : 1, "Tau" : 3}#
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Muon" : 2, "Tau" : 2}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Muon" : 3, "Tau" : 1}#x
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         cumulative_dict=cumulative_countdict,
                                         filter_dict=filter_countdict)

        return names

    @classmethod
    def single_muon_photons(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Muon" : 1}
        cumulative_countdict = {"GammaEB" : 1}
        names = cls.filter_objects_only(ec_names,
                                        objects,
                                        filter_dict = countdict,
                                        cumulative_dict=cumulative_countdict)
        return names


    @classmethod
    def single_tau_photons(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Tau" : 1}
        cumulative_countdict = {"GammaEB" : 1}
        names = cls.filter_objects_only(ec_names,
                                        objects,
                                        filter_dict = countdict,
                                        cumulative_dict=cumulative_countdict)
        return names


    @classmethod
    def single_ele_photons(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Ele" : 1}
        cumulative_countdict = {"GammaEB" : 1}
        names = cls.filter_objects_only(ec_names,
                                        objects,
                                        filter_dict = countdict,
                                        cumulative_dict=cumulative_countdict)
        return names

    @classmethod
    def double_muon_photons(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Muon" : 2}
        cumulative_countdict = {"GammaEB" : 1}
        names = cls.filter_objects_only(ec_names,
                                             objects,
                                             filter_dict = countdict,
                                             cumulative_dict=cumulative_countdict)
        return names


    @classmethod
    def double_tau_photons(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Tau" : 2}
        cumulative_countdict = {"GammaEB" : 1}
        names = cls.filter_objects_only(ec_names,
                                             objects,
                                             filter_dict = countdict,
                                             cumulative_dict=cumulative_countdict)
        return names

    @classmethod
    def double_ele_photons(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Ele" : 2}
        cumulative_countdict = {"GammaEB" : 1}
        names = cls.filter_objects_only(ec_names,
                                             objects,
                                             filter_dict = countdict,
                                             cumulative_dict=cumulative_countdict)
        return names

    @classmethod
    def ele_muon_photons(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Muon" : 1, "Ele" : 1}
        cumulative_countdict = {"GammaEB" : 1}
        names_ele_muon = cls.filter_objects_only(ec_names,
                                             objects,
                                             filter_dict = countdict,
                                             cumulative_dict=cumulative_countdict)

        return names_ele_muon


    @classmethod
    def ele_tau_photons(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Tau" : 1, "Ele" : 1}
        cumulative_countdict = {"GammaEB" : 1}
        names_ele_muon = cls.filter_objects_only(ec_names,
                                             objects,
                                             filter_dict = countdict,
                                             cumulative_dict=cumulative_countdict)

        return names_ele_muon

    @classmethod
    def muon_tau_photons(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Muon" : 1, "Tau" : 1}
        cumulative_countdict = {"GammaEB" : 1}
        names_ele_muon = cls.filter_objects_only(ec_names,
                                             objects,
                                             filter_dict = countdict,
                                             cumulative_dict=cumulative_countdict)

        return names_ele_muon



    @classmethod
    def triple_lepton_photons(cls, ec_names, objects, jet_threshold=6, **kwargs):
        cumulative_countdict = {"GammaEB" : 1 }
        countdict = {"Muon" : 3}
        names = cls.filter_objects_only(ec_names,
                                        objects,
                                        filter_dict=countdict,
                                        cumulative_dict=cumulative_countdict)
        countdict = {"Ele" : 3}
        names |= cls.filter_objects_only(ec_names,
                                            objects,
                                            filter_dict=countdict,
                                            cumulative_dict=cumulative_countdict)
        countdict = {"Tau" : 3}
        names |= cls.filter_objects_only(ec_names,
                                            objects,
                                            filter_dict=countdict,
                                            cumulative_dict=cumulative_countdict)
        countdict = { "Ele" : 1, "Muon" : 2}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        countdict = { "Ele" : 2, "Muon" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        countdict = { "Tau" : 1, "Muon" : 2}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        countdict = { "Tau" : 2, "Muon" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        countdict = { "Tau" : 1, "Ele" : 2}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        countdict = { "Tau" : 2, "Ele" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        countdict = { "Tau" : 1, "Ele" : 1, "Muon" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)

        return names

    #~ @classmethod
    #~ def multi_lepton_photons(cls, ec_names, objects, jet_threshold=6, **kwargs):
        #~ cumulative_countdict = {"GammaEB" : 1, "Muon" : 4}
        #~ names = cls.filter_objects_only(ec_names,
                                        #~ objects,
                                        #~ cumulative_dict=cumulative_countdict)
        #~ cumulative_countdict = {"GammaEB" : 1, "Ele" : 1, "Muon" : 3}
        #~ names |= cls.filter_objects_only(ec_names,
                                         #~ objects,
                                         #~ cumulative_dict=cumulative_countdict)
        #~ cumulative_countdict = {"GammaEB" : 1, "Ele" : 2, "Muon" : 2}
        #~ names |= cls.filter_objects_only(ec_names,
                                               #~ objects,
                                               #~ cumulative_dict=cumulative_countdict)
        #~ cumulative_countdict = {"GammaEB" : 1, "Ele" : 3, "Muon" : 1}
        #~ names |= cls.filter_objects_only(ec_names,
                                               #~ objects,
                                               #~ cumulative_dict=cumulative_countdict)
        #~ cumulative_countdict = {"GammaEB" : 1, "Ele" : 4}
        #~ names |= cls.filter_objects_only(ec_names,
                                            #~ objects,
                                            #~ cumulative_dict=cumulative_countdict)
        #~ return names

    @classmethod
    def single_ele_met(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Ele" : 1, "MET": 1}
        names = cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict)
        return names

    @classmethod
    def single_muon_met(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Muon" : 1, "MET": 1}
        names = cls.filter_objects_only(ec_names,
                                        objects,
                                        filter_dict=countdict)
        return names

    @classmethod
    def single_tau_met(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Tau" : 1, "MET": 1}
        names = cls.filter_objects_only(ec_names,
                                        objects,
                                        filter_dict=countdict)
        return names


    @classmethod
    def double_lepton_met(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Muon" : 2, "MET": 1}
        names = cls.filter_objects_only(ec_names,
                                             objects,
                                             filter_dict=countdict)
        countdict = {"Ele" : 2, "MET": 1}
        names |= cls.filter_objects_only(ec_names,
                                             objects,
                                             filter_dict=countdict)
        countdict = {"Tau" : 2, "MET": 1}
        names |= cls.filter_objects_only(ec_names,
                                             objects,
                                             filter_dict=countdict)

        return names

    @classmethod
    def triple_lepton_sf_met(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"MET": 1, "Muon" : 3}
        names = cls.filter_objects_only(ec_names,
                                             objects,
                                             filter_dict=countdict)
        countdict = {"Ele" : 3, "MET" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict)
        countdict = {"Tau" : 3, "MET" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict)
        return names

    @classmethod
    def triple_lepton_of_met(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Muon" : 2, "Ele" : 1, "MET" : 1}
        names = cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict)
        countdict = {"Muon" : 2, "Tau" : 1, "MET" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict)
 
        countdict = {"Muon" : 1, "Ele" : 2, "MET" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict)
        countdict = {"Muon" : 1, "Tau" : 2, "MET" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict)
        countdict = {"Tau" : 1, "Ele" : 2, "MET" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict)
        countdict = {"Tau" : 2, "Ele" : 1, "MET" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict)
        countdict = {"Tau" : 1, "Ele" : 1, "Muon" : 1,"MET" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict)

        return names

    @classmethod
    def multi_lepton_met(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"MET": 1}
        cumulative_countdict = {"Muon" : 4}
        names = cls.filter_objects_only(ec_names,
                                             objects,
                                             filter_dict=countdict,
                                             cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Ele" : 1, "Muon" : 3}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)

        cumulative_countdict = {"Ele" : 2, "Muon" : 2}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Ele" : 2, "Tau" : 2}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Ele" : 2, "Tau" : 1, "Muon" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Muon" : 2, "Tau" : 1, "Ele" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Tau" : 2, "Ele" : 1, "Muon" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Tau" : 2, "Muon" : 2}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Ele" : 3, "Muon" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Ele" : 3, "Tau" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Muon" : 3, "Tau" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Tau" : 3, "Muon" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Tau" : 3, "Ele" : 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Ele" : 4}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        cumulative_countdict = {"Tau" : 4}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)

        return names

    @classmethod
    def single_ele_photons_met(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Ele" : 1, "MET": 1}
        cumulative_countdict = {"GammaEB" : 1}
        names = cls.filter_objects_only(ec_names,
                                             objects,
                                             filter_dict=countdict,
                                             cumulative_dict=cumulative_countdict)
        return names

    @classmethod
    def single_muon_photons_met(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Muon" : 1, "MET": 1}
        cumulative_countdict = {"GammaEB" : 1}
        names = cls.filter_objects_only(ec_names,
                                             objects,
                                             filter_dict=countdict,
                                             cumulative_dict=cumulative_countdict)
        return names

    @classmethod
    def single_tau_photons_met(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Tau" : 1, "MET": 1}
        cumulative_countdict = {"GammaEB" : 1}
        names = cls.filter_objects_only(ec_names,
                                             objects,
                                             filter_dict=countdict,
                                             cumulative_dict=cumulative_countdict)
        return names


    @classmethod
    def double_lepton_photons_met(cls, ec_names, objects, jet_threshold=6, **kwargs):
        countdict = {"Muon" : 2, "MET": 1}
        cumulative_countdict = {"GammaEB" : 1}
        names = cls.filter_objects_only(ec_names,
                                        objects,
                                        filter_dict=countdict,
                                        cumulative_dict=cumulative_countdict)
        countdict = {"Ele" : 2, "MET": 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)
        countdict = {"Tau" : 2, "MET": 1}
        names |= cls.filter_objects_only(ec_names,
                                         objects,
                                         filter_dict=countdict,
                                         cumulative_dict=cumulative_countdict)

        return names

    #~ @classmethod
    #~ def multi_lepton_photons_met(cls, ec_names, objects, jet_threshold=6, **kwargs):
        #~ countdict = {"MET": 1}
        #~ cumulative_countdict = {"Muon" : 3, "GammaEB" : 1}
        #~ names = cls.filter_objects_only(ec_names,
                                        #~ objects,
                                        #~ filter_dict=countdict,
                                        #~ cumulative_dict=cumulative_countdict)
        #~ cumulative_countdict = {"Ele" : 3, "GammaEB" : 1}
        #~ names |= cls.filter_objects_only(ec_names,
                                         #~ objects,
                                         #~ filter_dict=countdict,
                                         #~ cumulative_dict=cumulative_countdict)
        #~ cumulative_countdict = {"Ele" : 1, "Muon" : 2, "GammaEB" : 1}
        #~ names |= cls.filter_objects_only(ec_names,
                                         #~ objects,
                                         #~ filter_dict=countdict,
                                         #~ cumulative_dict=cumulative_countdict)
        #~ cumulative_countdict = {"Ele" : 2, "Muon" : 1, "GammaEB" : 1}
        #~ names |= cls.filter_objects_only(ec_names,
                                         #~ objects,
                                         #~ filter_dict=countdict,
                                         #~ cumulative_dict=cumulative_countdict)

        #~ return names
