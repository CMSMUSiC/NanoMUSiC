# Constant for the luigi_music path
LUIGI_MUSIC_DIR = __path__[0]

LUIGI_MUSIC_LOCAL_CONFIG = "luigi_local.cfg"
LUIGI_MUSIC_LOGGING_CONFIG = LUIGI_MUSIC_DIR + "/luigi_logging.ini"



import luigi as _luigi

# Append "luigi_local.cfg", will get the highest priority
# see https://luigi.readthedocs.io/en/stable/configuration.html
_luigi.configuration.LuigiConfigParser.add_config_path(LUIGI_MUSIC_LOCAL_CONFIG)

# Setup logging early
_luigi.interface.setup_interface_logging(LUIGI_MUSIC_LOGGING_CONFIG)

# Import internal packages
from . import targets
from . import tasks
from . import util

