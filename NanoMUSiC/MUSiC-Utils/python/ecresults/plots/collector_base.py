from .general import GeneralPlotsBase
from ..util import ECRecordList

class ECCollectorConsumerPlot(GeneralPlotsBase):
    ''' Base class for plots which rely on a ECRecordList item as collected by ECCollector

    This class is derived from GeneralPlotsBase and provides standard filtering of
    ECRecordList items based on the past criteria (object_group_tag)
    '''
    def __init__( self,
                  ec_records,
                  config=None,
                  object_group_tag=None,
                  distribution=None,
                  class_type=None,
                  deviation_type=None):
        ec_records = ECRecordList([r for r in ec_records if r['integral']])
        # cache for ec_records
        self._ec_records = None
        self.object_group_tag = object_group_tag
        # Filter for object group if passed as argument
        if self.object_group_tag:
            ec_records = ec_records.filter_object_group(config,
                                                        object_group_tag)
        if class_type:
            self.class_type = class_type
            ec_records = ec_records.filter_in("class_type", [self.class_type])

        self.ec_records = ec_records
        # Set style
        super( ECCollectorConsumerPlot, self ).__init__( config,
                                                         lumi=self.ec_records.lumi,
                                                         cme=self.ec_records.cme )

    def set_cache_dirty(self):
        self._canvas = None

    @property
    def ec_records(self):
        return self._ec_records

    @ec_records.setter
    def ec_records(self, val):
        self.set_cache_dirty()
        self._ec_records = val

    def valid_class(ec_record, conf):
        if os.path.exists(conf.scan_db_path) and conf.scan_id and (conf.show_p_value or conf.sort_option == "p-value"):
            if ec_record["scan_skipped"]:
                return False
            if ec_record['p-value'] < self.config.min_p_value:
                return False
        return True

    @property
    def is_empty(self):
        return len(self.ec_records) == 0

    @classmethod
    def add_commandline_options( cls, parser, with_parent=True ):

        group = parser.add_argument_group( title="Simpsons plotting options" )
        group.add_argument( "--show-skipped-classes" , help="Include skipped classes in simpsons plosts")
        if with_parent:
            super(ECCollectorConsumerPlot, cls).add_commandline_options(parser)
        else:
            group.add_argument( "--min-p-value", help="Show only p-values above this value", default=0., type=float )


