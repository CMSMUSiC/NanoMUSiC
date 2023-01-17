from table2latex.textable import TexTableConfig
from table2latex.textable import escape_latex
import rounding

import re

config = TexTableConfig()

# Basic configuration
# column key used to sort table / group
#config.sortkey = None
#
# key used to group rows
#config.groupkey=None
#
# Flag to control if grouping column should be visible in the table
#config.hide_group = False
#
# latex style used for table object
#config.tablestyle = "table"
#
# Number of entries before the table is splitted in subtables
config.chunksize = 999
#
# Flag for latex landscape mode
#config.landscape = False

# Basic configuration
config.add_package("longtable")
config.tablestyle = "longtable"
config.use_document = False
# column key used to sort table / group
#~ config.sortkey = "sa_crosssection"
config.sortkey = "sa_name"
#
# Flag to control if grouping column should be visible in the table
config.hide_group = False
# Number of entries before the table is splitted in subtables
#~ config.chunksize =
#
# Flag for latex landscape mode
config.landscape = True

# Use either a single column or a callback function to create grouping
def process_group(texrow):
     import processgroup
     return processgroup.get_process_group(texrow.sa_name)

# callback function used to construct grouping key. Callback expect a TexRow as input
config.group_func = process_group

# key used to group rows, the key here needs to be set even if a group_func is given
# In this case the key is only used to identify the group column e.g. for replacements.
config.groupkey="sa_process_group"

# Define columns using funcitons
def short_datasetpath(texrow):
     split = texrow.sk_datasetpath.split("/")
     dp = split[1]
     if re.findall("_ext\d", texrow.sk_datasetpath):
          dp += re.findall("_ext\d", texrow.sk_datasetpath)[0]
     return escape_latex(dp)

config.add_column_func(short_datasetpath, 'sk_datasetpath')

def combined_xs(texrow):
     return "%s (%s)" % (texrow.rounding.latex( texrow.sa_crosssection), texrow.sa_crosssection_order)

config.add_column_func(combined_xs, 'sa_crosssection')

def combined_kfactor(texrow):
     if texrow.sa_kfactor == 1.:
          return "-"
     if texrow.sa_kfactor_order:
          return "%s (%s)" % (texrow.rounding.latex( texrow.sa_kfactor), texrow.sa_kfactor_order)
     return texrow.rounding.latex( texrow.sa_kfactor)

config.add_column_func(combined_kfactor, 'sa_kfactor')

def total_weight(texrow):
     w = 35922.0 * texrow.sa_kfactor * texrow.sa_filterefficiency * texrow.sa_kfactor / texrow.sk_nevents
     return texrow.rounding.latex(w)

config.add_column_func(total_weight, 'sa_total_weight')
# define grouping order in table
grouporder = [ 'DrellYan',
               'ZToInvisible',
               'W',
               'Gamma',
               'DiBoson',
               'DiPhoton',
               'TTbar',
               'TTG',
               'TTW',
               'TTZ',
               'TTbarTTbar',
             ]

config.add_group_order(grouporder)
# add a list of column keys with all keys included in the table
chosen_column_keys = [   #'sa_name',
                        'sk_datasetpath',
                    'sa_crosssection',
                    'sa_filterefficiency',
                    'sa_kfactor',
                    'sk_nevents',
                    'sa_total_weight'
                    ]
config.add_column_keys(chosen_column_keys)


# add custom row width
attribute_width_map =  {
                            'sa_process_group'      :  2.2,
                            'sk_datasetpath':  11.5,
                            'sa_crosssection'         :  2.6,
                            'sa_filterefficiency'  :  1.5,
                            'sa_kfactor'    :  2.0,
                            'sk_nevents'    :  1.5,
                            'da_total_weights'    :  1.5,
                            }
config.add_column_widths(attribute_width_map)

# Header replacements for different lines
header_first_line_map = {
                            'sa_process_group'      :  '\\textbf{Processgroup}',
                            'sk_datasetpath':  '\\textbf{Datasetpath}',
                            'sa_crosssection'         :  '\\textbf{X-Section}   ',
                            'sa_filterefficiency'  :  '\\textbf{Filter}      ',
                            'sa_kfactor'    :  '\\textbf{k-Factor}    ',
                            'sk_nevents'    :  '\\textbf{Number}      ',
                            'sa_total_weight' :  '\\textbf{Weight}   ',
                        }
config.add_header_line(header_first_line_map)

header_second_line_map = {
                            'sa_crosssection'         :  '\\textbf{in pb}      ',
                            'sa_filterefficiency'  :  '\\textbf{Efficiency} ',
                            'sa_kfactor'    :  '\\textbf{(Order)}    ',
                            'sk_nevents'    :  '\\textbf{of Events}  ',
                            'sa_total_weight' :  '\\textbf{$\\alpha_{MC}$}   ',
                        }
config.add_header_line(header_second_line_map)

