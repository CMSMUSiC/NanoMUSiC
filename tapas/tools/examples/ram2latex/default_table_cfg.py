from table2latex.textable import TexTableConfig
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
#config.chunksize = 1e9
#
# Flag for latex landscape mode
#config.landscape = False

# Basic configuration
config.add_package("longtable")
config.tablestyle = "longtable"
config.use_document = False
# column key used to sort table / group
config.sortkey = "sa_crosssection"
#~ config.sortkey = "sa_name"
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

def short_datasetpath(texrow):
     split = texrow.sk_datasetpath.split("/")
     return split[1]


# mark columns to be outputed as the raw input (no escaping)
#config.add_raw_flag("my_columns_key")

config.add_column_func(short_datasetpath, 'sk_datasetpath')

# callback function used to construct grouping key. Callback expect a TexRow as input
config.group_func = process_group



# key used to group rows, the key here needs to be set even if a group_func is given
# In this case the key is only used to identify the group column e.g. for replacements.
config.groupkey="sa_process_group"


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
                    #~ 'sa_filterefficiency',
                    #~ 'sa_kfactor',
                    #~ 'sk_nevents',
                    #~ 'sk_skimmer_globaltag',
                    ]
config.add_column_keys(chosen_column_keys)


# add custom row width
attribute_width_map =  {
                            'sa_process_group'      :  2.5,
                            'sa_name'       :  8.5,
                            'sk_datasetpath':  20.5,
                            'sa_crosssection'         :  1.6,
                            'sa_filterefficiency'  :  1.5,
                            'sa_kfactor'    :  2.0,
                            'sk_nevents'    :  1.5,
                            'sk_skimmer_globaltag' :   3.,
                            }
config.add_column_widths(attribute_width_map)

# Header replacements for different lines
header_first_line_map = {
                            'sa_process_group'      :  '\\textbf{Processgroup}',
                            'sa_name'       :  '\\textbf{Dataset Name}',
                            'sk_datasetpath':  '\\textbf{Datasetpath}',
                            'sa_crosssection'         :  '\\textbf{X-Section}   ',
                            'sa_filterefficiency'  :  '\\textbf{Filter}      ',
                            'sa_kfactor'    :  '\\textbf{k-Factor}    ',
                            'sk_nevents'    :  '\\textbf{Number}      ',
                            'sk_skimmer_globaltag' :  '\\textbf{Conditions}   ',
                        }
config.add_header_line(header_first_line_map)

header_second_line_map = {
                            'sa_crosssection'         :  '\\textbf{in pb}      ',
                            'sa_filterefficiency'  :  '\\textbf{Efficiency} ',
                            'sa_kfactor'    :  '\\textbf{(Order)}    ',
                            'sk_nevents'    :  '\\textbf{of Events}  ',
                        }
config.add_header_line(header_second_line_map)

