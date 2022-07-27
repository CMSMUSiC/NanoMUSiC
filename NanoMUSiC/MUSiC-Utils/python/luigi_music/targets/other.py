import os, os.path

import luigi

class EnvTarget( luigi.Target ):
    def __init__( self, name ):
        self.name = name

    def exists( self ):
        return self.name in os.environ
