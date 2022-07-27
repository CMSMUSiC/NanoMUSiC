import collections
import math
import ROOT as ro
import numpy as np
import os
import csv

from cmsstyle import exponent_notation, convert_tlatex_to_tbox
from .collector_base import ECCollectorConsumerPlot
from ectools.register import ecroot

class SimpsonsBasePlot(ECCollectorConsumerPlot):

    def __init__( self, ec_records, class_type, config=None, object_group_tag=None):
        # needs to be set befor init because it is needed in canvas name creation
        self.class_type = class_type
        self.ratio = False
        # call parent constructor
        super( SimpsonsBasePlot, self ).__init__( ec_records,
                                                  config=config,
                                                  object_group_tag=object_group_tag,
                                                  class_type=class_type)
        self.canvas
        self.max_y = 0
        if self.config.ymax:
            self.max_y = self.config.ymax

        self.legend_entries = []
        self._header_elements = None
        self.ec_records = self.ec_records.sort_records(self.config.sort_option)

        # check if we show p values and therefore need to differentiate between skipped scans
        if os.path.exists(config.scan_db_path) and config.scan_id and (config.show_p_value or config.sort_option == "p-value"):
            self.ec_records = self.ec_records.filter_skipped(invert=self.config.show_skipped_classes)

        # check if we use a requirement for the minimal p-value
        if self.config.min_p_value:
            self.ec_records = self.ec_records.filter_gt("p-value",
                                                        self.config.min_p_value,
                                                        allow_equal=True)

        #truncate to number of classes to show
        self.n_ranking = min(config.n_ranking, len(self.ec_records))
        self.ec_records = self.ec_records[ : self.n_ranking ]

    @property
    def header_elements(self):
        if self._header_elements is None:
            extra_info_text = "%s classes" % self.class_type
            if self.object_group_tag:
                object_group = ecroot.get_object_group_info_by_name_tag(self.object_group_tag)
                extra_info_text = "%s, %s" % (self.class_type, object_group.root_tag)
            self.header_elements = ecroot.create_tex_header(self.canvas,
                                                  header_title=self.config.header_title,
                                                  header_subtitle=self.config.header_subtitle,
                                                  text_size_header=0.045,
                                                  text_size_info=0.03,
                                                  header_outside=False,
                                                  lumi=self.lumi,
                                                  cme=self.cme,
                                                  dataset_info_outside=False,
                                                  extra_info_text=extra_info_text
                                                )
        return self._header_elements

    @header_elements.setter
    def header_elements(self, val):
        self._header_elements = val

    def sort_records(self):
        if self.config.sort_option == "objects":
            self.ec_records = sorted( self.ec_records, key=lambda k: ecroot.rank_by_objects(k["name"]), reverse=True)
        elif "integral" in self.config.sort_option:
             self.ec_records = reversed(sorted( self.ec_records, key=lambda k: k[ self.config.sort_option ] ))
        else:
            self.ec_records = sorted( self.ec_records, key=lambda k: k[ self.config.sort_option ] )

    @classmethod
    def get_plot_name(cls, config, class_type='class_type', object_group_tag=""):
        kwargs = {'class_type': class_type,
                  'sort_option': config.sort_option,
                  'n_ranking': str(config.n_ranking),
                  'object_group' : object_group_tag }
        if object_group_tag:
            name = "Simpsons_{object_group}_{class_type}_{sort_option}".format(**kwargs)
        else:
            name = "Simpsons_{class_type}_{sort_option}_n_{n_ranking}".format(**kwargs)
        if config.min_p_value:
            name += "_minp_%d" % (config.min_p_value * 100)
        if config.show_skipped_classes:
            name += "_skipped"
        return name

    @classmethod
    def get_subfolder(cls, config, object_group=None, **kwargs):
        if object_group:
            return "Simpsons/{}".format(object_group)
        return "Simpsons"

class SimpsonsPlot( SimpsonsBasePlot ):

    def __init__( self, ec_records, class_type, config=None, object_group_tag=None):

        super( SimpsonsPlot, self ).__init__( ec_records,
                                              class_type,
                                              config=config,
                                              object_group_tag=object_group_tag)

        self.process_hists_dict = collections.OrderedDict()
        self.data_hist = None
        self.syst_hist = None
        self.p_axis = []
        for i, class_dict in enumerate( self.ec_records ):
            self.add_class_mc( i + 1, class_dict )
            self.add_class_syst( i + 1, class_dict )
            self.add_class_data( i + 1, class_dict )
        # do not try to create the plot components if no class survived the filter
        if not self.is_empty:
            self.create_mc_stack()

    def add_class_mc( self, i, class_dict ):
        for proc, proc_dict in class_dict[ "integral_process_dict" ].items():
            if not proc in self.process_hists_dict:
                self.process_hists_dict[ proc ] = {}
                self.process_hists_dict[ proc ][ "mchist" ] = ro.TH1F( proc,
                                                                     proc,
                                                                     self.n_ranking,
                                                                     0,
                                                                     self.n_ranking )
                self.process_hists_dict[ proc ][ "mchist" ].SetFillColor( proc_dict[ "color" ] )
                self.process_hists_dict[ proc ][ "legend_name" ] = proc_dict[ "legend_name" ]
                self.process_hists_dict[ proc ][ "name" ] = proc_dict[ "name" ]
                self.process_hists_dict[ proc ][ "total_integral" ] = 0.
            self.process_hists_dict[ proc ][ "mchist" ].SetBinContent( i, proc_dict[ "integral" ] )
            self.process_hists_dict[ proc ][ "total_integral" ] += proc_dict[ "integral" ]


    def add_class_data( self, i, class_dict ):
        if self.data_hist is None:
            self.data_hist = ro.TH1F( "data",
                                      "data",
                                      self.n_ranking,
                                      0,
                                      self.n_ranking )
            self.data_hist.UseCurrentStyle()
            self.data_hist.SetBinErrorOption( ro.TH1.kPoisson )
            self.data_hist.GetXaxis().SetLabelFont( 62 )
            self.data_hist.GetYaxis().SetTitle( "Events per Class" )
        if self.config.show_class_names:
            self.data_hist.GetXaxis().SetBinLabel( i, class_dict[ "label" ] )
        else:
            self.data_hist.GetXaxis().SetBinLabel( i, "" )
        self.data_hist.GetXaxis().SetLabelFont( 42 )
        self.data_hist.SetBinContent( i, class_dict[ "data_integral" ] )
        if self.config.ymin:
            self.data_hist.SetMinimum( self.config.ymin )
        if self.config.ymax:
            self.data_hist.SetMaximum( self.config.ymax )
        self.max_y = max(self.max_y, self.data_hist.GetMaximum())

    def add_class_syst( self, i, class_dict ):
        if self.syst_hist is None:
            self.syst_hist = ro.TH1F( "syst",
                                      "syst",
                                      self.n_ranking,
                                      0,
                                      self.n_ranking )
            self.syst_hist.SetFillStyle( 3244 )
            self.syst_hist.SetFillColor( 35 )
            self.syst_hist.SetMarkerStyle( 0 )
        self.syst_hist.SetBinContent( i, class_dict[ "integral" ] )
        self.syst_hist.SetBinError( i, class_dict[ "total_syst_integral" ] )
        self.max_y = max(self.max_y, self.syst_hist.GetMaximum())

    def initalize_legend( self ):
        self.legend = ro.TLegend( 0.2, 0, 1, 1, "", "nbNDC")
        self.legend.SetFillStyle( 0 )
        self.legend.SetTextAngle( 90 )
        self.legend.SetTextAlign( 22 )
        self.legend.SetTextSize( self.config.legend_text_size )
        self.legend.SetNColumns( self.config.legend_number_columns )

    def create_mc_stack( self ):
        process_hists_list = self.process_hists_dict.values()
        # sort list of process hists by total integral
        process_hists_list = sorted( process_hists_list, key=lambda k: k[ "total_integral" ] )

        # create and fill stack
        self.total_stack = ro.THStack( "simpsons", "simpsons" )
        self.total_mc_hist = process_hists_list[0][ "mchist" ].Clone()
        self.total_mc_hist.Reset()
        for proc_dict in process_hists_list:
            self.total_stack.Add( proc_dict[ "mchist" ] )
            self.total_mc_hist.Add(proc_dict[ "mchist" ])
        for proc_dict in reversed( process_hists_list ):
            #self.legend_dict[ proc_dict[ "mchist" ].Integral() ] = [ proc_dict[ "mchist" ].GetName(), proc_dict[ "legend_name" ] ]
            #~ self.legend_dict[  ] = [ proc_dict[ "mchist" ], proc_dict[ "legend_name" ] ]
            self.legend_entries.append( ( proc_dict[ "mchist" ].Integral(), proc_dict[ "mchist" ], proc_dict[ "legend_name" ] ) )
        self.max_y = max(self.max_y, self.total_mc_hist.GetMaximum())

    def create_p_axis( self, max_y ):
        for i, class_dict in enumerate( self.ec_records ):
            self.p_axis.append( ro.TLatex( self.data_hist.GetBinCenter( i + 1 ),
                                           max_y * 1.5,
                                           "%.2g" % class_dict[ "p-value" ]
                                           )
                              )
            self.p_axis[ i ].SetTextAngle( 90 )
            self.p_axis[ i ].SetTextAlign( 12 )
            self.p_axis[ i ].SetTextSize( 0.04 )
            self.p_axis[ i ].SetNDC( 0 )

    def save_csv(self):
        out = os.path.join(self.config.out,
                               "integral_list" + "_" + \
                               self.class_type + "_" + \
                               self.config.sort_option + \
                               "_n_" + str(self.n_ranking) +".csv")
        with open(out, "w") as outfile:
            csv_writer = csv.writer(outfile)
            header_row = ["name", "integral", "data_integral", "syst_integral", "p-value"]
            csv_writer.writerow(header_row)
            for class_dict in self.ec_records:
                csv_writer.writerow([ class_dict[k] for k in header_row])

    @property
    def canvas( self ):
        if self._canvas is None:
            if self.config.show_class_names:
                height = self.config.canvas_height * 1.16
            self._canvas = ro.TCanvas( "simpsons_plot" + self.class_type, "",
                                      self.config.canvas_width,
                                      int(height) )
            self._canvas.UseCurrentStyle()
            self._canvas.SetLogy()
            if self.config.show_class_names:
                self._canvas.SetBottomMargin( 0.37 )
            else:
                self._canvas.SetBottomMargin( 0.05 )

            if self.config.show_p_value:
                self._canvas.SetTopMargin( 0.15 )
            else:
                self._canvas.SetTopMargin( 0.05 )
        return self._canvas

    def create_legend( self, include_data=True ):
        # Get Number of entries in legend. Dict contains only MC
        n_entries = len( self.legend_entries )
        if include_data:
            n_entries += 1

        # Set start values
        self.xup_legend = self.config.legend_xup
        self.yup_legend = self.config.legend_yup + 0.04 - ro.gPad.GetTopMargin()
        self.xlow_legend = self.xup_legend - self.config.legend_column_width
        self.ylow_legend = self.yup_legend - self.config.legend_text_size * n_entries * 2

        n_columns = self.config.legend_number_columns
        relative_differences = []
        max_hist_ndcs = []
        legend_xlows = [ self.xlow_legend ]
        legend_ylows = [ self.ylow_legend ]
        do_not_change_ymax = False

        # Start with 1 column and generate more if necessary
        if not self.config.legend_size_fixed:
            for i in range( self.config.legend_number_columns - 1 ):
                # if no collision we have enough columns
                relative_difference, max_hist_ndc = self.legend_collision()
                if relative_difference < 0:
                    do_not_change_ymax = True
                    break
                # Increase size in x
                else:
                    relative_differences.append( relative_difference )
                    max_hist_ndcs.append( max_hist_ndc )
                    self.xlow_legend -= self.config.legend_column_width
                    n_columns += 1

                    # Calculate number of entries per columns
                    n_entries_per_column = n_entries / n_columns
                    if n_entries % n_columns != 0:
                        n_entries_per_column += 1
                    self.ylow_legend = self.yup_legend - self.config.legend_text_size * n_entries_per_column
                    legend_xlows.append( self.xlow_legend )
                    legend_ylows.append( self.ylow_legend )

        # Get y coordinates of canvas
        xlow_canvas = ro.Double( 0. )
        ylow_canvas = ro.Double( 0. )
        xup_canvas = ro.Double( 0. )
        yup_canvas = ro.Double( 0. )
        self.canvas.Modified()
        self.canvas.Update()
        self.canvas.GetRangeAxis( xlow_canvas, ylow_canvas, xup_canvas, yup_canvas )

        # Add difference for last n_columns (not calculated before)
        relative_difference, max_hist_ndc = self.legend_collision()
        # Adjust ymax
        if not do_not_change_ymax and relative_difference > 0:
            relative_differences.append( relative_difference )
            max_hist_ndcs.append( max_hist_ndc )
            min_diff = min( relative_differences )
            index = relative_differences.index( min_diff )
            self.xlow_legend = legend_xlows[ index ]
            self.ylow_legend = legend_ylows[ index ]
            n_columns = index + 1

        # Create legend
        self.legend = ro.TLegend( self.xlow_legend, self.ylow_legend, self.xup_legend, self.yup_legend , "", "nbNDC")
        self.legend.SetFillStyle( 0 )
        # 0.8 chosen by eye to be a good value
        self.legend.SetTextSize( self.config.legend_text_size * 0.8 )
        self.legend.SetNColumns( n_columns )

        # Go to correct pad
        self.canvas.cd()

        tmp_legend_items_ordered = []

        # Sort by integral descending (=reverse)
        for integral, histname, label in sorted( self.legend_entries, key=lambda t: t[0], reverse=True ):
            tmp_legend_items_ordered.append( ( histname, label ) )

        # Add data entry in legend
        if include_data:
            # Plot as errorbars, label includes the event count.
            name = "Data"
            if self.config.legend_include_yield:
                name += " (%s)" % exponent_notation( self.data_hist.Integral() )

            self.legend.AddEntry( self.data_hist, name, "ELP" )

            tmp_legend_items_ordered.insert( 0, None )

        n_entries_mc = len( tmp_legend_items_ordered )
        for n_row in range( int( math.ceil( float( n_entries_mc ) / n_columns ) ) ):
            i = n_row
            while i < n_entries_mc:
                if tmp_legend_items_ordered[ i ]:
                    self.legend.AddEntry( tmp_legend_items_ordered[ i ][ 0 ], tmp_legend_items_ordered[ i ][ 1 ], "F" )
                i += int( math.ceil( float( n_entries_mc ) / n_columns ) )
        difference, maximum = ecroot.generic_collision( self.canvas, self.legend, [ self.total_mc_hist, self.data_hist, self.syst_hist ] )
        if difference > 0:
            self.max_y = max(self.max_y, ecroot.calculate_new_canvas_size( self.canvas, self.legend.GetY1(), maximum))

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
        n_points = self.syst_hist.GetNbinsX()
        x_values = np.zeros( n_points )
        x_errors = np.zeros( n_points )
        for ibin in range( n_points ):
            x_values[ ibin ] = self.syst_hist.GetBinCenter( ibin + 1 )
            x_errors[ ibin ] = self.syst_hist.GetBinWidth( ibin + 1 )
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
            y_mc = self.syst_hist.GetBinContent( ibin + 1 )
            y_data = self.data_hist.GetBinContent( ibin + 1 )
            y_maximum = max( y_maximum, y_mc, y_data )

        # Calculate maximum in NDC
        is_log = self.canvas.GetLogy()

        if is_log:
            ymax_relative_position = ( np.log10( y_maximum ) - ylow_canvas ) / ( yup_canvas - ylow_canvas )
        else:
            ymax_relative_position = ( y_maximum - ylow_canvas ) / ( yup_canvas - ylow_canvas )

        # Return if it collides
        return ( ymax_relative_position - self.ylow_legend, y_maximum )
        #if ymax_relative_position > self.ylow_legend:
        #    return True
        #else:
        #    return False

    def adjust_y_axes( self ):
        # determine max y from objects
        max_y = self.max_y
        for element in self.header_elements:
            box = convert_tlatex_to_tbox( self.canvas, element )
            difference, maximum = ecroot.generic_collision( self.canvas, box,
                [ self.total_mc_hist, self.syst_hist, self.data_hist ] )
            if difference > 0:
                max_y = ecroot.calculate_new_canvas_size( self.canvas,
                                                          box.GetY1(),
                                                          maximum )
                if max_y > self.max_y:
                    self.max_y = max_y

        box = ro.TBox( self.xlow_legend, self.ylow_legend, self.xup_legend, self.yup_legend )
        difference, maximum = ecroot.generic_collision( self.canvas, box,
                [ self.total_mc_hist, self.syst_hist, self.data_hist ] )
        if difference > 0:
            max_y = ecroot.calculate_new_canvas_size( self.canvas,
                                                          box.GetY1(),
                                                          maximum )
            self.max_y = max(self.max_y, max_y)

        self.data_hist.SetMaximum( self.max_y * 1000.)
        if self.data_hist.GetMinimum() > 1:
            self.data_hist.SetMinimum( 1 )

    def plot(self):
        self.canvas.cd()
        ro.gStyle.SetLineWidth( 2 )
        self.create_legend()

        self.adjust_y_axes()

        self.data_hist.SetLineWidth( 1 )
        self.data_hist.Draw( "pe" )
        self.data_hist.LabelsOption( 'v', 'x' )
        self.total_stack.Draw( "same hist" )
        self.syst_hist.Draw( "same e2" )
        self.data_hist.Draw( "pe same" )
        self.data_hist.Draw( "axis same" )

        self.canvas.RedrawAxis()

        # Get size of canvas
        xlow_canvas = ro.Double( 0. )
        ylow_canvas = ro.Double( 0. )
        xup_canvas = ro.Double( 0. )
        yup_canvas = ro.Double( 0. )
        if hasattr( self, 'pad_plot' ):
            self.pad_plot.Modified()
            self.pad_plot.Update()
            self.pad_plot.GetRangeAxis( xlow_canvas, ylow_canvas, xup_canvas, yup_canvas )
        else:
            self.canvas.Modified()
            self.canvas.Update()
            self.canvas.GetRangeAxis( xlow_canvas, ylow_canvas, xup_canvas, yup_canvas )

        if self.config.show_p_value:
            # Get x coordinates and errors and find energy at intersection between legend and axis
            self.create_p_axis( pow( 10, yup_canvas ) )
            for text in self.p_axis:
                text.Draw()

            p_value_axis_title = ro.TLatex( -0.5, pow( 10, yup_canvas) * 1.5, "p" )
            p_value_axis_title.Draw()


        self.legend.Draw()
        self.canvas.cd()
        # Draw the header elements
        for element in self.header_elements:
            element.Draw()

    def show( self ):
        print( "Most significant Classes" )
        print( "Distribution:", self.config.distribution )
        head = " {:>40s} | {:>9s} | {:<9s}".format( "Event Class", "p value", "p-tilde value" )
        print( head )
        print( "="*( len( head ) ) )
        for class_dict in self.ec_records:
            print( " {:>40s} | {:>1.8f} | {:<1.8f}".format( class_dict[ "name" ],
                                                            class_dict[ "p" ],
                                                            class_dict[ "p-tilde" ],
                                                           ) )

    @classmethod
    def add_commandline_options( cls, parser, with_parent=True ):
        super(cls, SimpsonsPlot).add_commandline_options(parser, with_parent=with_parent)
        group = parser.add_argument_group( title="Simpsons plotting options" )


        group.add_argument( "--distribution", default="SumPt",
                            choices=[d.name for d in ecroot.distribution_types()],
                            help="Distribution to plot choices:%(choices)s" )
        group.add_argument( "--sort-option", default="p-value", choices =[ "p-value", "data_integral", "objects" ],
                            help="Sorting option for the order of classes" )
        group.add_argument( "--n-ranking", default=10, type=int,
                            help="Number of classes to show in plot" )
        group.add_argument( "--show-class-names" ,help="Show class names in x-axis", default=True, type=bool )
        group = parser.add_argument_group( title="Legend plotting options" )
        group.add_argument( "--create-legend", help="Create legend.", default=True, type=bool )


class SimpsonsRatioPlot(SimpsonsBasePlot):
    def __init__( self, ec_records, class_type, config=None, object_group_tag=None):
        super( SimpsonsRatioPlot, self ).__init__( ec_records,
                                      class_type,
                                      config=config,
                                      object_group_tag=object_group_tag)
        self._ratio_hist = None
        self._ratio_syst_hist = None

        self.ratio_x = array.array( 'f' )
        self.ratio_y = array.array( 'f' )
        self.ratio_x_err = array.array( 'f' )
        self.ratio_y_err = array.array( 'f' )
        for i,record in enumerate(ec_records):
            self.add_class_mc(i, record)

    @classmethod
    def get_plot_name(cls, config, class_type='class_type', object_group_tag=""):
        basename = super(SimpsonsRatioPlot, cls).get_plot_name(config,
                                                               class_type=class_type,
                                                               object_group_tag=object_group_tag)
        return basename +  "_ratio"

    @property
    def ratio_hist(self):
        if not self._ratio_hist:
            self._ratio_hist = ro.TH1F( "ratio_hist",
                     "ratio_hist",
                     self.n_ranking,
                     0,
                     self.n_ranking )
        return self._ratio_hist

    @property
    def ratio_syst_hist(self):
        if not self._ratio_syst_hist:
            self._ratio_syst_hist = ro.TH1F( "syst",
                                      "syst",
                                      self.n_ranking,
                                      0,
                                      self.n_ranking )
            self._ratio_syst_hist.SetFillStyle( 3244 )
            self._ratio_syst_hist.SetFillColor( 35 )
            self._ratio_syst_hist.SetMarkerStyle( 0 )
        return self._ratio_syst_hist

    def add_class_mc( self, i, class_dict ):
        ratio = class_dict["data_integral"] / class_dict["integral"]
        ratio_syst = class_dict["integral"] / class_dict["total_syst_integral"]
        self.ratio_hist.SetBinContent(i, ratio)
        self.ratio_hist.SetBinError(i, sqrt(class_dict["integral"]) / class_dict["integral"] )

        self.ratio_syst_hist.SetBinContent(i, ratio)
        self.ratio_syst_hist.SetBinError(i, ratio_syst)

    def plot(self):
        ro.gStyle.SetLineWidth(2)
        self.header_elements = ecroot.create_tex_header(self.canvas,
                                              header_title=self.config.header_title,
                                              header_subtitle=self.config.header_subtitle,
                                              text_size_header=0.045,
                                              text_size_info=0.03,
                                              header_outside=False,
                                              lumi=self.lumi,
                                              cme=self.cme,
                                              dataset_info_outside=False,
                                              extra_info_text="%s classes" % self.class_type,
                                            )

        self.adjust_y_axes()
        self.ratio_hist.SetLineWidth(1)
        self.ratio_hist.Draw("pe")

        self.ratio_syst_hist.Draw("same e2")

