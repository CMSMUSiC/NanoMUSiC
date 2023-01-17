from ectools.type_definitions import ECClassType
from ectools.misc import *

class ECClassTypeMixin(object):
    """Mixin class which contains all class type definitions for MUSiC (exclusive, inclusive, etc.).

    This class defines the available class types and related meta data to represent
    them in various formats. In addition this module defines filter functions to
    filter for a class type or veto a class based on a n-jet threshold

    """
    # some functions to filter lists of class names based on their class type
    @classmethod
    def class_types(cls):
        '''Return a list of event class types with formating innformation and filter functions.

            Returns:
                A list of ecroot.ECClassType instances
        '''
        class_types = []

        class_types.append(ECClassType("exclusive", "",
                                   short_name="excl",
                                   filter_func=cls.exclusive_only,
                                   mpl_tag=r'\ excl.',
                                   root_tag=r' excl.',
                                   latex_tag=r' excl.',
                                   ))
        class_types.append(ECClassType("inclusive", "X",
                                   short_name="incl",
                                   mpl_tag=r'\ incl.',
                                   root_tag=r'  incl.',
                                   latex_tag=r' incl.',
                                   filter_func=cls.inclusive_only,
                                   ))
        class_types.append(ECClassType("jet-inclusive", "NJets",
                                   short_name="jetincl",
                                   mpl_tag=r'\ jet\ incl.',
                                   root_tag=r'  jet incl.',
                                   latex_tag=r' jet incl.',
                                   filter_func=cls.jet_inclusive_only,
                                   ))
        return class_types

    @classmethod
    def jet_threshold_veto(cls,
                           ec_names,
                           objects,
                           n=6,
                           nmax=20,
                           keep_jet_incl=False):
        '''Filter a list of event class names with a n-jets threshold.

        Args:
            ec_names: A list or set of ec_names.
            objects: A list of used ECObjectType names (e.g. Ele).
            n: Maximum number of jets.
            nmax: Maximum number of which may exist in class names to filter.
            keep_jet_incl: keep the jet-inclusive classes for njets = n case.

        Returns:
            A list of ecroot.ECClassType instances
        '''
        rules = []
        if not keep_jet_incl:
            for obj in objects:
                rules.append("*" + obj + "+NJets")
                rules.append("*" + obj + "_1MET+NJets")
            for i in xrange(1, n):
                rules.append("*%dJet*+NJets*" % i)
        for i in xrange(1, n+1):
            for j in xrange(n-i + 1 , n+1):
                rules.append("*%dJet_%dbJet*" % (j ,i ))
                rules.append("*%dJet_1MET_%dbJet*" % (j ,i ))
        rules.append("*%dJet" % n)
        rules.append("*%dJet_1MET" % n)
        rules.append("*%dJet_1MET_1bJet" % (n -1) )

        for j in xrange(n+1, nmax):
            rules.append("*%dJet*" % j)
            rules.append("*%dbJet*" % j)
        return set(ec_names) - set(multi_filter(ec_names, rules))

    @classmethod
    def restrict_endcap_eles(cls, ec_names):
        '''Filter a list of event class names to contain not more than one endcap electron.

        Args:
            ec_names: A list or set of ec_names.

        Returns:
            A filtered set of event class names
        '''
        rules = []
        for n in range(2,10):
            rules.append("*%dEleEE*" % n)
        return set(ec_names) - set(multi_filter(ec_names, rules))

    @classmethod
    def restrict_single_gamma(cls, ec_names):
        single_gamma_classes = set(multi_filter(ec_names, ["*1GammaEB*"]))
        single_gamma_classes_with_lepton = set(multi_filter(single_gamma_classes, ["*Muon*", "*Ele*"]))#"*Tau*"
        single_gamma_classes_wo_lepton = single_gamma_classes - single_gamma_classes_with_lepton
        return set(ec_names) - set(single_gamma_classes_wo_lepton)

    @classmethod
    def restrict_double_gamma(cls, ec_names):
        double_gamma_classes = set(multi_filter(ec_names, ["Rec_2GammaEB*"]))
        return set(ec_names) - set(double_gamma_classes)

    @classmethod
    def restrict_z_gamma(cls, ec_names):
        z_gamma_classes = set(multi_filter(ec_names, ["Rec_2Muon_1GammaEB*", "Rec_2Ele_1GammaEB*"]))#"Rec_2Tau_1GammaEB*"
        return set(ec_names) - set(z_gamma_classes)
    
    # ****** Yannik changes main ******* 
    @classmethod
    def restrict_TEST_fewclass(cls, ec_names):
        TEST_classes = set(multi_filter(ec_names, ["Rec_2Muon_1MET","Rec_1Ele_1GammaEB_1bJet_3Jet_1MET","Rec_1Ele_1GammaEB_1bJet_4Jet_1MET","Rec_1Ele_1GammaEB_2bJet_2Jet_1MET","Rec_1Ele_1Muon_1GammaEB_1bJet_1Jet_1MET","Rec_1Ele_1Muon_1GammaEB_1bJet_1MET","Rec_1Ele_1Muon_1GammaEB_3Jet_1MET","Rec_1Ele_1Muon_1GammaEB_3bJet_1MET"]))
        return set(TEST_classes)
    # *** END Yannik changes *********
    
    #***** LOR REDUCE TAU CLASSES ****
    @classmethod
    def restrict_multi_tau(cls, ec_names):
        multi_tau_classes = set(multi_filter(ec_names, ["*5Tau*", "*6Tau*", "*7Tau*","*8Tau*","*9Tau*","*10Tau*","*11Tau*","*12Tau*","*13Tau*"]))
        return set(ec_names) - set(multi_tau_classes)

    #**** END LOR REDUCE*****


    @classmethod
    def base_filters(cls, ec_names, objects, jet_threshold, keep_jet_incl=False):
        ec_names = cls.jet_threshold_veto(ec_names, objects, jet_threshold,keep_jet_incl=keep_jet_incl)
        ec_names = cls.restrict_single_gamma(ec_names)
        ec_names = cls.restrict_double_gamma(ec_names)
        ec_names = cls.restrict_z_gamma(ec_names)
        #ec_names = cls.restrict_multi_tau(ec_names) #LOR ADDED to Reduce Tau Classes
        #ec_names = cls.restrict_TEST_fewclass(ec_names) #Yannik changes. Added this line
        return ec_names

    @classmethod
    def inclusive_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        '''Filter a list of event class names to contain only inclusive classes and apply a n-jets threshold.

        Args:
            ec_names: A list or set of ec_names.
            objects: A list of used ECObjectType names (e.g. Ele).
            jet_threshold: Maximum number of jets.
            nmax: Maximum number of which may exist in class names to filter.
            keep_jet_incl: keep the jet-inclusive classes for njets = n case.
            **kwargs: Arbitrary keyword arguments.

        Returns:
            A filtered set of event class names
        '''
        all_incl = set(multi_filter(ec_names, ("*+X", )))
        cleaned = cls.base_filters(all_incl, objects, jet_threshold)
        return cleaned

    @classmethod
    def exclusive_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        '''Filter a list of event class names to contain only exclusive classes and apply a n-jets threshold.

        Args:
            ec_names: A list or set of ec_names.
            objects: A list of used ECObjectType names (e.g. Ele).
            jet_threshold: Maximum number of jets.
            nmax: Maximum number of which may exist in class names to filter.
            keep_jet_incl: keep the jet-inclusive classes for njets = n case.
            **kwargs: Arbitrary keyword arguments.

        Returns:
            A filtered set of event class names
        '''
        excl = set(ec_names) - set(multi_filter(ec_names, ("*+X", ))) \
                             - set(multi_filter(ec_names, ("*+NJets", )))
        cleaned = cls.base_filters(excl, objects, jet_threshold)
        return cleaned

    @classmethod
    def jet_inclusive_only(cls, ec_names, objects, jet_threshold=6, **kwargs):
        '''Filter a list of event class names to contain only jet-inclusive classes and apply a n-jets threshold.

        Args:
            ec_names: A list or set of ec_names.
            objects: A list of used ECObjectType names (e.g. Ele).
            jet_threshold: Maximum number of jets.
            nmax: Maximum number of which may exist in class names to filter.
            keep_jet_incl: keep the jet-inclusive classes for njets = n case.
            **kwargs: Arbitrary keyword arguments.

        Returns:
            A filtered set of event class names
        '''
        jet_incl = set(multi_filter(ec_names, ("*+NJets", )))
        cleaned = cls.base_filters(jet_incl, objects, jet_threshold, keep_jet_incl=True)
        return cleaned
