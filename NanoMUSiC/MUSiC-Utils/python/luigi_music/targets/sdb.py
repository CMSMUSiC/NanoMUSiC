import os
import logging
import luigi
import collections
from contextlib import contextmanager
import sqlalchemy

import dbtools
from dbtools import Result, CorrectedResult
from luigi_music.util import sdb

logger = logging.getLogger('luigi-interface')

class ScanBaseTarget(luigi.target.Target):
    _engine_dict = {}  # dict of sqlalchemy engine instances
    Connection = collections.namedtuple("Connection", "engine pid")
    def __init__(self,
             db_path,
             echo=False,
             connect_args=None,
             engine_prefix='sqlite:///'):

        self.db_path = db_path
        self.echo = echo
        self.connect_args = {}
        self.engine_prefix = engine_prefix
        if connect_args is not None:
            self.connect_args = connect_args

    @property
    def connection_string(self):
        return self.engine_prefix + self.db_path

    @property
    def engine(self):
        """
        Return an engine instance, creating it if it doesn't exist.

        Recreate the engine connection if it wasn't originally created
        by the current process.
        """
        pid = os.getpid()
        conn = ScanResultTarget._engine_dict.get(self.connection_string)
        if not conn or conn.pid != pid:
            # create and reset connection
            engine = sqlalchemy.create_engine(
                self.connection_string,
                connect_args=self.connect_args,
                echo=self.echo
            )
            ScanResultTarget._engine_dict[self.connection_string] = self.Connection(engine, pid)
        return ScanResultTarget._engine_dict[self.connection_string].engine

    @contextmanager
    def session( self ):
        with dbtools.session_scope( self.engine ) as session:
            yield session

class ScanResultTarget(ScanBaseTarget):

    def __init__(self,
                 db_path,
                 hash,
                 ec_name,
                 distribution,
                 first_round,
                 last_round,
                 echo=False,
                 connect_args=None,
                 engine_prefix='sqlite:///'):

        super(ScanResultTarget,self).__init__(db_path,echo,connect_args)
        self.hash = hash
        self.ec_name = ec_name
        self.distribution = distribution
        self.first_round = first_round
        self.last_round = last_round


    def missing_rounds(self,
                       hash,
                       event_class,
                       round_start,
                       round_end):
        rounds = []

        with self.session() as session:
            res = session.query(Result) \
                         .filter(Result.hash == hash) \
                         .filter(Result.event_class == event_class) \
                         .filter(Result.round >= round_start) \
                         .filter(Result.round <= round_end) \
                         .order_by(Result.round) \
                         .all()
            if len(res) == round_end - round_start:
                return []
            rounds = [r.round for r in res]
        return sorted(set(range(round_start, round_end + 1)).difference(rounds))

    def exists(self):
        if not os.path.exists(self.db_path):
            return False
        if self.missing_rounds( self.hash,
                                        self.ec_name,
                                        self.first_round,
                                        self.last_round):
            return False
        return True

class ScanCorrectedResultTarget(ScanBaseTarget):

    def __init__(self,
                 db_path,
                 hash,
                 compared_hash,
                 ec_name,
                 nrounds,
                 echo=False,
                 connect_args=None):

        super(ScanCorrectedResultTarget,self).__init__(db_path,echo,connect_args)
        self.hash = hash
        self.ec_name = ec_name
        self.compared_hash = compared_hash
        self.nrounds = nrounds

    def exists(self):
        if not os.path.exists(self.db_path):
            return False
        if self.missing_rounds( self.hash,
                                self.compared_hash,
                                self.ec_name,
                                self.nrounds):
            return False
        return True

    def missing_rounds(self,
                       hash,
                       compared_hash,
                       event_class,
                       nrounds,
                       ):
        rounds = []

        with self.session() as session:
            res = session.query(CorrectedResult) \
                         .filter(CorrectedResult.hash == hash) \
                         .filter(CorrectedResult.compared_hash == compared_hash) \
                         .filter(CorrectedResult.event_class == event_class) \
                         .filter(CorrectedResult.round >= 0) \
                         .filter(CorrectedResult.round <= nrounds) \
                         .order_by(CorrectedResult.round) \
                         .all()
            if len(res) == nrounds:
                return []
            rounds = [r.round for r in res]
        return sorted(set(range(0, self.nrounds + 1)).difference(rounds))
