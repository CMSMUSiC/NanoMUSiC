import os
from collections import OrderedDict
from .misc import super_filter

class ECStyleHelper(object):

    @classmethod
    def get_aggregation_groups(cls):
        """ Get dict with aggregation levels for ec process groups

        Function is intended to be reimplemented in specific ec project
        """
        return {}

    @classmethod
    def extra_kwargs(cls, conf):
        ''' extra kwargs for ec projet specific funtions in ec_loops

            Function is intended to be reimplemented in specific ec project information is passed
            via the conf object to the inherited function.
        '''
        return {}

    @classmethod
    def class_types(cls):
        """ Return a dict with meta information about the used class types

        Function is expected to be reimplemented for specific EC project and
        return a list of objects inheriting from ECClassType objects
        """
        return []

    @classmethod
    def distribution_types(cls):
        """ Return a list of ECDistributionType objects

        Function is expected to be reimplemented for specific EC project.
        """
        return []

    @classmethod
    def object_types(cls):
        """ Return a list of ECObjectType objects

        Function is expected to be reimplemented for specific EC project.
        """
        return []

    @classmethod
    def required_object_types(cls):
        """ Return a list of names for ec object types which are required
            for a class to be considered.

            Function is expected to be reimplemented for specific EC project.
        """
        return []

    @classmethod
    def rejected_object_types(cls):
        """ Return a list of names for ec object types which not allowed
            to be included in a class to be considered.

            Function is expected to be reimplemented for specific EC project.
        """
        return []

    @classmethod
    def pluralize(cls, singular):
        """ Return plural for a singular term string"""
        if singular.endswith("s"):
            plural = singular + "es"
        else:
            plural = singular + "s"
        return plural

    @classmethod
    def split_ec_name(cls, name):
        """ Split a given name into its parts:
            prefix, objects, class type tag
        """

        # Split and remove class prefix e.g. 'Gen_' or 'Rec_'.
        prefix, _, rest = name.partition('_')

        # Get class type tag (e.g. '+Njets' or '+X')
        content, _, class_tag = rest.partition("+")

        # Remove everything behind the class_tag indicator '+', i.e., '+X'
        content = content.split("_")

        # handle Empty classes
        if 'Empty' in name:
            return prefix, OrderedDict(), class_tag
        else:
            objects = OrderedDict()
            for part in content:
                num = ''
                obj = ''
                for c in part:
                    if c.isdigit():
                        num += c
                    elif c.isalpha():
                        obj += c
                    # Remove the brackets around the net lepton charge.
                    elif c in '[]':
                        pass
                    else:
                        msg = "Unexpected character '%s' in name of class " % c
                        msg += name
                        raise ValueError(msg)
                objects[obj.lower()] = int(num)

            return prefix, objects, class_tag

    @classmethod
    def join_ec_name(cls, ordered_countdict, class_tag, ec_type="Rec"):
        components = [ec_type]
        for obj, count in ordered_countdict.items():
            object_type = cls.get_object_type_info_by_name_tag(obj)
            components.append("%d%s" % (count, object_type.name))

        ec_name = "_".join(components)
        if class_tag:
            ec_name += "+" + class_tag
        return ec_name

    @classmethod
    def rank_by_objects(cls, ec_name):
        max_object_count = 100
        object_weight_map = {o.name_tag: 10**i * max_object_count for i,o in enumerate(reversed(cls.object_types()))}
        prefix, counts, class_tag = cls.split_ec_name(ec_name)
        return sum(object_weight_map[obj.name_tag] * (max_object_count - counts.get(obj.name_tag, 0)) for obj in cls.object_types())

    @classmethod
    def get_ec_out_path(cls,
                        basepath,
                        ec_name,
                        distribution,
                        out_type="EventClass",
                        fileformat='root'):
        return os.path.join(basepath,
                            fileformat,
                            out_type,
                            distribution,
                            ec_name + distribution + "." + fileformat)

    @classmethod
    def filter_class_names(cls,
                           ec_names,
                           options,
                           object_group=None,
                           distribution=None,
                           **kwargs):
        ''' Apply filter / veto requirements on a set of class names'''
        # First apply all filter criteria from options
        ec_names_good = super_filter(ec_names, options.filter, options.veto)
        ec_names_bad = ec_names.symmetric_difference(ec_names_good)
        objects = [o.name for o in cls.object_types()]
        ec_names_class = set()
        # filter by class type
        ec_class_types = []
        # Check if configuration filters for one classs type exist
        for class_type in cls.class_types():
            conf_name = "filter_" + class_type.name.replace("-", "_")
            if hasattr(options, conf_name) and getattr(options, conf_name):
                ec_class_types = [class_type]
                break
            ec_class_types.append(class_type)

        #make sure that class names correspond to any known class type
        for class_type in ec_class_types:
            cleaned = class_type.filter_func(ec_names_good, objects, **kwargs)
            ec_names_class.update(set(cleaned))
        # check if a object group filter is requested from options
        if hasattr(options, "object_group_tag"):
            object_group = cls.get_object_group_info_by_name_tag(options.object_group_tag)
        # filter class names by object group requirements
        if object_group is not None:
            cleaned = object_group.filter_func(ec_names_class, objects, **kwargs)
            ec_names_class = set(cleaned)
        # fitler class names by distribution type requirements
        if distribution is not None:
            # filter names which do not contain required objects for this
            distribution_type = cls.get_distribution_type_info_by_name_tag(distribution)
            required_objects = distribution_type.required_object_types
            if required_objects:
                ec_names_class = set([n for n in ec_names_class if all(t in n for t in required_objects)])

        # Filter for rejected objects
        ec_names_class = [name for name in ec_names_class if not any(n in name for n in cls.rejected_object_types())]
        # Filter for required objects
        ec_names_class = [name for name in ec_names_class if any(n in name for n in cls.required_object_types())]
        ec_names_class = set(ec_names_class)
        if len(ec_names_class):
            ec_names_good = ec_names_class
            class_type_bad = ec_names_good.symmetric_difference(ec_names_class)
            ec_names_bad.update(class_type_bad)
        else:
            return set(), set(ec_names)
        return ec_names_good, ec_names_bad

    @classmethod
    def get_class_type_info_by_name_tag(cls, class_tag):
        ''' Helper function to get class_type infos by class tag

            Returns the matched object from the list returned by class_types()
        '''
        for class_type in cls.class_types():
            if class_tag == class_type.name_tag:
                return class_type

    @classmethod
    def get_class_type_info_by_type_name(cls, type_name):
        ''' Helper function to get class_type infos by class tag '''
        for class_type in cls.class_types():
            if type_name == class_type.name:
                return class_type

    @classmethod
    def get_class_type_info_by_name(cls, ec_name):
        prefix, objects, class_tag = cls.split_ec_name(ec_name)
        return cls.get_class_type_info_by_name_tag(class_tag)

    @classmethod
    def get_object_type_info_by_name_tag(cls, object_name_tag):
        ''' Helper function to get class_type infos by class tag '''
        for obj in cls.object_types():
            if object_name_tag == obj.name_tag:
                return obj

    @classmethod
    def get_object_type_info_by_type_name(cls, object_type):
        ''' Helper function to get class_type infos by class tag '''
        for object_type in cls.class_types():
            if object_name == object_type.name:
                return object_type

    @classmethod
    def get_object_group_info_by_group_name(cls, group_name):
        ''' Helper function to get object group infos by group name '''
        for object_group in cls.object_groups():
            if group_name == object_group.name:
                return object_group

    @classmethod
    def get_object_group_info_by_name_tag(cls, group_name_tag):
        ''' Helper function to get class_type infos by object group name tag '''
        for object_group in cls.object_groups():
            if group_name_tag == object_group.name_tag:
                return object_group

    @classmethod
    def get_distribution_type_info_by_name(cls, distribution_name):
        ''' Helper function to get object group infos by group name '''
        for distribution_type in cls.distribution_types():
            if distribution_name == distribution_type.name:
                return distribution_type

    @classmethod
    def get_distribution_type_info_by_name_tag(cls, distribution_name_tag):
        ''' Helper function to get class_type infos by object group name tag '''
        for distribution_type in cls.distribution_types():
            if distribution_name_tag == distribution_type.name_tag:
                return distribution_type


    @classmethod
    def latex_ec_name(cls, name, style="mpl"):
        """Convert the event class name to a latex representation."""
        latex_names = {}

        obs_style_infos = cls.object_types()
        prefix, obs, class_tag = cls.split_ec_name(name)

        latex_elements = []
        for ob, count in obs.items():
            obs_style = None
            for obs_st in obs_style_infos:
                if obs_st.name_tag == ob:
                    obs_style = obs_st
                    break
            object_name = getattr(obs_style, style + "_tag")
            if obs_style.pluralize and obs_style.countable and count > 1:
                object_name = cls.pluralize(object_name)

            if obs_style.countable:
                # uncountable objects are e.g. MET
                if style == "root":
                    if len(object_name) > 1 and "#" not in object_name:
                        kernshift = 0.2
                    else:
                        kernshift = 0.35
                    chunk = "%d#kern[%.2f]{%s}" % (count, kernshift, object_name)
                elif style == "latex":
                    chunk = r"%d %s" % (count, object_name)
                else:
                    chunk = "%d %s" % (count, object_name)
            else:
                chunk = object_name


            latex_elements.append(chunk)

        retval = " + ".join(latex_elements)

        # add class type tag in chosen style
        if class_tag:
            class_info = cls.get_class_type_info_by_name_tag(class_tag)
            retval += getattr(class_info, style + "_tag")

        return retval

    @classmethod
    def ec_class_type_loop(cls,
                           func,
                           conf,
                           ec_names,
                           **kwargs):
        """
            Run a loop splitted by each class type defined in
            get_ecname_filter_func_map. all "class_type-only" options are still
            valid.
            calls func(conf, class_type_name, class_names, **kwargs)
        """
        class_type_results = {}
        ec_class_types = []
        # Check if configuration filters for one classs type
        for class_type in cls.class_types():
            if getattr(conf, "filter_" + class_type.name.replace("-", "_")):
                ec_class_types = [class_type]
                break
            ec_class_types.append(class_type)
        all_kwargs = cls.extra_kwargs(conf)
        all_kwargs.update(kwargs)
        for class_type in ec_class_types:
            class_type_option = "filter_" + class_type.name.replace("-", "_")
            setattr(conf, class_type_option, True)
            class_names, dropped = cls.filter_class_names( set(ec_names), conf )
            if not class_names:
                setattr(conf, class_type_option, False)
                continue
            class_type_results[class_type.name] = func(conf,
                                                       class_type.name,
                                                       class_names,
                                                       **all_kwargs)

        return class_type_results

    @classmethod
    def add_ec_standard_options(cls, parser, **kwargs):
        parser.add_argument("--filter",
                            help="Apply this filter on the choice of"
                                 "EventClasses.",
                            default=["*"],
                            nargs="+")
        parser.add_argument("--veto",
                            help="Do not include EventClasses which include"
                                 " one of the listed strings.",
                            default=[],
                            nargs="+")
        parser.add_argument("--keep-vetoed",
                            help="Copy vetoed classes in result file but do"
                                 " not perform any action on them.",
                            action="store_true")
        class_type_group = parser.add_mutually_exclusive_group()
        for class_type in cls.class_types():
            arg_kwargs = {"help": "Use only %s classes" % class_type.name,
                          "action": "store_true"}
            class_type_group.add_argument("--filter-" + class_type.name,
                                          **arg_kwargs)
