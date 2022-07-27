from ectools.register import ecroot

class ClassCountMixin(object):

    @property
    def suffix(self):
        return self.get_suffix(self.distribution)

    @classmethod
    def get_suffix(cls, distribution):
        distribution_type = ecroot.get_distribution_type_info_by_name_tag(distribution)
        if distribution_type is None:
            raise ValueError("No distribution type found for requestes tag {}".format(distribution))
        return "_".join(distribution_type.required_object_types)

    @classmethod
    def get_dominant_process_key(cls, quantile, distribution):
        return "dominant_processes_%d%s" % (int(100 * quantile),
                                            cls.get_suffix(distribution))

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

    @classmethod
    def calculate_dominant_process_statistics(cls, ec_records, quantile ):
        dominant_process_dict = {}
        for ec_dict in ec_records:
            max_yield = 0.
            dominant_process = ""
            for proc in ec_dict['integral_process_dict'].values():
                if proc["integral"] > max_yield:
                    max_yield = proc["integral"]
                    dominant_process = proc["name"]
            if not dominant_process in dominant_process_dict:
                dominant_process_dict[ dominant_process ] = 0
            if (max_yield / ec_dict["integral"]) > quantile:
                dominant_process_dict[ dominant_process ] += 1
            else:
                if not "mixed" in dominant_process_dict:
                    dominant_process_dict[ "mixed" ] = 0
                dominant_process_dict[ "mixed" ] += 1
            if "data_integral" in ec_dict and ec_dict['data_integral'] > 0:
                if not "data" in dominant_process_dict:
                    dominant_process_dict["data"] = 0
                dominant_process_dict['data'] +=1
        return dominant_process_dict
