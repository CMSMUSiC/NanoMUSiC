from ectools.register import ecroot

class ECRecordList(list):

    def __init__(self, records):
        list.__init__(self,records)

    @property
    def class_names(self):
        return [r["name"] for r in self]

    def filter_in(self, key, value_list):
        return ECRecordList([r for r in self if r[key] in value_list])

    def filter_object_group(self, config, object_group_tag):
        ''' return a record list filtered for one object_group '''
        object_group = ecroot.get_object_group_info_by_name_tag(object_group_tag)
        if object_group:
            selected, filtered = ecroot.filter_class_names(set(self.class_names),
                                                           config,
                                                           object_group=object_group)
            return self.filter_in("name", selected)
        else:
            raise ValueError("No object group found for requested tag %s" % object_group_tag)


    def _property_from_entries(self, key):
        ''' gets a value from all ec records for a key and raises an error if not all are equal '''
        vals = [ d[ key ] for d in self ]
        if len(set(vals)) > 1:
            raise ValueError( "Not all classes in ECRecordList list have same value for {}".format(key) )

    @property
    def lumi(self):
        return self._property_from_entries("lumi")

    @property
    def cme(self):
        return self._property_from_entries("cme")

    @property
    def distribution(self):
        return self._property_from_entries("distribution")

    @property
    def suffix(self):
        return self.get_suffix(self.distribution)

    @classmethod
    def get_suffix(cls, distribution):
        distribution_type = ecroot.get_distribution_type_info_by_name_tag(distribution)
        if distribution_type is None:
            raise ValueError("No distribution type found for requestes tag {}".format(distribution))
        return "_".join(distribution_type.required_object_types)

    @property
    def n_classes_key(self):
        return "nclasses{}".format(self.suffix)

    def record_class_type_loop(self,
                               func,
                               conf,
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
        for class_type in ecroot.class_types():
            if getattr(conf, "filter_" + class_type.name.replace("-", "_")):
                ec_class_types = [class_type]
                break
            ec_class_types.append(class_type)

        for class_type in ec_class_types:
            class_type_option = "filter_" + class_type.name.replace("-", "_")
            setattr(conf, class_type_option, True)
            class_names, dropped = ecroot.filter_class_names(set(self.class_names), conf)
            class_type_results[class_type.name] = func(conf,
                                                       class_type.name,
                                                       self.filter_in('name', class_names),
                                                       **kwargs)
            setattr(conf, class_type_option, False)
        return class_type_results

    def get_class_type_counts(self, conf):
        counts = {}
        stats = self.get_class_type_dominant_process_stats(conf,
                                                           self,
                                                           self.distribution,
                                                           0)
        for class_type, info in stats.items():
            counts[class_type] = {'mc': info[self.n_classes_key],
                                  'data' : info[self.n_classes_key + "_data"]}
        return counts

    def get_class_type_dominant_process_stats(self, conf, quantile, **kwargs):
        return self.record_class_type_loop(cls.get_dominant_process_stats,
                                          conf,
                                          quantile=quantile,
                                          distribution=self.distribution,
                                          **kwargs)

    @classmethod
    def get_dominant_process_stats(cls,
                                   class_type_name,
                                   conf,
                                   ec_records,
                                   quantile=None,
                                   distribution=None):
        infodict = {}
        suffix = cls.get_suffix(distribution)
        dominant_process_statistic = cls.calculate_dominant_process_statistics( ec_records, quantile )
        nclasses = 1. * sum([v for k,v in dominant_process_statistic.items() if k != "data"])
        infodict["nclasses{}".format(suffix)] = nclasses
        infodict["nclasses{}_data".format(suffix)] = 1. * sum([v for k,v in dominant_process_statistic.items() if k == "data"])
        rel_dominant_process_statistic = { k : v / nclasses for k, v in dominant_process_statistic.items() }
        infodict[cls.get_dominant_process_key(quantile, distribution)] = rel_dominant_process_statistic
        return infodict
