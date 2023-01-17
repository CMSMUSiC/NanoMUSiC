from os import kill
from signal import alarm, signal, SIGALRM, SIGKILL
import subprocess

class TimeoutExpired( Exception ):
    pass

def call( timeout, args ):
    '''
    Run a command with a timeout after which it will be forcibly
    killed.
    '''
    class Alarm( Exception ):
        pass

    def alarm_handler( signum, frame ):
        raise Alarm

    p = subprocess.Popen( args, stdout = subprocess.PIPE, stderr = subprocess.STDOUT )

    signal(SIGALRM, alarm_handler)
    alarm(timeout)

    try:
        stdout, stderr = p.communicate()
        alarm(0)
    except Alarm:
        try:
            kill( p.pid, SIGKILL )
        except OSError:
            pass
        raise TimeoutExpired

    return p.returncode, stdout


def retry( N, timeout, args ):
    """Try to run a call N times when it times out, then fail."""
    num_timeout = 0
    while( True ):
        try:
            return call( timeout, args )
        except TimeoutExpired:
            num_timeout += 1
            if num_timeout >= N:
                raise
            else:
                continue
        except:
            raise
