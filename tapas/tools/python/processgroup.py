import fnmatch

process_group_patterns = {}

process_group_patterns['DrellYan'] = ['ZTo*', 'DrellYan*' , 'DY*']
process_group_patterns['ZToInvisible'] = ['ZJetsToNuNu*']
process_group_patterns['W'] = ['WTo*', 'WJets*']
process_group_patterns['Gamma'] = ['GJets*']

process_group_patterns['WZ'] = ['WZTo*']
process_group_patterns['WG'] = ['WGTo*', 'WGJets*']
process_group_patterns['ZG'] = ['ZGTo*']
process_group_patterns['ZZ'] = ['ZZTo*']
process_group_patterns['WW'] = ['WWTo*']
process_group_patterns['GG'] = ['GG_*', 'GGJets_*', 'DiPhotonJets*']

process_group_patterns['ZZZ'] = ['ZZZ_*']
process_group_patterns['WWW'] = ['WWW_*']
process_group_patterns['WWG'] = ['WWG_*']
process_group_patterns['WWZ'] = ['WWZ_*']
process_group_patterns['WGG'] = ['WGG_*']
process_group_patterns['WZG'] = ['WZG_*']
process_group_patterns['WZZ'] = ['WZZ_*']

process_group_patterns['TTbar'] = ['TT_*', 'TTTo*']
process_group_patterns['TTW'] = ['TTWJets*']
process_group_patterns['TTG'] = ['TTGJets*']
process_group_patterns['TTZ'] = ['TTZTo*']
process_group_patterns['TTGG'] = ['TTGG_0Jets*']

process_group_patterns['tG'] = ['TGJets*']
process_group_patterns['tZQ'] = ['tZq_*']
process_group_patterns['Top'] = ['ST_*-channel*', 'ST_tW_*']
process_group_patterns['TTbarTTbar'] = ['TTTT_*']

process_group_patterns['HIG'] = ['GluGluHTo*', 'VBFHTo*', 'VBF_HTo*', 'VHTo*', 'WplusH_HTo*', 'WminusH_HTo*', 'ZH_HTo*', 'ggZH_HTo*', 'ttHTo*']

process_group_patterns['QCD'] = ['QCD_*']

def get_process_group(sample_name):
    for pgroup, patterns in process_group_patterns.items():
        for pattern in patterns:
            if fnmatch.fnmatchcase(sample_name, pattern):
                return pgroup
    return ""
