#include "TauSelector.hh"
#include "Tools/PXL/Sort.hh"
#include "Tools/Tools.hh"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

//--------------------Constructor-----------------------------------------------------------------

TauSelector::TauSelector(const Tools::MConfig &cfg, OldNameMapper *globalOldNameMap)
    : // Add parent constructor
      ObjectSelector(cfg, globalOldNameMap, "Tau", false),
      // Taus:

      // Get Tau-Discriminators and save them
      m_tau_discriminators(Tools::splitString<std::string>(cfg.GetItem<std::string>("Tau.Discriminators"), true)),
      m_tau_syst_discriminators(
          Tools::splitString<std::string>(cfg.GetItem<std::string>("Tau.SystDiscriminators"), true))
{
}

//--------------------Destructor------------------------------------------------------------------

TauSelector::~TauSelector()
{
}

int TauSelector::passObjectSelection(pxl::Particle *tau, double const tauRho, const std::string &idType,
                                     const bool isSyst // use alternative kinematic cuts for syst
) const
{
    // pt cut
    // std::cout << " \n ****** START OBJECT SELECTION FOR 1 TAU************* " << std::endl;
    bool passKin = ObjectSelector::passKinematics(tau, isSyst);
    bool passID = true;

    tau->setUserRecord("usedID", idType);
    if (not isSyst)
    {
        for (std::vector<std::string>::const_iterator discr = m_tau_discriminators.begin();
             discr != m_tau_discriminators.end(); ++discr)
        {
            // In theory all tau discriminators have a value between 0 and 1.
            // Thus, they are saved as a float and the cut value is 0.5.
            // In practice most (or all) discriminators are boolean.
            // See also:
            // https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePFTauID#Discriminators
            std::cout << " \n ******** NO SYST ********** " << std::endl;
            // std::cout << "Tau Discriminator Name: " <<  *discr  << std::endl;
            // std::cout << "Tau Discriminator Value: " << tau->getUserRecord( *discr ) << std::endl;
            // std::cout << "Tau Discriminator Value TO DOUBLE: " << tau->getUserRecord( *discr ).toDouble() <<
            // std::endl;
            if (tau->getUserRecord(*discr).toDouble() < 0.5)
            {
                passID = false;
            }
            // std::cout << "Tau passID Value : " << passID << std::endl;
            // std::cout << " ***************** " << std::endl;
        }
    }
    else
    {
        for (std::vector<std::string>::const_iterator discr = m_tau_syst_discriminators.begin();
             discr != m_tau_syst_discriminators.end(); ++discr)
        {
            // In theory all tau discriminators have a value between 0 and 1.
            // Thus, they are saved as a float and the cut value is 0.5.
            // In practice most (or all) discriminators are boolean.
            // See also:
            // https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePFTauID#Discriminators

            // std::cout << "\n ********* isSyst  *********** " << std::endl;
            // std::cout << "Tau Discriminator Name: " <<  *discr  << std::endl;
            // std::cout << "Tau Discriminator Value: " << tau->getUserRecord( *discr ) << std::endl;
            // std::cout << "Tau Discriminator Value TO DOUBLE: " << tau->getUserRecord( *discr ).toDouble() <<
            // std::endl;

            if (tau->getUserRecord(*discr).toDouble() < 0.5)
            {
                passID = false;
            }
            // std::cout << "Tau passID Value : " << passID << std::endl;
            // std::cout << " ******** END isSyst  \n *********** " << std::endl;
        }
    }

    // NEWBY LOR
    bool passDecayMode = true;
    if (tau->getUserRecord("decayMode") == 5 or tau->getUserRecord("decayMode") == 6)
        passDecayMode = false;

    // std::cout << "  ****** END OBJECT SELECTION FOR 1 TAU************* " << std::endl;
    if (passKin && passID && passDecayMode)
    {
        std::cout << " ****** RETURN 0 ************* " << std::endl;
        return 0;
    }
    if (passKin && !passID && passDecayMode)
    { // std::cout << " ****** RETURN 1 ************* " << std::endl;
        return 1;
    }
    if (!passKin && passID && passDecayMode)
    { // std::cout << " ****** RETURN 2 ************* " << std::endl;
        return 2;
    }
    // std::cout << " ****** RETURN 3 ************* " << std::endl;
    return 3;

    // OLD
    // if      (passKin  && passID )return 0;
    // else if (passKin  && !passID )  return 1;
    // else if (!passKin && passID )  return 2;
    // return 3;
}

// tag the jets for tau double count
void TauSelector::tagJetsAsTau(std::vector<pxl::Particle *> &taus, std::vector<pxl::Particle *> &jets) const
{

    for (std::vector<pxl::Particle *>::const_iterator jet = jets.begin(); jet != jets.end(); ++jet)
    {
        for (std::vector<pxl::Particle *>::const_iterator tau = taus.begin(); tau != taus.end(); ++tau)
        {
            if (DeltaR((*jet), (*tau)) < 0.3 and (*tau)->hasUserRecord("systIDpassed") and
                (*tau)->getUserRecord("systIDpassed").toBool())
            {
                (*jet)->setUserRecord("isTau", true);
            }
        }
    }
}

double TauSelector::DeltaR(pxl::Particle *part_j, pxl::Particle *part_i) const
{
    return part_j->getVector().deltaR(part_i->getVector());
}
