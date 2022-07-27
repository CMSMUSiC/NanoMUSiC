import ROOT as ro

from .root import ECRootHelper

class ECTestHelper(ECRootHelper):

    @classmethod
    def init_ec(cls,
                countDict,
                distTypeBinDict,
                distTypeMinDict,
                distTypeGlobalMinDict,
                sys_names = None,
                useCharge = False,
                isData = False,
                useBjets = True,
                isInclusive = True,
                ec_type = "Rec_",
                ec_name = "Empty+X",
                ec_runhash = "testHash",
                ec_cme = 13000.,
                lumi = 35922.0,
                total_charge = 0,
                numPDFs = 4,
                ):
        ''' Covert python objects to C types and init a new ec '''
        countMap = ro.std.map('string,int')()
        for obj_name, count in countDict.items():
            countMap[obj_name] = count

        distTypeBins = ro.std.map(str, 'vector<double>')()
        for distribution, binlist in distTypeBinDict.items():
            bins = ro.std.vector('double')()
            for x in binlist:
                bins.push_back(1. * x)
            distTypeBins[distribution] = bins

        distTypeMins = ro.std.map('string,double')()
        for distribution, distribution_min in distTypeMinDict.items():
            distTypeMins[distribution] = distribution_min

        distTypeGlobalMins = ro.std.map('string,double')()
        for distribution, distribution_min in distTypeGlobalMinDict.items():
            distTypeGlobalMins[distribution] = distribution_min

        systNames = ro.std.set(str)()
        if sys_names:
            for sys in sys_names:
                systNames.insert(sys)

        return ro.TEventClass( ec_type,
                               ec_name,
                               ec_runhash,
                               isData,
                               ec_cme,
                               countMap,
                               useBjets,
                               distTypeBins,
                               useCharge,
                               total_charge,
                               isInclusive,
                               distTypeMins,
                               numPDFs,
                               distTypeGlobalMins,
                               lumi,
                               systNames
                               );

    @classmethod
    def fill_ec(cls,
                ec,
                process,
                dist_type_value_dict,
                dist_type_resolution_dict,
                weight=1.,
                sys_weights=None,
                pdf_weights=None):
        ''' Covert python objects to C types and fill ec '''
        distTypeValues = ro.std.map('string,double')()
        for distribution, value in dist_type_value_dict.items():
            distTypeValues[distribution] = value
        #~ for pair in distTypeValues:
            #~ if pair.second < 0:
                #~ print pair.second

        distTypeResolutions = ro.std.map('string,pair<double,double>')()
        for distribution, values in dist_type_resolution_dict.items():
            distTypeResolutions[distribution] = ro.std.pair('double','double')(values[0],values[1])

        sysWeights = ro.std.map('string,double')()
        if sys_weights is not None:
            for systematic, value in sys_weights.items():
                sysWeights[systematic] = value

        pdfWeightsVector = ro.std.vector('float')()
        if pdf_weights is not None:
            for pdf_weight in pdf_weights:
                pdfWeightsVector.push_back(pdf_weight)
        #~ print "Fill"
        ec.Fill(process,
                distTypeValues,
                distTypeResolutions,
                weight,
                sysWeights,
                pdfWeightsVector)

