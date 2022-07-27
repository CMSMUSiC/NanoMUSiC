import luigi

from luigi_music.tasks import *
from luigi_music.util import restart_python

class DeployAll( luigi.WrapperTask ):
    def requires( self ):
        return PxlAnalyzerEnvTask(), RoIScannerEnvTask(), ConfigsEnvTask()

if __name__ == "__main__":
    luigi.run( main_task_cls=DeployAll )
