#ifndef ScaleFactor_hh
#define ScaleFactor_hh

#include "TFile.h"
#include "TH2F.h"
#include "TROOT.h"
#include <string>
#include <vector>

#include "Pxl/Pxl/interface/pxl/core.hpp"
#include "Pxl/Pxl/interface/pxl/hep.hpp"

#include "MConfig.hpp"

class ScaleFactor
{
  public:
    ScaleFactor(const std::string &particleType, const std::string &scale_factor_name, const Tools::MConfig &cfg);
    ScaleFactor(const ScaleFactor &COPY);
    ScaleFactor &operator=(const ScaleFactor &rhs);

    ~ScaleFactor();
    double getScaleFactor(const pxl::Particle *object) const;
    double getScaleFactorError(const pxl::Particle *object) const;
    std::string getType()
    {
        return m_type;
    };
    std::string getName()
    {
        return m_name;
    };
    double getSystematic() const;

  private:
    // Member functions
    void copyScaleFactorFields(const ScaleFactor &sf);
    double getPtEtaScaleFactor(const pxl::Particle *object) const;
    double getEtaPtScaleFactor(const pxl::Particle *object) const;
    double getPtEtaScaleFactorError(const pxl::Particle *object) const;
    double getEtaPtScaleFactorError(const pxl::Particle *object) const;
    bool inrange(const double x) const;
    bool inYrange(const double y) const;
    // Member variables
    std::string m_name;
    std::string m_type;
    std::string m_scale_factor_file_name;
    std::string m_scale_factor_directory_name;
    std::string m_scale_factor_histogram_name;
    bool m_abs_eta;
    double m_systematic;
    double m_min_x;
    double m_min_y;
    double m_max_x;
    double m_max_y;

    TH2F *m_scale_factor_hist;
};
#endif
