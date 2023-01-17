#!/bin/env python

import argparse
import ConfigParser
import fnmatch
import ROOT
import logging
from collections import OrderedDict

class PlotAdjusting:

    options = None
    config_parser = ConfigParser.ConfigParser()
    root_file = None
    #logger = logging.getLogger()

    # Init
    def __init__( self ):
        self.parse_arguments()
        numeric_level = getattr( logging, self.options.debug.upper(), None)
        logging.basicConfig( level=numeric_level )
        logging.info( "Adjusting plots" )
        logging.info( "Input file: %s", self.options.file )
        logging.info( "Output file: %s", self.options.output )
        logging.info( "Event Class Names: %s", self.options.name )
        
        logging.debug( "Opening root file %s", self.options.file )
        self.root_file = ROOT.TFile( self.options.file, 'OPEN' )
        logging.debug( "Root file opened" )

        logging.debug( "Reading config file" )
        self.config_parser.read( self.options.output )

        self.loop()

        logging.debug( "Closing root file" )
        self.root_file.Close()
        logging.debug( "Root file closed" )

        logging.debug( "Opening output file" )
        config_file = open( self.options.output, 'w' )
        logging.info( "Writing config file %s", self.options.output )
        self.config_parser.write( config_file )

    # Argparser
    def parse_arguments( self ):

        help = 'Adjust plot and save config\n'
        help += '1. Set axis range by drag and drop.\n'
        help += '2. Set log scale of ratio plot.\n'
        help += '3. Delete ratio plot by deleting the pad (right click -> Delete).\n'
        help += '4. Change legend:\n'
        help += '\t4.1. Drag and drop to change position.\n'
        help += '\t4.2. Right click -> SetNColummns to change number of columns.\n'
        help += '\t4.3. Right click -> SetTextAttributes -> Text to change text size.\n'
        help += '5. Change canvas size by drag and drop.\n'
        help += '6. RoI and text boxes:\n'
        help += '\t6.1. Delete RoI markers to disable them.\n'
        help += '\t6.2. Delete p and p-tilde value text fields to disable them.\n'
        help += '\t6.3. Set text size of all text boxes above plot by\n'
        help += '\t     Right click on class name box -> SetTextAttributes -> Text Size.\n'
        help += '\t6.4. Set text size of CMS title and subtitle by\n'
        help += '\t     Right click on CMS box -> SetTextAttributes -> Text Size.\n'
        help += '\t6.5. Set Y position of CMS title and subtitle by\n'
        help += '\t     Right click on CMS box -> SetY. Do not use drag and drop!\n'
        help += '\t6.6. Set subtitle text by right click on it -> SetText.\n'


        parser = argparse.ArgumentParser( description=help, formatter_class=argparse.RawTextHelpFormatter )
        
        parser.add_argument( 'file',
                            help='ROOT file containing plots' )
        parser.add_argument( '--name', '-n', default='*',
                            help='Name of the Event Class (may use unix filename pattern matching)' )
        parser.add_argument( '--output', '-o', default='plot.cfg',
                            help='Output file' )
        parser.add_argument( "--debug", default="INFO",
                            help="Debug level (DEBUG, INFO, WARNING, ERROR, CRITICAL)" )

        self.options = parser.parse_args()

    # Opens plot and let user adjust it
    def arrange_plot( self, canvas ):
        logging.debug( "Drawing Canvas" )
        canvas.Draw()
        
        # had ratio plot?
        had_ratio = False
        tmp = canvas.GetListOfPrimitives()
        if tmp.FindObject( "pad1a" ):
            logging.debug( "Had ratio plot before adjusting" )
            had_ratio = True

        raw_input( "Press ENTER to exit or go to next plot!" )

        objects = canvas.GetListOfPrimitives()
        if objects.FindObject( "pad1a" ):
            logging.debug( "Found pad for histogram 'pad1a'" )
            pad_histogram = objects.FindObject( "pad1a" )
        else:
            logging.debug( "Found no subpads. Taking canvas" )
            pad_histogram = canvas
            #except ValueError( "Did not find histogram pad in canvas" )
        
        dict = OrderedDict({})

        # Canvas size
        dict[ "CANVAS_HIGHT" ] = canvas.GetWh()
        dict[ "CANVAS_WIDTH" ] = canvas.GetWw()

        # Ratio
        if objects.FindObject( "pad1b" ):
            dict[ "RATIO" ] = True
            pad_ratio = objects.FindObject( "pad1b" )
        else:
            dict[ "RATIO" ] = False

        # Logy
        if dict[ "RATIO" ]:
            dict[ "RATIO_SCALE" ] = pad_ratio.GetLogy()

        # coordinates
        dict[ "XMIN" ] = ROOT.Double( 0. )
        dict[ "XMAX" ] = ROOT.Double( 0. )
        dict[ "YMIN" ] = ROOT.Double( 0. )
        dict[ "YMAX" ] = ROOT.Double( 0. )
        pad_histogram.GetRangeAxis( dict[ "XMIN" ], dict[ "YMIN" ], dict[ "XMAX" ], dict[ "YMAX" ] )
        dict[ "YMIN" ] = pow( 10, dict[ "YMIN" ] )
        dict[ "YMAX" ] = pow( 10, dict[ "YMAX" ] )
        
        # legend, text boxes, RoI markers
        dict[ "SHOW_ROI" ] = False
        dict[ "SHOW_P_VALUE" ] = False
        dict[ "SHOW_P_TILDE_VALUE" ] = False
        if had_ratio:
            pad_objects = pad_histogram.GetListOfPrimitives()
        else:
            pad_objects = objects
            
        for object in pad_objects:
            print object.ClassName(), object.GetName()
            if object.ClassName() == "TLegend":
                legend = object
            if object.ClassName() == "TLine":
                dict[ "SHOW_ROI" ] = True
            if object.ClassName() == "TLatex":
                if object.GetName() == "cmspre":
                    dict[ "HEADER_TITLE" ] = object.GetTitle()
                    dict[ "HEADER_TITLE_TEXT_SIZE" ] = object.GetTextSize()
                    dict[ "HEADER_POSITION_Y" ] = object.GetY()
                    print object.GetY()
                if object.GetName() == "cmspre2":
                    dict[ "HEADER_SUBTITLE" ] = object.GetTitle()
                if object.GetName() == "class_name_text":
                    dict[ "HEADER_INFO_TEXT_SIZE" ] = object.GetTextSize()
                if object.GetName() == "p_value_text":
                    dict[ "SHOW_P_VALUE" ] = True
                if object.GetName() == "p_tilde_value_text":
                    dict[ "SHOW_P_TILDE_VALUE" ] = True
        
        dict[ "TEXT_SIZE_SCALE_FACTOR" ] = 1.

        dict[ "LEGEND_NUMBER_COLUMNS" ] = legend.GetNColumns()

        dict[ "LEGEND_XUP" ] = legend.GetX2NDC()
        dict[ "LEGEND_XLOW" ] = legend.GetX1NDC()
        dict[ "LEGEND_YUP" ] = legend.GetY2NDC()
        dict[ "LEGEND_YLOW" ] = legend.GetY1NDC()
        dict[ "LEGEND_TEXT_SIZE" ] = legend.GetTextSize() / 0.8 # ECPlot code...

        return dict

    # Writes setting of current plot into the config parser
    def write_config( self, dict, class_name ):
        logging.debug( "Adding infos to config" )
        if self.config_parser.has_section( class_name ):
            logging.debug( "Section already in config. Deleting..." )
            self.config_parser.remove_section( class_name )
        logging.debug( "Adding new section" )
        self.config_parser.add_section( class_name )
        for key, value in dict.iteritems():
            logging.debug( "key, value: %s : %s", key, value )
            self.config_parser.set( class_name, key, value )

    # Loops over all Classes in root file and selects the ones to adjust
    def loop( self ):
        class_names = self.options.name
        keys = self.root_file.GetListOfKeys()
        logging.debug( "Looping over %f keys", len( keys ) )
        for key in keys:
            if fnmatch.fnmatch( key.GetName(), class_names ):
                logging.info( "Adjusting event class %s", key.GetName() )
                # class name = canvas name
                canvas = self.root_file.Get( key.GetName() )
                if canvas:
                    dict = self.arrange_plot( canvas )
                    self.write_config( dict, key.GetName() )
                else:
                    logging.error( "No canvas %s found in file %s", key.GetName(), self.options.file )

def main():
    PlotAdjusting()

if __name__ == "__main__":
    main()
