import os

from ectools.register import ecroot

from ecresults.util import ClassCountMixin

class ECCountTex(ClassCountMixin):

    def __init__(self, config, ec_records, distribution, object_group_tag=None):
        self.config = config
        self.object_group_tag = object_group_tag
        self.distribution = distribution
        # Filter for object group if passed as argument
        if self.object_group_tag:
            ec_records = ec_records.filter_object_group(config,
                                                        object_group_tag)
        self.ec_records = ec_records


    @classmethod
    def get_outfile_name(cls, out, distribution, object_group_tag=None):
        filename = "class_counts{}".format(cls.get_suffix(distribution))
        if object_group_tag:
            filename = "{}_{}".format(filename, object_group_tag)
        return os.path.join(out, "tex", filename + ".tex")

    @property
    def outfile_name(self):
        return self.get_outfile_name(self.config.out,
                                     self.distribution,
                                     self.object_group_tag)

    def write_table(self):
        class_counts = self.ec_records.get_class_type_counts(self.config)
        period = "sixteen"
        suffix = self.get_suffix(self.ec_records.distribution)
        with open(self.outfile_name, "w") as f:
            for class_type, countdict in class_counts.items():
                print "class_type", class_type
                class_type_info = ecroot.get_class_type_info_by_type_name(class_type)
                for data_type, count in countdict.items():
                    key = 'n{}{}{}{}'.format(class_type_info.short_name,
                                             suffix.replace("_","").lower(),
                                             data_type,
                                             period)
                    f.write('\\newcommand{\\%s}{\\ensuremath{{%d}}\\xspace}\n' % (key, count))

