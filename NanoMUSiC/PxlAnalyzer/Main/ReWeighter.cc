#include "ReWeighter.hh"

#include <string>

#include "Tools/MConfig.hh"
#include "Tools/Tools.hh"

ReWeighter::ReWeighter(const Tools::MConfig &cutconfig, int i)
    : m_year(cutconfig.GetItem<std::string>("year")),
      m_json_pu_file(Tools::AbsolutePath(cutconfig.GetItem<std::string>("Pileup.JSONFile"))),
      m_useGenWeights(cutconfig.GetItem<bool>("General.UseGeneratorWeights")),
      m_useREcoVertices(cutconfig.GetItem<bool>("Pileup.UseRecoVertices")),
      m_usePileUpReWeighting(cutconfig.GetItem<bool>("Pileup.UsePileupReWeighting")),
      m_quietMode(cutconfig.GetItem<bool>("Pileup.QuietMode", false)), m_init(false)
{
    m_syst = 0;

    if (i >= 1)
    {
        m_syst = 1;
    }
    if (i <= -1)
    {
        m_syst = -1;
    }
}

void ReWeighter::init()
{
    // if true Pileup weights will not be dumped to stdout when read
    if (m_quietMode)
        std::cout.setstate(std::ios_base::failbit);
    // m_pu_correction_set = edm::LumiReWeighting(mcname,
    //                                      dataname,
    //                                      mchistname,
    //                                      datahistname);
    m_pu_correction_set = correction::CorrectionSet::from_file(m_json_pu_file);

    std::map<std::string, std::string> pu_key = {
        {"2016APV", "Collisions16_UltraLegacy_goldenJSON"},
        {"2016", "Collisions16_UltraLegacy_goldenJSON"},
        {"2017", "Collisions17_UltraLegacy_goldenJSON"},
        {"2018", "Collisions18_UltraLegacy_goldenJSON"},
    };
    m_pu_correction = m_pu_correction_set->at(pu_key[m_year]);
    // TFile *temp_mc_file = new TFile(mcname.c_str(), "READ");
    // TH1F *temp_mc_hist = (TH1F *)temp_mc_file->Get(mchistname.c_str());
    // m_max_gen_vertices = temp_mc_hist->GetXaxis()->GetBinUpEdge(temp_mc_hist->FindLastBinAbove(0));
    // if (m_quietMode)
    //    std::cout.clear();
    // delete temp_mc_hist;
    // temp_mc_file->Close();
    // delete temp_mc_file;
    m_init = true;
}

void ReWeighter::ReWeightEvent(pxl::Event *event)
{
    if (not m_usePileUpReWeighting)
        return;
    if (not m_init)
        init();
    pxl::EventView *GenEvtView = event->getObjectOwner().findObject<pxl::EventView>("Gen");
    pxl::EventView *RecEvtView = event->getObjectOwner().findObject<pxl::EventView>("Rec");

    // Disable generator weights.
    if (not m_useGenWeights)
        GenEvtView->setUserRecord("genWeight", 1.0);

    if (m_usePileUpReWeighting)
    {
        float numVerticesPUTrue = GenEvtView->getUserRecord("Pileup_nTrueInt");
        if (m_useREcoVertices)
        {
            numVerticesPUTrue = RecEvtView->getUserRecord("NumVertices");
        }

        if (numVerticesPUTrue > m_max_gen_vertices)
        {
            std::cerr << "(WARNING): found event with " << numVerticesPUTrue << " but maximum vertices in gen dist is "
                      << m_max_gen_vertices << std::endl
                      << "Using max val from gen dist" << std::endl;
            // Take last bin (m_max_gen_vertices) is upper bin edge
            numVerticesPUTrue = m_max_gen_vertices - 0.1;
        }

        if (m_syst == 1)
        {
            GenEvtView->setUserRecord("PUWeightUp", m_pu_correction->evaluate({numVerticesPUTrue, "up"}));
        }
        if (m_syst == -1)
        {
            GenEvtView->setUserRecord("PUWeightDown", m_pu_correction->evaluate({numVerticesPUTrue, "down"}));
        }
        if (m_syst == 0)
        {
            GenEvtView->setUserRecord("PUWeight", m_pu_correction->evaluate({numVerticesPUTrue, "nominal"}));
        }
    }
    else
    {
        if (m_syst == 1)
        {
            GenEvtView->setUserRecord("PUWeightUp", 1.);
        }
        if (m_syst == -1)
        {
            GenEvtView->setUserRecord("PUWeightDown", 1.);
        }
        if (m_syst == 0)
        {
            GenEvtView->setUserRecord("PUWeight", 1.);
        } // GenEvtView->setUserRecord( "PUWeight", 1. );
    }
}

void ReWeighter::adaptConfig(Tools::MConfig &config, const std::string &filename)
{
    const auto start = filename.rfind('/') + 1;
    const auto end = filename.rfind('_');
    const auto sample_name = filename.substr(start, end - start);
    config.AddItem("Pileup.GenHistName", sample_name);
    config.AddItem("Pileup.GenHistNameUp", sample_name);
    config.AddItem("Pileup.GenHistNameDown", sample_name);
}
