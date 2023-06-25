#ifndef SHIFTS
#define SHIFTS
#include <string>
#include <vector>

#include <fmt/format.h>

class Shifts
{
  private:
    const std::vector<std::string> m_shifts;

  public:
    Shifts(bool is_data)
        : m_shifts(is_data ? std::vector<std::string>{"Nominal"}
                           : std::vector<std::string>{"Nominal",                 //
                                                      "PU_Up",                   //
                                                      "PU_Down",                 //
                                                      "Luminosity_Up",           //
                                                      "Luminosity_Down",         //
                                                      "xSecOrder_Up",            //
                                                      "xSecOrder_Down",          //
                                                      "Fake_Up",                 //
                                                      "Fake_Down",               //
                                                      "ScaleFactor_Up",          //
                                                      "ScaleFactor_Down",        //
                                                      "PreFiring_Up",            //
                                                      "PreFiring_Down",          //
                                                      "MuonResolution_Up",       //
                                                      "MuonResolution_Down",     //
                                                      "MuonScale_Up",            //
                                                      "MuonScale_Down",          //
                                                      "ElectronResolution_Up",   //
                                                      "ElectronResolution_Down", //
                                                      "ElectronScale_Up",        //
                                                      "ElectronScale_Down",      //
                                                      "PhotonResolution_Up",     //
                                                      "PhotonResolution_Down",   //
                                                      "PhotonScale_Up",          //
                                                      "PhotonScale_Down",        //
                                                      "JetResolution_Up",        //
                                                      "JetResolution_Down",      //
                                                      "JetScale_Up",             //
                                                      "JetScale_Down"})
    {
    }

    // Iterator types
    using const_iterator = std::vector<std::string>::const_iterator;

    // Member functions for iterators
    const_iterator begin() const
    {
        return m_shifts.cbegin();
    }

    const_iterator end() const
    {
        return m_shifts.cend();
    }

    const_iterator cbegin() const
    {
        return m_shifts.cbegin();
    }

    const_iterator cend() const
    {
        return m_shifts.cend();
    }

    auto get_shifts() const -> std::vector<std::string>
    {
        return m_shifts;
    }

    auto size() const -> std::size_t
    {
        return m_shifts.size();
    }

    static auto get_pu_variation(const std::string &shift) -> std::string
    {
        if (shift == "PU_Up")
        {
            return "up";
        }

        if (shift == "PU_Down")
        {
            return "down";
        }

        return "nominal";
    }

    static auto get_prefiring_weight(float L1PreFiringWeight_Nom,
                                     float L1PreFiringWeight_Up,
                                     float L1PreFiringWeight_Dn,
                                     const std::string &shift) -> float
    {
        if (shift == "PreFiring_Up")
        {
            return L1PreFiringWeight_Up;
        }
        if (shift == "PreFiring_Down")
        {
            return L1PreFiringWeight_Dn;
        }
        return L1PreFiringWeight_Nom;
    }

    static auto scale_luminosity(const double luminosity, const std::string &shift) -> float
    {
        if (shift == "Luminosity_Up")
        {
            return luminosity * (1 + 2.5 / 100.);
        }
        if (shift == "Luminosity_Down")
        {
            return luminosity * (1 - 2.5 / 100.);
        }

        return 1.;
    }
};

#endif /*SHIFTS*/
