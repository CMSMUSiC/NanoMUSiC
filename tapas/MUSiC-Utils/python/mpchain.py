from __future__ import print_function 

import multiprocessing, multiprocessing.dummy
try:
    import queue
except ImportError:
    import Queue as queue    
import sys
import time
import traceback
import warnings

def _iter_queue( q, stop_event ):
    while True:
        try:
            item = q.get( block=False )
        except queue.Empty:
            if stop_event.is_set():
                break
            # this sleep seems to accelerate execution with processes!
            # (tested with an empty function layer)
            time.sleep( 0.0000001 )
            continue
        else:
            yield item
            q.task_done()
                

def _call_wrapped( func, in_queue=None, out_queue=None, exception_queue=None, stop_event=None, args=tuple(), kwargs=dict() ):
    if in_queue is not None:
        in_iterator = _iter_queue( in_queue, stop_event )
        func_iterator = func( in_iterator, *args, **kwargs )
    else:
        func_iterator = func( *args, **kwargs )
        
    while True:
        try:
            result = next( func_iterator )
        except StopIteration:
            break
        except KeyboardInterrupt:
            pass
        except Exception:
            exc_type, exc, tb = sys.exc_info()
            error_message = "".join( traceback.format_exception( exc_type, exc, tb.tb_next ) )
            exception_queue.put( ( exc, error_message ) )
            break
        else:
            if out_queue is not None:
                out_queue.put( result, block=True )
    
class _Layer:
    def __init__( self, func, args=tuple(), kwargs=dict(), jobs=1, source=False, queue_size=0, mp_api=multiprocessing ):
        # make multiprocessing exchangable with threading via multiprocessing.dummy
        self.mp_api = mp_api
    
        # each layer always *owns* its in_queue, but not necessarily the out_queue
        self.in_queue = self.mp_api.JoinableQueue( queue_size ) if not source else None
        self.exception_queue = self.mp_api.Queue()
        self.out_queue = None
        
        self.stop_event = self.mp_api.Event()
        
        self.func = func
        self.args = args
        self.kwargs = kwargs
        
        self.processes = []
        self.jobs = jobs
        
        assert( jobs >= 1 )
        
    def __str__( self ):
        return "func: %s, processes: %d / %d, waiting( in: %d, out: %d )" \
                    % ( self.func.__name__, len( self.processes ), self.jobs, 
                        self.in_queue.qsize() if self.in_queue else -1,
                        self.out_queue.qsize() if self.out_queue else -1 )
        
    def __repr__( self ):
        return '<Layer: %s>' % str( self )
        
    def __del__( self ):
        self.terminate()
        
    def start( self ):
        self.respawn_workers()
        
    def terminate( self ):
        for process in self.processes:
            # Threads cannot be terminated, thus the entire function is missing.
            if hasattr( process, "terminate" ):
                process.terminate()
            else:
                warnings.warn( "Cannot had-terminate process (might be a thread?).", stacklevel=2 )
    
    def join( self, kill_timeout=None ):
        if self.in_queue is not None and hasattr( self.in_queue, "close" ):
            # Closing and joining queues only works on multiprocessing.JoinableQueues,
            # not on multiprocessing.dummy.JoinableQueue, which is just a basic Queue...
            # So this part will be skipped with threading.
            # It will also be skipped within layers created with source=True
            
            # This (outer) process will not add any more data to this queue
            self.in_queue.close()
            
            # Wait until queue is empty, everything has been worked
            # Since it's closed, it will stay empty afterwards
            # This .join() unfortunately does not take a timeout, so disable it 
            # when a timeout is provided.
            if kill_timeout is None:
                self.in_queue.join()
                
        # Gracefully terminate the main loop, will call close() on this layer's
        # out_queue, which is the in_queue of another layer.
        self.stop_event.set()
            
        for process in self.processes:
            # Wait for the last couple lines to finish (after stop_event has been
            # set), including the teardown function
            process.join( kill_timeout )
            
            # If the process survives the timeout, kill it
            if process.is_alive():
                warnings.warn( "Process is still alive, will be forcefully killed." )
                process.terminate()
                process.join()
            assert not process.is_alive()
            
        self.processes = []
        
    def remove_dead( self ):
        dead_processes = set()
        for process in self.processes:
            if not process.is_alive():
                dead_processes.add( process )
                
        for process in dead_processes:
            process.join()
            self.processes.remove( process )
            
        if not self.processes:
            warnings.warn( "All processes in layer %r have unexpectedly." % self, stacklevel=2 )
            
    def respawn_workers( self ):
        name = self.func.__name__
        
        wrapping_kwargs = {
            "func": self.func,
            "in_queue": self.in_queue,
            "out_queue": self.out_queue,
            "exception_queue": self.exception_queue,
            "stop_event": self.stop_event,
            "args": self.args,
            "kwargs": self.kwargs,
        }
        
        while len( self.processes ) < self.jobs:
            index = len( self.processes )
            process = self.mp_api.Process( target=_call_wrapped, name="%s[%d]" % ( name, index ), kwargs=wrapping_kwargs )
            process.daemon = False
            process.start()
            self.processes.append( process )
            
    def poll_errors( self, action='raise' ):
        action_options = ('ignore', 'print', 'raise')
        if action not in action_options:
            raise ValueError( "action must be one of %r" % action_options )
            
        try:
            exception, error_message = self.exception_queue.get( block=False )
        except queue.Empty:
            return None
        else:
            if action == 'raise':
                raise exception
            elif action == 'print':
                print( "Caught error in child process:", file=sys.stderr )
                print( error_message, file=sys.stderr )
            elif action == 'ignore':
                pass
        return exception
        
    def append( self, layer ):
        # make sure we are not running
        assert not self.processes 
        assert self.out_queue is None or self.out_queue.empty()
        
        self.out_queue = layer.in_queue
        
    def apply( self, item ):
        if self.in_queue is not None:
            self.in_queue.put( item )
        else:
            raise ValueError( "Cannot call apply() on a source layer." )
            
            
class Chain:
    def __init__( self, api="processes" ):
        self.layers = []
        
        if api == "processes":
            self.mp_api = multiprocessing
        elif api == "threads":
            self.mp_api = multiprocessing.dummy
        else:
            raise ValueError( "Invalid API type: %r" % api )
        
    def layer( self, *args, **kwargs ):
        # syntax candy
        layer = _Layer( *args, mp_api=self.mp_api, **kwargs )
        if len( self.layers ) > 0:
            self.layers[-1].append( layer )
        self.layers.append( layer )
        
        return self
        
    def print_status( self ):
        for i, layer in enumerate( self.layers, 1 ):
            print( "%d: %s" % ( i, str( layer ) ) )
        
    def map( self, items ):
        for item in items:
            self.apply( item )
        
    def apply( self, item ):
        self.layers[0].apply( item )
        
    def start( self ):
        for layer in self.layers:
            layer.start()
        
    def join( self, on_error='print', kill_timeout=None ):
        try:
            for layer in self.layers:
                layer.poll_errors( action=on_error )
                layer.join()
                
        except KeyboardInterrupt:
            if kill_timeout:
                print( "Ctrl+C while joining", file=sys.stderr )
                print( "Trying to quit gracefully within %.1f seconds." % kill_timeout )
                for layer in self.layers:
                    layer.join( kill_timeout=kill_timeout )
            else:
                raise

    def terminate( self ):
        for layer in self.layers:
            layer.terminate()
            
    def run_forever( self, respawn=True, on_error='print', kill_timeout=30.0 ):         
        self.start()
        try:
            while True:
                for layer in self.layers:
                    layer.poll_errors( on_error )
                    layer.remove_dead()
                    if respawn:
                        layer.respawn_workers()
                        
                time.sleep( 0 )
        except KeyboardInterrupt:
            pass
        finally:
            self.join( kill_timeout=kill_timeout )
            
 
# Testing Routines
 
import random

def source():
    print("source start")
    for i in range(100):
        yield random.random()
    print("source end")
    
def multiplier(items):
    print("multiplier start")
    for item in items:
        time.sleep(0.1)
        yield 2*item
    print("multiplier end")
    
def printer(iter):
    print("printer start")
    for item in iter:
        print(item)
        yield
    print("printer end")
    
if __name__=="__main__":
    l = []
    
    g = Chain( "processes" ) \
        .layer( source, jobs=1, source=True ) \
        .layer( multiplier, jobs=2, queue_size=5 ) \
        .layer( printer, jobs=1 )
    
    #g.map(range(20000))
    #g.run_forever( respawn=False, on_error='print', kill_timeout=1.0 )
    
    start = time.time()
    g.start()
    g.join( kill_timeout=1.0 )
    g.print_status()
    end = time.time()
    print("Time:", end-start)
    
    #x = multiprocessing.Process( target=func_a, args=(1, ) )
    #x.start()
    #x.join()
    