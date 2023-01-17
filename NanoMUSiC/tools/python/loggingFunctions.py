import logging


LOG_CHOICES = [ 'ERROR', 'WARNING', 'INFO', 'DEBUG', 'CRAB' ]

def setupLogging( logger,
                  loglevel,
                  log_file_name="",
                  quiet_crab=True):
    pyloglevel = loglevel
    if loglevel == 'CRAB':
        pyloglevel = 'ERROR'
    #setup logging
    date = '%F %H:%M:%S'
    format = '%(levelname)s (%(name)s) [%(asctime)s]: %(message)s'
    logging.basicConfig( level=logging._levelNames[ pyloglevel ], format=format, datefmt=date )
    logger.setLevel(logging._levelNames[ pyloglevel ])
    formatter = logging.Formatter( format )
    if log_file_name:
        hdlr = logging.FileHandler( log_file_name, mode='w' )
        hdlr.setFormatter( formatter )
        logger.addHandler( hdlr )
    if not loglevel == 'CRAB' and quiet_crab:
        from CRABClient.UserUtilities import getConsoleLogLevel, setConsoleLogLevel
        from CRABClient.ClientUtilities import LOGLEVEL_MUTE
        logging.getLogger('CRAB3').propagate = False  # Avoid any CRAB message to propagate up to the handlers of the root logger.
        setConsoleLogLevel(LOGLEVEL_MUTE)
