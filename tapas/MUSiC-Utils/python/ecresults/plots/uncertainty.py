from __future__ import print_function

import os.path
import warnings
import ROOT as ro

from ectools.register import ecroot
from colors import PROCESS_GROUP_STYLES, SYSTEMATIC_STYLES, DETAILED_SYSTEMATIC_STYLES
from roothelpers import root_last_filled_bin, root_first_filled_bin, root_latex_convert, root_hist2arrays
from cmsstyle import *

from .general import GeneralPlots

class ECUncertaintyPlot( GeneralPlots ):
    def __init__( self, config, distribution, signal_ec=None, data_ec=None, mc_ec=None, relative=True, session=None ):
        if mc_ec is None:
            raise ValueError( "No MC event class given, but a plot requested." )

        # Relative or absolute
        self.relative = relative

        self.ec_name = mc_ec.GetName()

        # Fetch EC-specific config (automatically falls back to DEFAULT in configargparser)
        #~ specific_config = config( ec_name + distribution )

        # Call init of base class
        super( ECUncertaintyPlot, self ).__init__( config, distribution, data_ec=data_ec, mc_ec=mc_ec, session=session )
        self.canvas
        # hists
        self.error_stack = ro.THStack( "error_stack", "error_stack" )

        self.init_text_sizes()

        self.mc_stack = None
        self.create_mc_stack( "mc", add_to_legend=False,)
        if "data" in self.plot_case:
            self.create_data_hist()

        # needs to be plottet once before...
        tmp = ro.TCanvas( "ctmp", "", 100, 100 )
        self.mc_stack.Draw()

        self.determine_x_axis_range()

        self.create_error_stack()
        self.create_error_total()

        self.min_yield = config.min_yield

        self.skip = False
        self.skipreason = ""
        name, integral = ecroot.get_class_yields(self.config,
                                                 distribution=self.distribution,
                                                 min_yield=self.min_yield,
                                                 mc_ec=self.mc_ec)
        if integral < self.min_yield:
            self.skip = True
            self.skipreason = "min-yield"
            return

        self.plot()

        self.adjust_axes_to_histogram()

        y_axis_label=""
        if self.relative:
            y_axis_label = "relative uncertainty"
        self.label_axis( self.error_stack, ratio=False,y_axis_label=y_axis_label )

        self.legend_fillstyle = "L"
        self.create_legend( include_data=False )
        self.draw_legend()

        self.resize_y_axis_to_legend()

    @property
    def canvas( self ):
        if self._canvas is None:
            self._canvas = ro.TCanvas( self.get_plot_name(self.config, self.ec_name, self.distribution),
                                       "All BG syst",
                                       self.config.canvas_width, int( self.config.canvas_height * 0.8 ) )
        return self._canvas

    @classmethod
    def get_subfolder(cls, config, ec_name="Rec_Empty", distribution="dist"):
        key = distribution
        if config.cumulative:
            key += "-cumulative"
        prefix, objects, class_tag = ecroot.split_ec_name(ec_name)
        ec_base_name = ecroot.join_ec_name(objects,"")
        return os.path.join( "EventClass", ec_base_name, key )

    @classmethod
    def get_plot_name(cls, config, ec_name="Rec_undefined", distribution="Dist", **kwargs):
        return "Uncert_" + ec_name + distribution

    def get_uncert_group_hist( self, group_infos ):
        # Group ROOT histograms from all processes by systematic name (as dict).

        systematics = ecroot.combined_uncertainty_by_systematic_name(
            self.mc_ec,
            self.distribution,
            filter_systematics=self.config.filter_systematics,
            relative=False )

        systematics[ "stat" ] = ecroot.total_combined_stat_uncertainty( self.mc_ec,
                                                                        self.distribution )

        # Group by systematic group
        groups = {}
        hists_by_systematic =  {}
        for systematic, hist in systematics.items():
            if self.config.print_uncert_integral:
                print(systematic, hist.Integral())
            for name in group_infos:
                if name in systematic:
                    if name not in hists_by_systematic:
                        hists_by_systematic[ name ] = [ hist ]
                    else:
                        hists_by_systematic[ name ].append( hist )
                    break
            else:
                hists_by_systematic[ systematic ] = [ hist.Clone() ]

        for name, hists in hists_by_systematic.items():
            groups[ name ] = root_sqsum_hist( hists )
            if self.config.scale_to_area:
                groups[ name ].Scale( self.min_bin_width, "width" )
            if self.relative:
                groups[ name ].Divide( self.total_mc_hist )

        groups = list( groups.items() )
        groups.sort( key=lambda t: t[ 1 ].Integral(), reverse=True )
        return groups

    def create_error_stack( self ):
        """Plot stacked MC background in the main histogram."""

        if not self.config.detailed_systematics:
            colors = SYSTEMATIC_STYLES
        else:
            colors = DETAILED_SYSTEMATIC_STYLES

        groups = self.get_uncert_group_hist( colors )

        minimum = 1e10
        for zindex, tuple in enumerate( groups ):
            name, hist = tuple

            if hist.Integral() <= 0:
                continue

            if name in colors:
                label = colors[ name ].label
                color = colors[ name ].color
            else:
                label = name
                color = 15

            for ibin in range( 1, hist.GetNbinsX() + 1 ):
                hist.SetBinError( ibin, 0. )
                if hist.GetBinContent( ibin ) > 0 and hist.GetBinContent( ibin ) < minimum and hist.GetBinLowEdge( ibin ) >= self.xlow and hist.GetBinLowEdge( ibin ) < self.xup:
                    minimum = hist.GetBinContent( ibin )
                if hist.GetBinContent( ibin ) < 0.00:
                    #warnings.warn( "Bin Content of histogram %s negative" % hist.GetName() )
                    hist.SetBinContent( ibin, 1e-8 )

            hist.SetLineColor( color )
            hist.SetLineWidth(2)
            hist.SetFillStyle(0)
            hist = self.rescale_mc_to_data( hist )
            self.error_stack.Add( hist )

            self.legend_entries.append( ( hist.Integral(), hist, label ) )

        self.ylow = minimum

    def cleanup( self ):
        self.canvas.Close()
        #delete stuff
        self.error_stack.Delete()
        self.error_total.Delete()
        if self.data_hist:
            self.data_hist.Delete()
        if self.total_mc_hist:
            self.total_mc_hist.Delete()
        del self._canvas

    def plot( self ):
        self.canvas.cd()
        self.set_log_axis()
        self.error_stack.Draw( "hist nostack" )
        self.error_total.Draw( "hist same" )

        self.canvas.Update()

        self.draw_tex_header( )

    def create_error_total( self ):
        """Plot the MC systematic uncertainties, centered around the MC values."""
        self.error_total = ecroot.total_combined_uncertainty(
            self.mc_ec,
            self.distribution,
            filter_systematics=self.config.filter_systematics,
            with_stat=True )
        if not self.error_total:
            return


        self.error_total = self.rescale_mc_to_data( self.error_total )
        if self.config.scale_to_area:
            self.error_total.Scale( self.min_bin_width, "width" )

        if self.relative:
            self.error_total.Divide( self.total_mc_hist )

        for ibin in range( 1, self.error_total.GetNbinsX() + 1 ):
            self.error_total.SetBinError( ibin, 0. )
            if self.error_total.GetBinContent( ibin ) <= 0.00:
                warnings.warn( "Bin Content of histogram %s negative" % "total" )
                self.error_total.SetBinContent( ibin, 1e-8 )

        self.error_total.SetLineColor( 1 )

        self.legend_entries.append( ( self.error_total.Integral(), self.error_total.GetName(), "total" ) )

    def set_log_axis( self ):
        self.canvas.SetLogy( 1 )

    def adjust_axes_to_histogram( self ):
        max_y_err = 0.
        for ibin in range( self.error_total.FindBin( self.xlow ), self.error_total.FindBin( self.xup ) ):
            if self.error_total.GetBinContent( ibin ) > max_y_err:
                max_y_err = self.error_total.GetBinContent( ibin )

        # set plot region for stack
        if self.config.ymin is not None:
            self.error_stack.SetMinimum( self.config.ymin )
        else:
            self.error_stack.SetMinimum( self.ylow * 0.95 )
        if self.config.ymax is not None:
            self.error_stack.SetMaximum( self.config.ymax )
        else:
            if self.relative:
                self.error_stack.SetMaximum( max_y_err * 1.05 )
            else:
                self.error_stack.SetMaximum( max_y_err * 1.05 )
        if self.error_stack.GetXaxis():
            self.error_stack.GetXaxis().SetRangeUser( self.xlow, self.xup )
        if self.error_total.GetXaxis():
            self.error_total.GetXaxis().SetRangeUser( self.xlow, self.xup )

    def legend_collision( self ):
        # Get size of canvas
        xlow_canvas = ro.Double( 0. )
        ylow_canvas = ro.Double( 0. )
        xup_canvas = ro.Double( 0. )
        yup_canvas = ro.Double( 0. )
        self.canvas.Modified()
        self.canvas.Update()
        self.canvas.GetRangeAxis( xlow_canvas, ylow_canvas, xup_canvas, yup_canvas )

        # Get x coordinates and errors and find energy at intersection between legend and axis
        n_points = self.error_total.GetNbinsX()
        x_values = np.zeros( n_points )
        x_errors = np.zeros( n_points )
        for ibin in range( n_points ):
            x_values[ ibin ] = self.error_total.GetBinCenter( ibin + 1 )
            x_errors[ ibin ] = self.error_total.GetBinWidth( ibin + 1 )
        x_hist_range_gev = xup_canvas - xlow_canvas
        xlow_hist_gev = x_hist_range_gev * self.xlow_legend + xlow_canvas
        xup_hist_gev = x_hist_range_gev * self.xup_legend + xlow_canvas

        # Get bin number of intersection
        xlow_hist = 0
        xup_hist = x_values[ n_points - 1 ]
        for i in range( 0, n_points ):
            if x_values[ i ] + x_errors[ i ] > xlow_hist_gev:
                xlow_hist = i
                break
        for i in reversed( range( 0, n_points ) ):
            if x_values[ i ] - x_errors[ i ] < xup_hist_gev:
                xup_hist = i
                break

        # Find maximum in region of legend in number entries
        y_maximum = 1e-10
        for ibin in range( xlow_hist, xup_hist + 1 ):
            y_mc = self.error_total.GetBinContent( ibin + 1 )
            y_maximum = max( y_maximum, y_mc )

        is_log = self.canvas.GetLogy()

        if is_log:
            ymax_relative_position = ( np.log10( y_maximum ) - ylow_canvas ) / ( yup_canvas - ylow_canvas )
        else:
            ymax_relative_position = ( y_maximum - ylow_canvas ) / ( yup_canvas - ylow_canvas )

        # Return if it collides
        return ( ymax_relative_position - self.ylow_legend, y_maximum )

    def resize_y_axis_to_legend( self ):
        if self.config.ymax is None:
            self.error_stack.SetMaximum( self.max_y * 1.05 )
            self.error_total.SetMaximum( self.max_y * 1.05 )

    @classmethod
    def add_commandline_options( cls, parser ):
        super( ECUncertaintyPlot, cls ).add_commandline_options( parser )
        cls.add_distribution_cli_option(parser)
        cls.add_label_cli_options( parser )
        cls.add_legend_cli_options( parser )
        parser.add_argument( "--relative", help="Show relative uncertainties", action='store_true' )
        parser.add_argument( "--detailed-systematics", help="Show less aggeragated systematics uncertainties", action='store_true' )
        parser.add_argument( "--print-uncert-integral", help="Print the integral of each uncert to the prompt for debugging", action="store_true" )
