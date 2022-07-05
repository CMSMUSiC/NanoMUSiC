"""Implementation of the ECStyleHelper, ECStatHelper and ECRootHelper classes from ectools

This module contains the music specific implementations for the main helper base classes
ECStyleHelper, ECStatHelper and ECRootHelper in ectools. These classes are registered in
ectools.register and usually used when "ecroot" is imported in other modules

The helper classes are related by inheritance ECMUSiCBaseHelper <- ECMUSiCStatsHelper <- ECMUSiCRootHelper
and all of the contain the full set of available mixins in this module.
"""
from ectools.base import ECStyleHelper
from ectools.stats import ECStatHelper
from ectools.root import ECRootHelper
from ectools.test import ECTestHelper
from ectools.misc import *

# load ec object definitions for MUSiC
from .ec_class_types import ECClassTypeMixin
from .ec_object_types import ECObjectTypeMixin
from .ec_object_groups import ECObjectGroupMixin
from .ec_distribution_types import ECDistributionTypeMixin
from .aggregation_groups import AggregationGroupMixin

class ECMUSiCBaseHelper(ECClassTypeMixin,
                        ECObjectGroupMixin,
                        ECObjectTypeMixin,
                        AggregationGroupMixin,
                        ECDistributionTypeMixin,
                        ECStyleHelper,
                        ):
    ''' Base helper for MUSiC EC analysis '''
    @classmethod
    def add_ec_standard_options(cls,
                                parser,
                                add_ecout_option=True,
                                add_parent=True,
                                **kwargs):
        """ Add standard options for programs running over TEventClass
            containing root files """
        kwargs["add_ecout_option"] = add_ecout_option
        kwargs["add_parent"] = add_parent
        if add_parent:
            parent = super(ECMUSiCBaseHelper, cls)
            parent.add_ec_standard_options(parser, **kwargs)
        parser.add_argument("--njet-threshold",
                            help="Maximum number of jets to consider before"
                                 " merging in one jet inclusive class",
                            default=6)

    @classmethod
    def extra_kwargs(cls, conf):
        return {"jet_threshold": conf.njet_threshold}

    @classmethod
    def n_jets_from_ecname(cls, ec_name):
        prefix, objects, class_tag = cls.split_ec_name(ec_name)
        njets = objects.get('jet', 0)
        nbjets = objects.get('bjet', 0)
        return njets, nbjets


class ECMUSiCStatsHelper(ECMUSiCBaseHelper, ECStatHelper):
    @classmethod
    def add_ec_standard_options(cls,
                                parser,
                                add_parent=True,
                                **kwargs):
        """ Add standard options for programs running over TEventClass
            containing root files """
        if add_parent:
            parent = super(ECMUSiCStatsHelper, cls)
            parent.add_ec_standard_options(parser, **kwargs)
        kwargs['add_parent'] = False


class ECMUSiCRootHelper(ECMUSiCStatsHelper, ECRootHelper):
    @classmethod
    def add_ec_standard_options(cls,
                                parser,
                                add_ecout_option=True,
                                add_parent=True,
                                **kwargs):
        """ Add standard options for programs running over TEventClass
            containing root files """
        kwargs["add_ecout_option"] = add_ecout_option
        kwargs["add_parent"] = add_parent
        if add_parent:
            parent = super(ECMUSiCRootHelper, cls)
            parent.add_ec_standard_options(parser, **kwargs)
        # kwargs["add_parent"] = False
        # kwargs["from"] = "music2"
        # ECRootHelper.add_ec_standard_options(parser, **kwargs)

    @classmethod
    def get_ec_distribution_types(cls, event_class):
        """ Reimplementation using class name as fallback
        """
        dists = super(ECMUSiCStatsHelper,
                      cls).get_ec_distribution_types(event_class)

        if not dists:
            dists = [d.name_tag for d in cls.distribution_types()]
            if "MET" not in event_class.GetName():
                del dists["MET"]
            return dists
        return dists

class ECMUSiCTestHelper (ECMUSiCRootHelper, ECTestHelper):
    pass
