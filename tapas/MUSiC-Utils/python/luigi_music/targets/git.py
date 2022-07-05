import os.path

import luigi

from ..util import GitRepository

class GitTarget( luigi.Target ):
    def __init__( self, repo, commit="master" ):
        if isinstance( repo, GitRepository ):
            self.repo = repo
        else:
            self.repo = GitRepository( repo )
        self.commit = commit

    def exists( self ):
        if not os.path.isdir( self.repo.git_dir ):
            return False

        checked_out = self.repo.head
        desired = self.repo.get_commit( self.commit )

        return checked_out == desired
