import logging
import luigi
import os

from contextlib import contextmanager
from sqlalchemy import text
import sqlalchemy

from dbtools import *

logger = logging.getLogger('luigi-interface')

# function to determine if a class chunk should be added to a job
def missing_rounds_submitted(dbsession,
                           hash,
                           event_class,
                           round_start,
                           round_end):
    res = dbsession.query(JobSplitting) \
                 .filter(JobSplitting.event_class == event_class) \
                 .filter(JobSplitting.hash == hash) \
                 .filter(JobSplitting.round_end >= round_start) \
                 .filter(JobSplitting.round_start <= round_end) \
                 .order_by(JobSplitting.round_start) \
                 .all()
    gaps = []
    # check if we have overlap above
    min_round = min([r.round_start for r in res])
    max_round = max([r.round_end for r in res])

    if min_round > round_start:
        print round_start, min_round
        gaps.append((round_start, min_round -1))

    for i in range(1 , len(res)):
        if res[i].round_start - res[i-1].round_end > 1:
            print res[i].round_start, res[i].round_end
            gaps.append((res[i-1].round_end, res[i].round_start -1 ))

    if max_round < round_end:
        gaps.append((max_round +1 , round_end))

    return gaps

class _ScanDBSessionMixin( object ):
    #~ scan_db_path = "scandb.sqlite"
    _engine = None

    @contextmanager
    def session( self ):
        if not os.path.exists(self.scan_db_path):
            init_database( self.engine )
        #~ self.engine.execute(text("PRAGMA temp_store = 2"))
        with session_scope( self.engine ) as session:
            #~ self.execute(text("PRAGMA temp_store = 2"))
            yield session

    @property
    def engine(self):
        if self._engine is None:
            self._engine = sqlalchemy.create_engine( 'sqlite:///' + self.scan_db_path,
                                                    echo=False )
        return self._engine

    def missing_rounds_scanned(self,
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


