from __future__ import print_function

import json
import os, os.path
import warnings
import uuid
from contextlib import contextmanager
import sqlalchemy

import ROOT
from ectools.register import ecroot
import dbtools

import ctypes

from roothelpers import root_last_filled_bin, root_first_filled_bin, \
                        root_latex_convert, TextAlign, Font

from colors import PROCESS_GROUP_STYLES, HistStyle
from cmsstyle import *
from collections import namedtuple

ROOT.gROOT.SetBatch()

ROOT.gErrorIgnoreLevel = ROOT.kWarning


class PlotMarginHelper(object):
    ''' Base class which provides properties for margins based on the current ROOT.gStyle'''
    # Margins of the plotted axis range
    @property
    def canvas_top( self ):
        return 1 - ROOT.gStyle.GetPadTopMargin()

    @property
    def canvas_bottom( self ):
        return ROOT.gStyle.GetPadBottomMargin()

    @property
    def canvas_left( self ):
        return ROOT.gStyle.GetPadLeftMargin()

    @property
    def canvas_right( self ):
        return 1 - ROOT.gStyle.GetPadRightMargin()

    @property
    def text_position_y_info( self ):
        return self.canvas_top + 0.01

class GeneralPlotsBase( PlotMarginHelper ):
    ''' Base class for all MUSiC plots

        This class provides commonly used functions to generate cavases,
        add labels (e.g. header), generalized functions to determine object
        collisions and properties to access scan databases (sdb)
    '''
    def __init__( self,
                  config,
                  lumi=0,
                  cme=13000 ):

        set_cms_root_style()

        # Set Lumi and sqrt s
        self.lumi = lumi
        self.cme = cme

        self.config = config

        self._canvas = None
        self._engine = None

        self.init_text_sizes()

    @contextmanager
    def session( self ):
        ''' A contextmanager object to create and maintain a scan database session '''
        if not os.path.exists(self.config.scan_db_path):
            init_database( self.engine )
        with dbtools.session_scope( self.engine ) as session:
            yield session

    @property
    def engine(self):
        '''Property for a cached sqlalchemy engine object (sqlite)'''
        if self._engine is None:
            self._engine = sqlalchemy.create_engine( 'sqlite:///' + self.config.scan_db_path,
                                                    echo=False )
        return self._engine

    @classmethod
    def get_subfolder(cls, config, **kwargs):
        '''Return the subfolder where the current plot should be saved'''
        return "sub"

    @classmethod
    def get_plot_name(cls, config, **kwargs):
        '''Return the file name (without extension) for the current plot'''
        return "Plotname"

    @classmethod
    def get_output_files(cls, config, name, sub_path):
        ''' Get a list of paths to all expected plot files from this class '''
        paths = []
        for extension in config.formats:
            paths.append(os.path.join( config.out,
                                       extension,
                                       sub_path,
                                       name + "." + extension ))
        return paths

    @property
    def canvas( self ):
        ''' Property for cached TH1F with nclass vs. class yield for Data

            Width and height are determined from the canvas_width and canvas_heigt
            conf options.
        '''
        if self._canvas is None:
            self._canvas = ro.TCanvas( self.get_plot_name(self.config),
                                       "GeneralPlots Canvas",
                                       self.config.canvas_width, int( self.config.canvas_height * 0.8 ) )

        return self._canvas

    def save_plot( self, name, sub_path='' ):
        ''' Save output files for all requested formats '''
        # save to file
        # Create output path, otherwise Print() will fail.
        for path in self.get_output_files(self.config, name, sub_path):
            i = 0
            while i < 3:
                if not os.path.isdir( os.path.dirname(path) ):
                    try:
                        os.makedirs( os.path.dirname(path) )
                    except OSError:
                        i+=1
                        continue
                break
            self.canvas.Print(path)

    def cleanup(self):
        ''' Clean up object (mostly ROOT) which are not correctly deleted from memory'''
        pass

    def tex_header_collision( self ):
        ''' Find collisions between plot objects and header labels and adjust self.max_y '''
        box = convert_tlatex_to_tbox( self.the_pad, self.title_element )
        if self.signal_stack:
            signal_hists = [ hist for hist in self.signal_stack.GetHists() ]
        else:
            signal_hists = []
        difference, maximum = ecroot.generic_collision( self.the_pad,
                                                        box,
                                                        [ self.total_mc_hist,
                                                          self.error_band,
                                                          self.data_graph ] + signal_hists )
        if difference > 0:
            self.max_y = ecroot.calculate_new_canvas_size( self.the_pad, box.GetY1(), maximum )

    def draw_tex_header( self, title_outside=False ):
        ''' Draw a tex header with setting from the conf object

            By default this produces a header with CMS Work in progress,
            the center-of-mass energy and the int. luminosity.

            The defaults can be changed using the following conf options:
            * header_title
            * header_subtitle
            * header_subtitle_text
            * text_position_y_info
            * text_size_header
        '''
        header_text = ""
        if self.config.header_title:
            header_text += "#bf{%s}" % self.config.header_title

        if header_text:
            if title_outside:
                x, y = self.canvas_left, self.text_position_y_info
                align = TextAlign.BOTTOM_LEFT
            else:
                x, y = self.canvas_left + 0.09, self.canvas_top - 0.03
                align = TextAlign.TOP_LEFT

            assert( 0 <= x <= 1 )
            assert( 0 <= y <= 1 )

            self.title_element = ROOT.TLatex( x, y, header_text )
            self.title_element.SetTextAlign( align )
            self.title_element.SetNDC()
            self.title_element.SetTextSize( self.text_size_header )
            self.title_element.SetName( "title_element" )
            self.title_element.SetTextFont( Font.SANS )
            self.title_element.Draw()

        if self.config.header_subtitle:
            header_subtitle_text = "#it{%s}" % self.config.header_subtitle

            if title_outside:
                x, y = self.canvas_left + 0.04, self.text_position_y_info
                align = TextAlign.BOTTOM_LEFT
            else:
                x, y = self.canvas_left + 0.09, self.canvas_top - 0.03 - self.text_size_header * 1.2
                align = TextAlign.TOP_LEFT

            assert( 0 <= x <= 1 )
            assert( 0 <= y <= 1 )

            self.subtitle_element = ROOT.TLatex( x, y, header_subtitle_text )
            self.subtitle_element.SetTextAlign( align )
            self.subtitle_element.SetNDC()
            self.subtitle_element.SetTextSize( self.text_size_header * 0.7 )
            self.subtitle_element.SetName( "title_element" )
            self.subtitle_element.SetTextFont( Font.SANS )
            self.subtitle_element.Draw()

        # Lumi, sqrt(s)
        text = ""
        lumi = self.lumi if not self.config.header_luminosity_overwrite else self.config.header_luminosity_overwrite
        if lumi > 0:
            # Lumi is stored as /pb, displayed as /fb
            text += "%.1f#kern[0.05]{fb^{-1}}" % ( lumi / 1000.0 )

        cme = self.cme if not self.config.header_energy_overwrite else self.config.header_energy_overwrite
        if cme > 0:
            # Center of Momentum Energy is stored in GeV, displayed in TeV
            content =  "%g TeV" % int( cme / 1000. )
            if text:
                text += " (%s)" % content
            else:
                text += content

        if text:
            self.dataset_info_element = ROOT.TLatex( self.canvas_right, self.text_position_y_info, text )
            self.dataset_info_element.SetTextAlign( TextAlign.BOTTOM_RIGHT )
            self.dataset_info_element.SetNDC()
            self.dataset_info_element.SetTextSize( self.text_size_info )
            self.dataset_info_element.SetTextFont( Font.SANS )
            self.dataset_info_element.SetName( "dataset_info_element" )
            self.dataset_info_element.Draw()


    def init_text_sizes( self ):
        ''' Determine tex sizes based on conf options and scale factor and set them in ROOT.gStyle '''
        self.text_size_header = self.config.header_title_text_size * self.config.text_size_scale_factor
        self.text_size_info = self.config.header_info_text_size * self.config.text_size_scale_factor
        self.text_height_correction_factor_legend = ROOT.gStyle.GetPadTopMargin() * ( self.config.text_size_scale_factor - 1 )
        ROOT.gStyle.SetPadTopMargin( ROOT.gStyle.GetPadTopMargin() * self.config.text_size_scale_factor )


    @classmethod
    def add_commandline_options( cls, parser ):
        ''' Add relevant command line options in plotting group to the passed parser

            Args:
                parser: An argparse ArgumentParser instance
        '''
        group = parser.add_argument_group( title="General plotting options" )

        group.add_argument( "--formats", nargs="+", help="Plotting format, each will be created in a separate subfolder.",
            type=str, choices=( "pdf", "png", "root", "eps", "ps", "gif", "svg" ), default=( "pdf", "root" ) )

        group.add_argument( "--out", help="Directory to create plot files in.", type=str, default='./plotOut' )
        group.add_argument( "--cache-dir", help="Directory where cached results should be place", type=str )
        group.add_argument( "--min-yield", default=0., type=float, help="Minimum yield for MC")
        group.add_argument( "--scan-db-path", help="Path to scan database", type=str, default='scandb.sqlite' )
        group.add_argument( "--scan-id", default=None, type=str, help="Either the name or hash for a scan" )
        group.add_argument( "--lumi", help="Luminosity for plotted dataset", default=35922.0, type=float )
        group.add_argument( "--header-title", help="Bold part of header title (e.g. 'CMS')", default="CMS", type=str )
        group.add_argument( "--header-subtitle", help="Normal port of header title (e.g. 'Preliminary').", default="Work in Progress", type=str )
        group.add_argument( "--header-energy-overwrite", help="Center of Mass energy in GeV to show in header.", default=None, type=float )
        group.add_argument( "--header-luminosity-overwrite", help="Luminosity in /pb to show in the header (ONLY!).", default=None, type=float )
        group.add_argument( "--header-luminosity-format", help="Number format and unit for luminosity. Use python number formatting, e.g. %%.2f.", default="%.2f fb^{-1}", type=str )
        group.add_argument( "--header-title-text-size", help="Text size for header title.", default=0.05, type=float )
        group.add_argument( "--header-info-text-size", help="Text size for header info.", default=0.04, type=float )
        group.add_argument( "--header-position-y", help="Vertical location of header.", default=0.88, type=float )

        group.add_argument( "--text-size-scale-factor", help="Relative scale factor for font sizes.", default=1., type=float )

        group.add_argument( "--canvas-width", help="Width of canvas.", default=1400, type=int )
        group.add_argument( "--canvas-height", help="Height of canvas.", default=1050, type=int )

        return group

class GeneralPlots( GeneralPlotsBase ):
    ''' Base class for all plots which represent a single distribution in an event class '''
    def __init__( self,
                  config,
                  distribution,
                  session=None,
                  mc_ec=None,
                  data_ec=None ):

        lumi = data_ec.getLumi() if data_ec is not None else mc_ec.getLumi()
        cme = data_ec.getCME() if data_ec is not None else mc_ec.getCME()

        super( GeneralPlots, self ).__init__( config, lumi=lumi, cme=cme )
        self.distribution = distribution
        # Store the arguments in instance-scope.
        self.data_ec = data_ec
        self.mc_ec = mc_ec
        self.dummy_hist = None
        self.session = session

        self.min_bin_width = self.data_ec.getMinBinWidth() if self.data_ec is not None else self.mc_ec.getMinBinWidth()
        self.ec_name = self.data_ec.GetName() if self.data_ec is not None else self.mc_ec.GetName()

        self.legend_entries = []
        self.legend = None
        self.legend_fillstyle = "F"

        # get plot_case: datamc dataonly, mconly
        self.determine_plot_case()

        # hists
        self.total_mc_hist = None
        #~ self.mc_stack = None
        self.signal_stack = None

        self.data_hist = None

        # cache for scan results
        self._data_scan = None
        self._data_scan_result = None
        self._pseudo_scan = None
        self._p_tilde = None

        # extra bins on upper edge
        self.extra_bins_up = 0

    @classmethod
    def get_plot_name(cls, config, ec_name="Rec_undefined", distribution="Dist", **kwargs):
        ''' Get the plot name from the current class name and distribution

            Args:
                config: A config namespace for this plot
                ec_name: An event class name
                distribution: A ECDistribution type name

            Returns:
                The plot name as a string
        '''
        return ec_name + distribution

    @classmethod
    def get_subfolder(cls, config, distribution="Dist"):
        ''' Get the subfolder name based on the current distribution distribution '''
        key = distribution
        if config.cumulative:
            key += "-cumulative"
        return os.path.join( "GeneralPlots", key )


    @property
    def data_scan(self):
        """ Property for data scan hash defined in config.scan_id

            This property caches the scan database query result

            Returns:
                The matched dbtools Scan object
        """
        if self._data_scan is None and self.session and self.config.scan_id:
            distribution = None
            if hasattr(self, "distribution"):
                distribution = self.distribution
            self._data_scan = self.get_data_scan(self.config.scan_id,
                                                 self.session,
                                                 distribution=distribution)

        return self._data_scan

    @classmethod
    def get_data_scan(cls, scan_id, session, distribution=None):
        """ Get the data scan hash for this scan_id (name or hash) for this session

            Returns:
                The matched dbtools Scan object
        """
        return dbtools.match_hash(scan_id,
                                  session,
                                  allow_name=True,
                                  scan_type="dataScan",
                                  distribution=distribution)

    @property
    def pseudo_scan(self):
        ''' Cached property for a sdb.Scan object for a pseudoScan requested for this plot'''
        if self._pseudo_scan and self.session:
            self._pseudo_scan = dbtools.match_hash(self.config.scan_id,
                                                   self.session,
                                                   allow_name=True,
                                                   scan_type="pseudoScan")
        return self._pseudo_scan

    @property
    def data_scan_result(self):
        ''' Cached property for a sdb.Scan object for a dataScan requested for this plot'''
        if self._data_scan_result is None and self.session and self._data_scan:
            #~ query = session.query( dbtools.Result.score.label("score"),
                                   #~ dbtools.Result.score.label("roi_lower_edge"),
                                   #~ dbtools.Result.score.label("roi_width")) \
            query = self.session.query( dbtools.Result) \
                .filter( dbtools.Result.hash==self.data_scan.hash, dbtools.Result.event_class==self.ec_name )
            if query.count():
                self._data_scan_result = query[ 0 ]
        return self._data_scan_result

    @property
    def p_tilde( self ):
        ''' Get a sdb.Scan object for the dataScan requested for this plot'''
        if self._p_tilde is None and self.session and self.data_scan:

            p_tilde = dbtools.collect_p_tilde_list_class(self.data_scan.hash,
                                                         self.ec_name,
                                                         self.session,
                                                         log10=False)
            if p_tilde:
                self._p_tilde = p_tilde[0]

        return self._p_tilde

    @classmethod
    def check_scan_missing(cls, session, config, ec_name, distribution=None):
        ''' Check if a scan and p-tilde value is available for the current class and distribution

            Args:
                session: A sqlite session object for a scan database
                config: A argparse config namespace as required for this class
                ec_name: The name of the event_class to plot

            Returns:
                A boolean flag to indicate the scans presence
        '''
        # we only expect a scan if a session and scan_id is present
        if session and config.scan_id:
            # try to get the scan hash
            try:
                data_scan = cls.get_data_scan(config.scan_id, session, distribution)
            except dbtools.NoHashFoundException:
                return True
            if not data_scan:
                return True
            # If a data scan was found, make sure this class is also present in
            # the database
            p_tilde = dbtools.collect_p_tilde_list_class(data_scan.hash,
                                                         ec_name,
                                                         session,
                                                         log10=False)

            if not p_tilde:
                return True
        return False

    @property
    def scan_missing(self):
        ''' Property which calls check_scan_missing with the parameters from the current object '''
        return self.check_scan_missing(self.session, self.config, self.ec_name)

    def determine_plot_case( self ):
        ''' Get the plot case based on the presence of data/ mc and signal

            Returns:
                A frozenset (list) containing "data", "mc", "signal" keywords
        '''
        case = []

        if self.data_ec is not None or self.config.data_json is not None:
            case.append( "data" )
        if self.mc_ec is not None:
            case.append( "mc" )
            if self.config.signal:
                case.append( "signal" )

        self.plot_case = frozenset( case )

    @staticmethod
    def is_signal(group):
        ''' Determine if a process group is a signal group '''
        return "signal" in group.lower()


    def label_axis( self, hist, ratio=False, y_axis_label="" ):
        ''' Add labels based on the plotted distribution to a hist

            Args:
                hist: A hist where axis labels should be added
                config: Boolean to control if the plot contains a ratio subplot
                y_axis_label: Alternative y-axis label, "Events / %.0f GeV" % self.mc_ec.getMinBinWidth() is used by default
        '''
        # The X-axis label is applied to the lower plot only. This is the ratio
        # plot if requested, otherwise it's the main histogram.

        # TODO: Take values from ectools
        xlabel = {
            "InvMass": self.config.label_invmass,
            "SumPt": self.config.label_sumpt,
            "MET": self.config.label_met,
        }[ self.distribution ]

        if ratio:
            self.ratio.GetXaxis().SetTitle( xlabel )
        else:
            hist.GetXaxis().SetTitle( xlabel )
        if y_axis_label:
            hist.GetYaxis().SetTitle( y_axis_label )
        else:
            if self.mc_ec:
                hist.GetYaxis().SetTitle( "Events / %.0f GeV" % self.mc_ec.getMinBinWidth() )
            else:
                hist.GetYaxis().SetTitle( "Events / %.0f GeV" % self.data_ec.getMinBinWidth() )

    def create_data_hist( self ):
        ''' Create the data hist for this plot '''
        # plot data

        if self.data_ec is not None:
            if self.config.signal_round == 0:
                # Obtain bin counts summed over all processes (= data samples).
                self.data_hist = ecroot.total_event_yield( self.data_ec, self.distribution )
            elif self.mc_ec.hasDataScan( self.distribution ):
                self.data_hist = self.mc_ec.getDicedDataHisto( self.distribution, self.config.signal_round )
            else:
                raise ValueError( "Signal plot requested, but no scan attached in MC-EC." )

        elif self.config.data_json is not None:
            if self.total_mc_hist is None:
                raise ValueError( "MC Stack not built, cannot deduce binning." )

            with open( self.config.data_json, 'r' ) as file:
                data = json.load( file )

            if data["name"] != self.ec_name or data["distribution"] != self.distribution:
                print( "Got EC: %s, expected: %s" % ( data["name"], self.ec_name ) )
                print( "Got distribution: %s, expected: %s" % ( data["distribution"], self.distribution ) )
                raise ValueError( "Wrong EC/distribution, ignoring JSON input." )

            round_data = data["ScanResults"][ self.config.signal_round ]

            if not "dicedData" in round_data:
                raise ValueError( "JSON does not contain diced data, did you set the verbose flag?" )

            diced_data = round_data[ "dicedData" ]

            # create copy of MC-hist to get the right bin width
            self.data_hist = ROOT.TH1F( self.total_mc_hist )
            assert len(diced_data) == self.data_hist.GetNbinsX()

            for ibin, value in enumerate( diced_data, 1 ):
                self.data_hist.SetBinContent( ibin, value )

        else:
            raise ValueError( "No data source (event class/json) given, but a plot requested." )

        self.data_hist.UseCurrentStyle()

        #self.data_hist.SetBinErrorOption( ROOT.TH1.kPoisson )

        # save integral
        self.total_n_data = self.data_hist.Integral()

        # keeping an unscaled hist will make creating the ratio plot easier
        self.data_hist_unscaled = self.data_hist.Clone()
        if self.config.scale_to_area:
            self.data_hist.Scale( self.min_bin_width, "width" )

        # create cumulative plot if option is used
        if self.config.cumulative:
            sum = 0
            for ibin in reversed( range( self.data_hist.GetNbinsX() ) ):
                sum += self.data_hist.GetBinContent( ibin + 1 ) * self.data_hist.GetBinWidth( ibin + 1 ) / self.min_bin_width
                self.data_hist.SetBinContent( ibin + 1, sum )

        # create cumulative plot if option is used
        if self.config.cumulative:
            sum = 0
            for ibin in reversed( range( self.data_hist_unscaled.GetNbinsX() ) ):
                sum += self.data_hist_unscaled.GetBinContent( ibin + 1 ) * self.data_hist_unscaled.GetBinWidth( ibin + 1 ) / self.min_bin_width
                self.data_hist_unscaled.SetBinContent( ibin + 1, sum )

        self.data_hist.SetLineWidth( 1 )
        if self.config.blind != 13000.:
            for ibin in range( 0, self.data_hist.GetNbinsX() ):
                if self.data_hist.GetBinLowEdge( ibin + 1 ) > self.config.blind:
                    self.data_hist.SetBinContent( ibin + 1, 0 )
                    self.data_hist.SetBinError( ibin + 1, 0 )

    def create_mc_stack( self, category, add_to_legend=True ):
        ''' Create the mc/signal stack and total hists

            Args:
            category: Either "mc" or "signal" to indicate which stack should be created
            add_to_legend: Flag to determine if stack entries should be added to the legend
         '''
        if category not in [ "mc", "signal" ]:
            raise ValueError( "Category must be one of mc or signal." )
        if category == "mc":
            ec = self.mc_ec
            legendstyle = "F"
        else:
            ec = self.signal_ec
            legendstyle = "L"
        if ec is None:
            raise ValueError( "No MC event class given, but a plot requested." )

        # Group ROOT histograms from all processes by process group (as dict).
        process_groups = ecroot.event_yield_by_process_group( ec,
                                                              self.distribution,
                                                              self.config.detailed_groups )

        # Convert dictionary to list of tuples (can be sorted).
        process_groups = list( process_groups.items() )

        # Sort list.
        # SM groups with lowest background contribution come FIRST (!).
        keyfunc = lambda t: t[ 1 ].Integral()
        process_groups.sort( key=keyfunc )

        # Generate stack with unique name for root
        stack = ROOT.THStack( str( uuid.uuid4() ), category )

        if not process_groups:
            raise RuntimeError("No process groups found in %s" % ec.GetName())

        first_hist = True
        for group, hist in process_groups:

            process_group_info = PROCESS_GROUP_STYLES.get( group, HistStyle( label=group, color=33 ) )

            name = root_latex_convert( process_group_info.label )

            # scale to lumi
            hist = self.rescale_mc_to_data( hist )

            if add_to_legend:
                if self.config.legend_include_yield:
                    name += " (%s)" % exponent_notation( hist.Integral() )

                self.legend_entries.append( ( hist.Integral(), hist.GetName(), name ) )

            if category == "mc":
                # keep an unscaled version of total mc contributions
                if first_hist:
                    self.total_mc_hist_unscaled = hist.Clone()
                else:
                    self.total_mc_hist_unscaled.Add( hist )
            else:
                hist.SetLineWidth(3)

            if self.config.scale_to_area:
                hist.Scale( self.min_bin_width, "width" )

            if hist.Integral() < 0 and self.config.ymin_negative_warning:
                warnings.warn( "BG process group '%s' has a negative total contribution." % group )
            else:
                self.all_mc_negative = False

            # create cumulative plot if option is used
            if self.config.cumulative:
                sum = 0
                for ibin in reversed( range( hist.GetNbinsX() ) ):
                    sum += hist.GetBinContent( ibin + 1 ) * hist.GetBinWidth( ibin + 1 ) / self.min_bin_width
                    hist.SetBinContent( ibin + 1, sum )

            # set color
            hist.SetFillStyle( process_group_info.fillstyle )
            if category == "signal":
                hist.SetLineWidth( 2 )
            hist.SetFillColor( process_group_info.color )
            hist.SetLineColor( process_group_info.color )

            if first_hist:
                total_hist = hist.Clone()
                first_hist = False
            else:
                total_hist.Add( hist )

            if category == "signal" and self.total_mc_hist is None:
                raise RuntimeError("The total mc hist needs to be build before signal stack")
            # add hist to stack
            stack.Add( hist )

        # create cumulative plot if option is used
        if self.config.cumulative:
            sum = 0
            for ibin in reversed( range(  self.total_mc_hist_unscaled.GetNbinsX() ) ):
                sum +=  self.total_mc_hist_unscaled.GetBinContent( ibin + 1 ) * self.total_mc_hist_unscaled.GetBinWidth( ibin + 1 ) / self.min_bin_width
                self.total_mc_hist_unscaled.SetBinContent( ibin + 1, sum )

        if category == "mc":
            self.mc_stack = stack
            total_hist.SetFillColor(0)
            total_hist.SetLineColor(0)
            self.total_mc_hist = total_hist
            name = "MC"
            if self.config.legend_include_yield and add_to_legend:
                name += " (%s)" % exponent_notation( self.total_mc_hist_unscaled.Integral(0, self.total_mc_hist_unscaled.GetNbinsX() +1 ) )
                self.legend_entries.append( ( self.total_mc_hist_unscaled.Integral(0, self.total_mc_hist_unscaled.GetNbinsX() + 1 ), total_hist, name ) )
        else:
            self.signal_stack = stack
            self.total_signal_hist = total_hist

    def rescale_mc_to_data( self, hist ):
        ''' Scale the event class to the luminosity in the data ec '''
        if self.data_ec is not None and self.mc_ec is not None:
            if self.data_ec.getLumi() > 0 and self.mc_ec.getLumi() > 0:
                return ecroot.rescale_to_luminosity( hist, self.mc_ec.getLumi(), self.data_ec.getLumi() )
            else:
                warnings.warn("Luminosity values are unphysical, no rescaling is performed.")
                return hist
        else:
            return hist

    def determine_x_axis_range( self ):
        ''' Determine the xaxis range and set self.xmax self.xmin (and self.max_y)

            * Chooses the largest filled bin in data
            * Chooses the largest filled bin in mc if no data is present
            * Determines if a RoI exists and extends the range to include the RoI
        '''
        if "data" in self.plot_case and "mc" in self.plot_case:
            max_bin_mc = self.total_mc_hist.GetMaximumBin()
            max_bin_data = self.data_hist.GetMaximumBin()
            self.max_y = max( self.total_mc_hist.GetBinContent( max_bin_mc ) + self.total_mc_hist.GetBinError( max_bin_mc ),
                        self.data_hist.GetBinContent( max_bin_data ) + self.data_hist.GetBinError( max_bin_data ) )
            max_filled_bin = root_last_filled_bin( self.data_hist )
            min_filled_bin = min( root_first_filled_bin( self.total_mc_hist ), root_first_filled_bin( self.data_hist ) )
        elif "mc" in self.plot_case:
            if self.all_mc_negative:
                max_bin = self.total_mc_hist.GetMinimumBin()
                self.max_y = self.total_mc_hist.GetBinContent( max_bin ) - self.total_mc_hist.GetBinError( max_bin )
            else:
                max_bin = self.total_mc_hist.GetMaximumBin()
                self.max_y = self.total_mc_hist.GetBinContent( max_bin ) + self.total_mc_hist.GetBinError( max_bin )
            max_filled_bin = root_last_filled_bin( self.total_mc_hist, min( self.config.ymin_threshold, 0.001 * abs( self.max_y ) ) )
            min_filled_bin = root_first_filled_bin( self.total_mc_hist )
        elif "data" in self.plot_case:
            max_bin = self.data_hist.GetMaximumBin()
            self.max_y = self.data_hist.GetBinContent( max_bin ) + self.data_hist.GetBinError( max_bin )
            max_filled_bin = root_last_filled_bin( self.data_hist )
            min_filled_bin = root_first_filled_bin( self.data_hist )

        # take one bin more if it is smaller than X percent of the whole range
        if "data" in self.plot_case:
            self.xlow = self.data_hist.GetXaxis().GetBinLowEdge( max( 1, min_filled_bin - 1 ) )

            if float( self.data_hist.GetXaxis().GetBinWidth( max_filled_bin + 2 ) ) / ( self.data_hist.GetXaxis().GetBinUpEdge( max_filled_bin + 2 ) - self.data_hist.GetXaxis().GetBinLowEdge( min_filled_bin ) ) < self.config.max_bin_width_percentage:
                self.xup = self.data_hist.GetXaxis().GetBinUpEdge( max_filled_bin + 2 )
                self.extra_bins_up = 2
            elif float( self.data_hist.GetXaxis().GetBinWidth( max_filled_bin + 1 ) ) / ( self.data_hist.GetXaxis().GetBinUpEdge( max_filled_bin + 1 ) - self.data_hist.GetXaxis().GetBinLowEdge( min_filled_bin ) ) < self.config.max_bin_width_percentage:
                self.xup = self.data_hist.GetXaxis().GetBinUpEdge( max_filled_bin + 1 )
                self.extra_bins_up = 1
            else:
                self.xup = self.data_hist.GetXaxis().GetBinUpEdge( max_filled_bin )
                self.extra_bins_up = 0
        else:
            self.xlow = self.mc_stack.GetXaxis().GetBinLowEdge( min_filled_bin )
            self.xup = self.mc_stack.GetXaxis().GetBinUpEdge( max_filled_bin )

        # check if RoI is in plot range
        if "mc" in self.plot_case and self.config.show_roi and self.data_scan_result is not None:
            roi_lower_edge = self.data_scan_result.roi_lower_edge
            roi_upper_edge = self.data_scan_result.roi_lower_edge + self.data_scan_result.roi_width
            roi_lower_edge_bin_number = self.mc_stack.GetXaxis().FindBin( roi_lower_edge )
            roi_upper_edge_bin_number = self.mc_stack.GetXaxis().FindBin( roi_upper_edge )
            if roi_lower_edge < self.xlow:
                # go bin beneath, but not below 1
                self.xlow = max( 1, self.mc_stack.GetXaxis().GetBinLowEdge( roi_lower_edge_bin_number - 1 ) )
            if roi_upper_edge > self.xup:
                # check as above bin width
                if float( self.mc_stack.GetXaxis().GetBinWidth( roi_upper_edge_bin_number + 2 ) ) / ( self.mc_stack.GetXaxis().GetBinUpEdge( roi_upper_edge_bin_number + 2 ) - self.mc_stack.GetXaxis().GetBinLowEdge( self.mc_stack.GetXaxis().FindBin( self.xlow ) ) ) < self.config.max_bin_width_percentage:
                    self.xup = self.mc_stack.GetXaxis().GetBinUpEdge( roi_upper_edge_bin_number + 2 )
                    self.extra_bins_up = 2
                elif float( self.mc_stack.GetXaxis().GetBinWidth( roi_upper_edge_bin_number + 1 ) ) / ( self.mc_stack.GetXaxis().GetBinUpEdge( roi_upper_edge_bin_number + 1 ) - self.mc_stack.GetXaxis().GetBinLowEdge( self.mc_stack.GetXaxis().FindBin( self.xlow ) ) ) < self.config.max_bin_width_percentage:
                    self.xup = self.mc_stack.GetXaxis().GetBinUpEdge( roi_upper_edge_bin_number + 1 )
                    self.extra_bins_up = 1
                else:
                    self.xup = self.mc_stack.GetXaxis().GetBinUpEdge( roi_upper_edge_bin_number )
                    self.extra_bins_up = 0

        if self.config.xmax is not None: self.xup = self.config.xmax
        if self.config.xmin is not None: self.xlow = self.config.xmin

    def create_legend( self, include_data=True, include_signal=True ):
        ''' Create the self.legend TLegend object

            * The legend is positioned using the plot config legend options and
              adjusted to avoid collisons with other plot elements (if legend_xup is not used)
            * Legend entries are sorted by their total contribution
            * The number of legend columns is adjusted to reduce collisons with other plot elements

            Args:
                include_data: Flag to control if data should be included in the legend
                include_signal: Flag to control if signals should be included in the legend
        '''
        # Get Number of entries in legend. Dict contains only MC
        n_entries = len( self.legend_entries )
        if "data" in self.plot_case and include_data:
            n_entries += 1

        if hasattr( self, '_pad1a' ) and self._pad1a is not None:
            self.the_pad = self.pad1a
        else:
            self.the_pad = self.canvas

        # Set start values
        if self.config.legend_xup:
            self.xup_legend = self.config.legend_xup
            self.yup_legend = self.config.legend_yup
            self.xlow_legend = self.config.legend_xlow
            self.ylow_legend = self.config.legend_ylow
        else:
            self.xup_legend = self.config.legend_xup
            self.yup_legend = self.config.legend_yup - self.text_height_correction_factor_legend
            self.xlow_legend = self.xup_legend - self.config.legend_column_width
            self.ylow_legend = self.yup_legend - self.text_size_legend * n_entries

        n_columns = 1
        relative_differences = []
        max_hist_users = []
        legend_xlows = [ self.xlow_legend ]
        legend_ylows = [ self.ylow_legend ]
        do_not_change_ymax = False

        # Start with 1 column and generate more if necessary
        if not self.config.legend_size_fixed:
            for i in range( self.config.legend_number_columns - 1 ):
                # if no collision we have enough columns
                relative_difference, max_hist_user = self.legend_collision()
                if relative_difference < 0:
                    do_not_change_ymax = True
                    break
                # Increase size in x
                else:
                    relative_differences.append( relative_difference )
                    max_hist_users.append( max_hist_user )
                    self.xlow_legend -= self.config.legend_column_width
                    n_columns += 1

                    # Calculate number of entries per columns
                    n_entries_per_column = n_entries / n_columns
                    if n_entries % n_columns != 0:
                        n_entries_per_column += 1
                    self.ylow_legend = self.yup_legend - self.text_size_legend * n_entries_per_column
                    legend_xlows.append( self.xlow_legend )
                    legend_ylows.append( self.ylow_legend )

        # Get y coordinates of canvas
        xlow_canvas = ctypes.c_double(0) #ROOT.double( 0. )
        ylow_canvas = ctypes.c_double(0) #ROOT.double( 0. )
        xup_canvas = ctypes.c_double(0) #ROOT.double( 0. )
        yup_canvas = ctypes.c_double(0) #ROOT.double( 0. )
        if hasattr( self, '_pad1a' ) and self._pad1a is not None:
            self.pad1a.Modified()
            self.pad1a.Update()
            self.pad1a.GetRangeAxis( xlow_canvas, ylow_canvas, xup_canvas, yup_canvas )
        else:
            self.canvas.Modified()
            self.canvas.Update()
            self.canvas.GetRangeAxis( xlow_canvas, ylow_canvas, xup_canvas, yup_canvas )

        # Add difference for last n_columns (not calculated before)
        relative_difference, max_hist_user = self.legend_collision()
        # Adjust ymax
        if not do_not_change_ymax and relative_difference > 0:
            relative_differences.append( relative_difference )
            max_hist_users.append( max_hist_user )
            min_diff = min( relative_differences )
            index = relative_differences.index( min_diff )
            self.xlow_legend = legend_xlows[ index ]
            self.ylow_legend = legend_ylows[ index ]
            n_columns = index + 1
            # Is log plot?
            if hasattr( self, '_pad1a' ) and self._pad1a is not None:
                is_log = self.pad1a.GetLogy()
            else:
                is_log = self.canvas.GetLogy()

            self.max_y = ecroot.calculate_new_canvas_size( self.the_pad,
                                                           self.ylow_legend,
                                                           max_hist_users[ index ] )

        # Create legend
        self.legend = ROOT.TLegend( self.xlow_legend, self.ylow_legend, self.xup_legend, self.yup_legend , "", "nbNDC")
        self.legend.SetFillStyle( 0 )
        # 0.8 chosen by eye to be a good value
        self.legend.SetTextSize( self.text_size_legend * 0.8 )
        self.legend.SetNColumns( n_columns )

        # Go to correct pad
        if hasattr( self, '_pad1a' ) and self._pad1a is not None:
            self.pad1a.cd()
        else:
            self.canvas.cd()

        tmp_legend_items_ordered = []

        # Add data entry in legend
        if "data" in self.plot_case and include_data:
            # Plot as errorbars, label includes the event count.
            name = self.config.legend_label_data
            if self.config.legend_include_yield:
                name += " (%s)" % exponent_notation( self.total_n_data )

            self.legend.AddEntry( self.data_hist, name, "ELP" )

            # Add dummy (empty) entry to skip later
            tmp_legend_items_ordered.append( None )

        # Sort by integral descending (=reverse)
        for integral, histname, label in sorted( self.legend_entries, key=lambda t: t[0], reverse=True ):
            if include_signal and self.is_signal( label ):
                self.legend.AddEntry( histname, label, "L" )
                tmp_legend_items_ordered.append( None )
            else:
                tmp_legend_items_ordered.append( ( histname, label ) )

        n_entries_mc = len( tmp_legend_items_ordered )
        n_rows = int( math.ceil( float( n_entries_mc ) / n_columns ) )
        for i_row in range( n_rows ):
            i = i_row
            while i < n_entries_mc:
                if tmp_legend_items_ordered[ i ]:
                    histname, label = tmp_legend_items_ordered[ i ]
                    self.legend.AddEntry( histname, label, self.legend_fillstyle  )
                i += n_rows

    def init_text_sizes( self ):
        ''' Init text sizes for plots

            Calls parent class implementation and extends with setting legend text sizes
        '''
        super( GeneralPlots, self ).init_text_sizes()
        self.text_size_legend = self.config.legend_text_size * self.config.text_size_scale_factor

    def draw_legend( self ):
        ''' Draw the legend in the correct pad

            Function chooses the correct TPad if a ratio plot is included
        '''
        if hasattr( self, '_pad1a' ) and self._pad1a is not None:
            self.pad1a.cd()
        else:
            self.canvas.cd()

        self.legend.Draw()

    def draw_tex_header( self ):
        ''' Draw the TeX header for this plot

            This function calls the parent function and extends it with
            a label for the class name and p-tilde values
        '''
        super( GeneralPlots, self ).draw_tex_header( title_outside=False )

        # Class name
        text = ecroot.latex_ec_name( self.ec_name, style= "root")
        self.class_name_text = ROOT.TLatex( self.canvas_left, self.text_position_y_info, text )
        self.class_name_text.SetNDC()
        self.class_name_text.SetTextAlign( TextAlign.BOTTOM_LEFT )
        self.class_name_text.SetTextSize( self.text_size_info )
        self.class_name_text.SetTextFont( Font.SANS )
        self.class_name_text.SetName( "class_name_text" )
        self.class_name_text.Draw()

        # p value
        if self.config.show_p_value and self.mc_ec and self.data_scan_result:
            p_value = self.data_scan_result.score
            p_value_info_text = "p = %.2g" % p_value

            self.p_value_text = ROOT.TLatex( 0.4, self.text_position_y_info, p_value_info_text)
            self.p_value_text.SetNDC()
            self.p_value_text.SetTextAlign( TextAlign.BOTTOM_LEFT )
            self.p_value_text.SetTextSize( self.text_size_info )
            self.p_value_text.SetTextFont( Font.SANS )
            self.p_value_text.SetName( "p_value_text" )
            self.p_value_text.Draw()

        if self.config.show_p_tilde_value and self.p_tilde is not None:
            p_tilde_value_info_text = "#tilde{p} = %.2g" % self.p_tilde

            self.p_tilde_value_text = ROOT.TLatex( 0.57, self.text_position_y_info, p_tilde_value_info_text)
            self.p_tilde_value_text.SetNDC()
            self.p_tilde_value_text.SetTextAlign( TextAlign.BOTTOM_LEFT )
            self.p_tilde_value_text.SetTextSize( self.text_size_info )
            self.p_tilde_value_text.SetTextFont( Font.SANS )
            self.p_tilde_value_text.SetName( "p_tilde_value_text" )
            self.p_tilde_value_text.Draw()

    @classmethod
    def add_legend_cli_options( cls, parser ):
        ''' Classmethod to add relevant command line options for legend plotting options

            Args:
                parser: An argparse ArgumentParser instance
        '''
        group = parser.add_argument_group( title="Legend plotting options" )
        group.add_argument( "--legend-include-yield", help="Include total yield in the legend (e.g. 'Data (3e8)').", default=True, type=bool )

        group.add_argument( "--legend-label-data", help="Legend entry for the data.", default="Data", type=str )
        group.add_argument( "--legend-label-uncert", help="Legend entry for the background uncertainty.", default="BG uncert", type=str )
        group.add_argument( "--legend-text-size", help="Text size to use in legend.", default=0.0375, type=float )

        group.add_argument( "--legend-size-fixed", help="Fix legend size.", default=False, type=bool )
        group.add_argument( "--legend-xlow", help="Left boundary of legend.", default=0.60, type=float )
        group.add_argument( "--legend-xup", help="Right boundary of legend.", default=0.9, type=float )
        group.add_argument( "--legend-ylow", help="Lower boundary of legend.", default=0.4, type=float )
        group.add_argument( "--legend-yup", help="Upper boundary of legend.", default=0.925, type=float )
        group.add_argument( "--legend-number-columns", help="Maximal number of columns.", default=2, type=int )
        group.add_argument( "--legend-column-width", help="Distance between legend columns. Will apparently not go below ROOT minimum.", default=0.2, type=float )

    @classmethod
    def add_label_cli_options( cls, parser ):
        ''' Classmethod to add relevant command line options for distribution labeling options

            Args:
                parser: An argparse ArgumentParser instance
        '''
        group = parser.add_argument_group( title="Axis plotting options" )
        group.add_argument( "--label-invmass", help="X axis label for the InvMass distribution.", default="Mass / GeV", type=str )
        group.add_argument( "--label-sumpt", help="X axis label for the SumPt distribution.", default="#Sigma p_{T} / GeV", type=str )
        group.add_argument( "--label-met", help="X axis label for the MET distribution.", default="MET / GeV", type=str )
        group.add_argument( "--label-transmass", help="X axis label for the InvMass distribution with MET (transverse mass).", default="{M^{T}} / GeV", type=str )


    @classmethod
    def add_distribution_cli_option( cls, parser ):
        ''' Classmethod to add relevant command line options for distribution specific options

            Args:
                parser: An argparse ArgumentParser instance
        '''
        parser.add_argument( "--max-bin-width-percentage", help="One empty bin is added above last data event. " \
                                                            "If it is too wide it won't be added. This is the threshold, " \
                                                            "how big this empty bin is alowed to be with respect to the " \
                                                            "size of the whole histogram.", default=0.1, type=float )
        parser.add_argument( "--signal-round", help="1-based index of signal round to plot.", default=0, type=int )
        parser.add_argument( "--no-stacked-signal", help="Do not stack the signal on top of the MC expectation for plotting", action="store_true" )
        parser.add_argument( "--cumulative", help="Plot cumulative distribution.", action="store_true" )
        parser.add_argument( "--blind", help="Blind data above this threshold.", default=13000., type=float )
        parser.add_argument( "--show-roi", help="Show region of interest (RoI) from scan if available.", default=True, type=bool )

        parser.add_argument( "--data-json", help="JSON file to load diced data from.", default=None, type=str )
        parser.add_argument( "--xmin", help="Minimum for x axis range.", default=None, type=float )
        parser.add_argument( "--xmax", help="Maximum for x axis range.", default=None, type=float )
        parser.add_argument( "--ymin", help="Minimum for y axis range.", default=None, type=float )
        parser.add_argument( "--ymax", help="Maximum for y axis range.", default=None, type=float )

        parser.add_argument( "--scale-to-area", help="Scale bin content to area.", default=True, type=bool )


    @classmethod
    def add_commandline_options( cls, parser ):
        ''' Add relevant command line options in plotting group to the passed parser

            Args:
                parser: An argparse ArgumentParser instance
        '''
        group = GeneralPlotsBase.add_commandline_options( parser )
        group.add_argument( "--ymin-threshold", help="Threshold for minimal bin content for adjusting of x-axis", default=1e-4, type=float )
        group.add_argument( "--ymin-negative-warning", action="store_true",
            help="Give a warning if some processes have total negative contribution" )

        group.add_argument( "--detailed-groups", help="Aggregation level of process groups in plotting groups: " \
                                                           "none: no aggregation (for debugging), " \
                                                           "process: every single sample will be plotted" \
                                                           "medium: normal aggregation (for normal use), " \
                                                           "strong: strong aggregation (for public use) " \
                                                           "total: total aggregation (only Data vs MC) " \
                                                           "custom: implement yourself" \
                                                           "customDY: only DY samples are NOT grouped" \
                                                           "customWJ: only WJ samples are NOT grouped", \
                                    choices=( "none", "medium", "strong", "total", "custom", "testing", "process", "customDY", "customWJ" ), default="medium", type=str )

        group.add_argument( "--show-p-value", help="Show local p-value if available.", default=True, type=bool )
        group.add_argument( "--show-p-tilde-value", help="Show post trial p-tilde value if available.", default=True, type=bool )
        group.add_argument( '-s', '--filter-systematics', default = [], nargs="+", help="Systematics that should be filtered from json" )



        return group

