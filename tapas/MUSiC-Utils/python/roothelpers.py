import math
import os
import sys
from contextlib import contextmanager
import array
from collections import namedtuple

import numpy as np
import matplotlib
import ROOT

ROOT.PyConfig.IgnoreCommandLineOptions = True
ROOT.TH1.AddDirectory( False )
ROOT.TDirectory.AddDirectory( False )

def enable_trace( filename ):
    import os, os.path, sys

    here = os.path.abspath( filename )
    print("Tracing only lines from '%s'." % here)

    cache = {}

    def trace( frame, event, arg ):
        if event == 'line':
            filename = os.path.abspath( frame.f_code.co_filename )
            if here == filename:
                if not filename in cache:
                    cache[filename] = open(filename, 'r').readlines()

                line = cache[filename][frame.f_lineno - 1].strip()
                sys.stderr.write( "TRACE: %s at %s:%d : %s\n" % ( event, os.path.basename( filename ), frame.f_lineno, line ) )

        sys.stderr.flush()
        return trace

    sys.settrace( trace )

def root_setup():
    ROOT.gROOT.ProcessLine( '#include <set>' )

    library_name = 'libTEventClass.so'
    for lib in ROOT.gSystem.GetLibraries( "", "D" ).split( " " ):
        if lib.endswith( library_name ):
            #  "TEventClass already loaded from '%s'.\n" % lib
            return

    pxlana_path = os.getenv( 'MUSIC_UTILS' )
    if not pxlana_path:
        raise EnvironmentError( "MUSIC_UTILS environment variable not set" )
    if not os.path.isdir( pxlana_path ):
        raise IOError( "MUSIC_UTILS path does not exist" )

    shared_lib = os.path.join( pxlana_path, 'lib', library_name )
    if not os.path.isfile( shared_lib ):
        raise IOError( "Shared library '%s' does not exist" % shared_lib )

    retval = ROOT.gSystem.Load( shared_lib )
    if retval != 0:
        raise IOError( "Shared library '%s' could not be loaded" % shared_lib )

    libs = ROOT.gSystem.GetLibraries( "", "D" ).split( " " )
    assert shared_lib in libs

@contextmanager
def root_open( filename, *args, **kwargs):
    f = ROOT.TFile.Open( filename, *args, **kwargs )
    try:
        yield f
    finally:
        if f:
            f.Close()

def root_color2hex( index ):
    color = ROOT.gROOT.GetColor( index )
    if not color:
        return None
    r, g, b = color.GetRed(), color.GetGreen(), color.GetBlue()
    return matplotlib.colors.rgb2hex( ( r, g, b ) )

def root_map_hist( func, hist, dummy= False):
    ''' Apply function func to each entry in a hist and return the resulting array.
        option dummy may be used to receive an array of zeros corresponding to a
        empty hist with same  number of bins '''
    N = hist.GetNbinsX()
    arr = np.zeros( N )
    if dummy:
        return arr
    for i in xrange( N ):
        arr[i] = func( i+1 )
    return arr

def root_load_array( func, hist ):
    N = hist.GetNbinsX()
    buffer = array.array( 'd', [0]*N )
    func( buffer )
    nparr = np.frombuffer( buffer )
    return nparr

def root_hist2arrays( hist, bin_center=True ):
    ''' Convert a root histo to array '''
    ar_x = array.array( "f" )
    ar_y = array.array( "f" )
    for ibin in xrange( hist.GetNbinsX() ):
        ar_y.append( hist.GetBinContent( ibin + 1 ) )
        if bin_center:
            ar_x.append( hist.GetBinCenter( ibin + 1 ) )
        else:
            ar_x.append( hist.GetBinLowEdge( ibin + 1 ) )
    return ar_x, ar_y

def root_sum_hist( hists ):
    if not hists:
        raise ValueError( "Sum over 0 histograms." )
    combined = hists[0].Clone()
    for hist in hists[1:]:
        combined.Add( hist )
    return combined

def root_get_error_hist( hists ):
    ''' Return combined error entries in hist as new hist'''
    combined = root_sum_hist( hists )
    N = combined.GetNbinsX()
    for i in xrange( 1, N+1 ):
        value = 0.
        for hist in hists:
            value = math.sqrt( value**2 + hist.GetBinError( i )**2 )
        combined.SetBinContent( i, value )
    return combined

def root_sqsum_hist( hists ):
    if not hists:
        raise ValueError( "Squared sum over 0 histograms." )

    combined = hists[0].Clone()
    N = combined.GetNbinsX()
    for i in xrange( 1, N+1 ):
        value = 0
        for hist in hists:
            value += math.pow( hist.GetBinContent( i ), 2 )
        value = math.sqrt( value )
        combined.SetBinContent( i, value )
    return combined

def root_last_filled_bin( hist, thresh = 1e-4 ):
    last = 0
    foundlast = False
    for i in range( hist.GetNbinsX() + 1):
        if abs( hist.GetBinContent( i ) ) < thresh and not foundlast:
            last = max( 0, i-1 )
            foundlast = True
        elif abs( hist.GetBinContent( i ) ) > thresh:
            foundlast = False
    return last

def root_latex_convert( texstring ):
    root_textstring = texstring.replace("$", "")
    root_textstring =  root_textstring.replace("\\", "#" )
    return root_textstring

def root_first_filled_bin( hist ):
    last = 0
    foundlast = False
    for i in range( hist.GetNbinsX() + 1):
        if abs( hist.GetBinContent( i ) ) > 1e-12:
            return i
    return i

def root_absolute_difference( hist1, hist2, sign_if_first_negative=False):
    N = hist1.GetNbinsX()
    if hist2.GetNbinsX() != N:
        raise ValueError( "Different number of bins encountered while computing difference." )

    combined = hist1.Clone()
    success = combined.Add( hist2, -1 )
    if not success:
        raise ValueError( "Error while adding histograms." )

    # Performance optimization and ensurance to be always positive
    if combined.GetMaximum() < 0:
        combined.Scale( -1 )

    if combined.GetMinimum() < 0:
        for ibin in range( combined.GetNbinsX() ):
            if combined.GetBinContent( ibin + 1 ) < 0:
                combined.SetBinContent( ibin + 1, - combined.GetBinContent( ibin + 1 ) )
            if sign_if_first_negative and hist1.GetBinContent(ibin + 1) < 0:
                combined.SetBinContent( ibin + 1, - combined.GetBinContent( ibin + 1 ) )

    return combined


def get_canvas_size(pad):
    """ Get user size of canvas """
    Coordinates = namedtuple("Coordinates", ("xlow", "xup", "ylow", "yup"))
    xlow_canvas = ROOT.Double( 0. )
    ylow_canvas = ROOT.Double( 0. )
    xup_canvas = ROOT.Double( 0. )
    yup_canvas = ROOT.Double( 0. )
    pad.Modified()
    pad.Update()
    pad.GetRangeAxis( xlow_canvas, ylow_canvas, xup_canvas, yup_canvas )
    return Coordinates(xlow_canvas, xup_canvas, ylow_canvas, yup_canvas)


def convert_ndc_axis_to_pad(value, margin1, margin2):
    """ Converts coordinates from NDC with respect to axis to NDC with respect to pad
        margin1 is left or bottom, margin2 right or top
    """
    return value * (1 - margin1 - margin2) + margin1


def convert_ndc_pad_to_axis(value, margin1, margin2):
    """ Converts coordinates from NDC with respect to pad to NDC with respect to axis
        margin1 is left or bottom, margin2 right or top
    """
    return (value - margin1) / (1 - margin1 - margin2)


def convert_user_to_ndc(value, low, up):
    """ Converts coordinates from user system to ndc
        low and up are borders of the canvas
    """
    return (value - low) / (up - low)


def convert_ndc_to_user(value, low, up):
    """ Converts coordinates from ndc system to user
        low and up are borders of the canvas
    """
    return value * (up - low) + low

def convert_tlatex_to_tbox(pad, text):
    """ Converts tlatex object to tbox object """
    coordinates_canvas = get_canvas_size(pad)
    x_ndc_pad = text.GetX()
    y_ndc_pad = text.GetY()
    width_user = text.GetXsize()
    height_user = text.GetYsize()
    # do not use function above since these are RELATIVE values and no ABSOLUTE values
    width_ndc_axis = width_user / (coordinates_canvas.xup - coordinates_canvas.xlow)
    width_ndc_pad = width_ndc_axis * (1 - pad.GetLeftMargin() - pad.GetRightMargin())
    height_ndc_axis = height_user / (coordinates_canvas.yup - coordinates_canvas.ylow)
    height_ndc_pad = height_ndc_axis * (1 - pad.GetBottomMargin() - pad.GetTopMargin())
    align = text.GetTextAlign()
    h_align = align / 10
    v_align = align - 10 * h_align
    if h_align == 1:
        dx = 0
    elif h_align == 2:
        dx = width_ndc_pad / 2
    elif h_align == 3:
        dx = width_ndc_pad
    else:
        raise ValueError( "Invalid horizontal alignment value" )
    if v_align == 1:
        dy = height_ndc_pad
    elif v_align == 2:
        dy = height_ndc_pad / 2
    elif v_align == 3:
        dy = 0
    else:
        raise ValueError( "Invalid vertical alignment value" )
    x_ndc_pad -= dx
    y_ndc_pad -= dy
    return ROOT.TBox(x_ndc_pad, y_ndc_pad, x_ndc_pad + width_ndc_pad, y_ndc_pad - height_ndc_pad)


def get_dummy_hist( objects ):
    dummy = None
    for object in objects:
        if object is None:
            continue
        if "TGraph" in object.ClassName():
            dummy = object.GetHistogram().Clone("dummy")
            dummy.Reset()
        if "TH1" in object.ClassName():
            dummy = object.Clone("dummy")
            dummy.Reset()
        if "THStack" in object.ClassName():
            hists = object.GetHists()
            if hists[0]:
                dummy = hists[0].Clone("dummy")
                dummy.Reset()
    return dummy


# Some Constants
class TextAlign:
    BOTTOM_LEFT         = 11
    BOTTOM_CENTER       = 21
    BOTTOM_RIGHT        = 31
    MIDDLE_LEFT         = 12
    MIDDLE_CENTER       = 22
    MIDDLE_RIGHT        = 32
    TOP_LEFT            = 13
    TOP_CENTER          = 23
    TOP_RIGHT           = 33

class Font:
    # SERIF normal doesn't exist
    SERIF_ITALIC        = 12
    SERIF_BOLD          = 22
    SERIF_BOLD_ITALIC   = 32

    SANS                = 42
    SANS_ITALIC         = 52
    SANS_BOLD           = 62
    SANS_BOLD_ITALIC    = 72

    MONOSPACE           = 82
    MONOSPACE_ITALIC    = 92
    MONOSPACE_BOLD      = 102
    MONOSPACE_BOLD_ITALIC = 112

    GREEK               = 122
    GREEK_ITALIC        = 152
    SYMBOLS             = 142

class Color:
    # Ugly default colors...
    # More colors, see https://root.cern.ch/doc/master/classTColor.html#C01
    WHITE               = 0
    BLACK               = 1
    RED                 = 2
    GREEN               = 3
    BLUE                = 4
    YELLOW              = 5
    PINK                = 6
    TEAL                = 7

class LineStyle:
    # https://root.cern.ch/doc/master/classTAttLine.html#L3
    SOLID               = 1
    DASHED              = 2
    DOTTED              = 3
    DOT_DASHED          = 4
    DOT_DASHED_2        = 5
    DOT_DOT_DOT_DASHED  = 6
    DASHED_2            = 7
    DOT_DOT_DASHED      = 8
    DASHED_3            = 9
    DOT_DASHED_3        = 10


LegendItem = namedtuple( "LegendItem", ( "category", "hist", "label", "style" ) )
