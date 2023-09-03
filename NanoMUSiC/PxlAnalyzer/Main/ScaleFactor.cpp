#include "ScaleFactor.hpp"

ScaleFactor::ScaleFactor(const std::string &particleType,
                         const std::string &scale_factor_name,
                         const Tools::MConfig &cfg)
    : m_name(scale_factor_name),
      m_type(cfg.GetItem<std::string>(particleType + ".ScaleFactor." + scale_factor_name + ".type", "")),
      m_scale_factor_file_name(
          cfg.GetItem<std::string>(particleType + ".ScaleFactor." + scale_factor_name + ".file", "")),
      m_scale_factor_directory_name(
          cfg.GetItem<std::string>(particleType + ".ScaleFactor." + scale_factor_name + ".dir", "")),
      m_scale_factor_histogram_name(
          cfg.GetItem<std::string>(particleType + ".ScaleFactor." + scale_factor_name + ".hist", "")),
      m_abs_eta(cfg.GetItem<bool>(particleType + ".ScaleFactor." + scale_factor_name + ".abseta", false)),
      m_systematic(cfg.GetItem<double>(particleType + ".ScaleFactor." + scale_factor_name + ".syst", 0)),
      m_min_x(-1.),
      m_min_y(-1.),
      m_max_x(-1.),
      m_max_y(-1.)
{
    // check if all info for initilaization is available
    if (m_scale_factor_file_name.empty())
    {
        std::stringstream err;
        err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
        err << "No file name specified for scale factor " << particleType << " " << scale_factor_name << " " << m_type
            << std::endl;
        throw Tools::value_error(err.str());
    }
    if (m_scale_factor_histogram_name.empty())
    {
        std::stringstream err;
        err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
        err << "No histogram name specified for scale factor " << particleType << " " << m_type << std::endl;
        throw Tools::value_error(err.str());
    }

    TFile *fin_scale_factor = new TFile(m_scale_factor_file_name.c_str(), "OPEN");
    if (!fin_scale_factor->IsOpen())
    {
        throw Tools::file_not_found(m_scale_factor_file_name);
    }
    TDirectory *direc = 0;
    // check if we need to get hist from sub directory
    if (!m_scale_factor_directory_name.empty())
    {
        direc = (TDirectory *)fin_scale_factor->Get(m_scale_factor_directory_name.c_str());
        if (!direc)
        {
            std::stringstream err;
            err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
            err << particleType << "getting histogram " << m_scale_factor_histogram_name << "containing scale factors"
                << std::endl;
            err << "Could not find directory " << m_scale_factor_directory_name << " in file "
                << m_scale_factor_file_name << std::endl;
            throw Tools::value_error(err.str());
        }
        m_scale_factor_hist = ((TH2F *)direc->Get(m_scale_factor_histogram_name.c_str()));
    }
    else
    {
        m_scale_factor_hist = ((TH2F *)fin_scale_factor->Get(m_scale_factor_histogram_name.c_str()));
    }
    if (!m_scale_factor_hist)
    {
        std::stringstream err;
        err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
        err << "Could not find histogram " << m_scale_factor_histogram_name << " for scale factor  "
            << scale_factor_name << std::endl;
        throw Tools::value_error(err.str());
    }
    m_scale_factor_hist->SetDirectory(0);
    if (m_scale_factor_directory_name.empty())
        delete direc;
    delete fin_scale_factor;
    // get upper bin edge for highest filled bins
    TH1D *projX = m_scale_factor_hist->ProjectionX("dummy", 1, 1);
    int nx = projX->GetNbinsX();
    m_max_x = projX->GetBinLowEdge(nx) + projX->GetBinWidth(nx);
    m_min_x = projX->GetBinLowEdge(1);

    TH1D *projY = m_scale_factor_hist->ProjectionY("dummy", 1, 1);
    int ny = projY->GetNbinsX();
    m_max_y = projY->GetBinLowEdge(ny) + projY->GetBinWidth(ny);
    m_min_y = projY->GetBinLowEdge(1);

    delete projX;
    delete projY;
}

// we have a TH2F pointer ( dynamically allocated memory) and thus need
// a deep copy implementation of assign operator and copy constructor
ScaleFactor::ScaleFactor(const ScaleFactor &COPY)
{
    copyScaleFactorFields(COPY);
}

ScaleFactor &ScaleFactor::operator=(const ScaleFactor &rhs)
{
    if (&rhs == this)
        return *this;
    copyScaleFactorFields(rhs);
    return *this;
}

// common copy function to avoid dublicated code in assignment operator
// and copy constructor
void ScaleFactor::copyScaleFactorFields(const ScaleFactor &sf)
{
    m_name = sf.m_name;
    m_type = sf.m_type;
    m_scale_factor_file_name = sf.m_scale_factor_file_name;
    m_scale_factor_directory_name = sf.m_scale_factor_directory_name;
    m_scale_factor_histogram_name = sf.m_scale_factor_histogram_name;
    m_abs_eta = sf.m_abs_eta;
    m_systematic = sf.m_systematic;
    m_min_x = sf.m_min_x;
    m_min_y = sf.m_min_y;
    m_max_x = sf.m_max_x;
    m_max_y = sf.m_max_y;
    m_scale_factor_hist = new TH2F(*sf.m_scale_factor_hist);
}

ScaleFactor::~ScaleFactor()
{
    m_scale_factor_hist->Delete();
}

double ScaleFactor::getScaleFactor(const pxl::Particle *object) const
{
    if (m_type == "PtEta")
        return getPtEtaScaleFactor(object);
    else if (m_type == "EtaPt")
        return getEtaPtScaleFactor(object);
    std::cout << "WARNING: Unknown type " << m_type << " used for scale factor m_name" << std::endl;
    return 1.;
}

double ScaleFactor::getScaleFactorError(const pxl::Particle *object) const
{
    if (m_type == "PtEta")
        return getPtEtaScaleFactorError(object);
    else if (m_type == "EtaPt")
        return getEtaPtScaleFactorError(object);
    std::cout << "WARNING: Unknown type " << m_type << " used for scale factor m_name" << std::endl;
    return 0.;
}

bool ScaleFactor::inrange(const double x) const
{
    if (x < m_min_x or x > m_max_x)
        return false;
    return true;
}
bool ScaleFactor::inYrange(const double y) const
{
    if (y < m_min_y or y > m_max_y)
        return false;
    return true;
}

// Return scale factor binned in pt and eta.
// If pt is greater than the highest bin, the value for the highest bin is uses
// If eta is outside the available range 1. is returned
double ScaleFactor::getPtEtaScaleFactor(const pxl::Particle *object) const
{
    double eta = object->getEta();
    double pt = object->getPt();
    if (m_abs_eta)
    {
        eta = std::fabs(eta);
    }
    int bin_number;
    if (inrange(pt) and inYrange(eta))
    {
        bin_number = m_scale_factor_hist->FindBin(pt, eta);
        return m_scale_factor_hist->GetBinContent(bin_number);
    }
    else if (inYrange(eta) and pt > m_max_x)
    {
        bin_number = m_scale_factor_hist->FindBin(m_max_x - 0.00001, eta);
        return m_scale_factor_hist->GetBinContent(bin_number);
    }
    else
    {
        return 1.;
    }
    return 1.;
}

double ScaleFactor::getEtaPtScaleFactor(const pxl::Particle *object) const
{
    double eta = object->getEta();
    double pt = object->getPt();
    if (m_abs_eta)
    {
        eta = std::fabs(eta);
    }
    int bin_number;
    if (inrange(eta) and inYrange(pt))
    {
        bin_number = m_scale_factor_hist->FindBin(eta, pt);
        return m_scale_factor_hist->GetBinContent(bin_number);
    }
    else if (inrange(eta) and pt > m_max_y)
    {
        bin_number = m_scale_factor_hist->FindBin(eta, m_max_y - 0.00001);
        return m_scale_factor_hist->GetBinContent(bin_number);
    }
    else
    {
        return 1.;
    }
    return 1.;
}

double ScaleFactor::getPtEtaScaleFactorError(const pxl::Particle *object) const
{
    double eta = object->getEta();
    double pt = object->getPt();
    if (m_abs_eta)
    {
        eta = std::fabs(eta);
    }
    int bin_number;
    if (inrange(pt) and inYrange(eta))
    {
        bin_number = m_scale_factor_hist->FindBin(pt, eta);
        return m_scale_factor_hist->GetBinError(bin_number);
    }
    else if (inYrange(eta) and pt > m_max_x)
    {
        bin_number = m_scale_factor_hist->FindBin(m_max_x - 0.000001, eta);
        return m_scale_factor_hist->GetBinError(bin_number);
    }
    else
    {
        return 0.;
    }
    return 0.;
}

double ScaleFactor::getEtaPtScaleFactorError(const pxl::Particle *object) const
{
    double eta = object->getEta();
    double pt = object->getPt();
    if (m_abs_eta)
    {
        eta = std::fabs(eta);
    }
    int bin_number;
    if (inrange(eta) and inYrange(pt))
    {
        bin_number = m_scale_factor_hist->FindBin(eta, pt);
        return m_scale_factor_hist->GetBinError(bin_number);
    }
    else if (inrange(eta) and pt > m_max_y)
    {
        bin_number = m_scale_factor_hist->FindBin(eta, m_max_y - 0.000001);
        return m_scale_factor_hist->GetBinError(bin_number);
    }
    else
    {
        return 0.;
    }
    return 0.;
}

double ScaleFactor::getSystematic() const
{
    return m_systematic;
}
