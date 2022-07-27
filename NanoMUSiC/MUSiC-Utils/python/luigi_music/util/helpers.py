import itertools
import os, os.path
import signal
import sys
import time

def chunker(n, iterable):
    it = iter(iterable)
    while True:
       chunk = tuple(itertools.islice(it, n))
       if not chunk:
           return
       yield chunk

class cached_property(object):
    def __init__(self,fget):
        self.fget = fget
        self.func_name = fget.__name__

    def __get__(self,obj,cls):
        if obj is None:
            return None
        value = self.fget(obj)
        setattr(obj,self.func_name,value)
        return value


def restart_luigi( script=None, python=None ):
    pid = os.fork()

    if pid > 0:
        # this is the parent thread
        disable_current_worker()
    else:
        # give the scheduler some time to clean up
        time.sleep( 5.0 )
        restart_python( script=script, python=python )

def disable_current_worker():
    # disable the worker (will not accept new jobs)
    os.kill( os.getpid(), signal.SIGUSR1 )

def restart_python( script=None, python=None ):
    if script is None:
        script = sys.argv[0]
    if python is None:
        python = sys.executable

    args = [ python, script ] + sys.argv[1:]

    # replace the current Python process with these args
    # will keep the same PID on linux
    os.execve( python, args, os.environ )
    # one will never get here

class Struct:
    """ Helper class to construct object from kwargs.

        >>> s = Struct(a=3, b="foo")
        >>> s.a
        3
        >>> s.b
        foo
        >>> s
        Struct(a=3, b="foo")

    """
    def __init__(self, **entries):
        self.__dict__.update(entries)

    def __repr__(self):
        return "{name}({dict})".format(
            name=self.__class__.__name__,
            dict=", ".join( "%s=%r" % ( k, v ) for k, v in self.__dict__.iteritems() )
        )
