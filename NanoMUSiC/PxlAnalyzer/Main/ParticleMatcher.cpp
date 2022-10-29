#include "ParticleMatcher.hpp"

#include <iostream>

#include "Tools/MConfig.hpp"
#include "Tools/Tools.hpp"

using namespace std;
using namespace pxl;

ParticleMatcher::ParticleMatcher(Tools::MConfig const &cfg, int const debug)
    : m_DeltaR_Particles(cfg.GetItem<double>("Matching.DeltaR.particles")),
      m_DeltaR_MET(cfg.GetItem<double>("Matching.DeltaR.met")),
      m_DeltaPtoPt(cfg.GetItem<double>("Matching.DeltaPtOverPt")),
      m_DeltaCharge(cfg.GetItem<double>("Matching.DeltaCharge")),

      m_jet_bJets_use(cfg.GetItem<bool>("Jet.BJets.use")), m_jet_bJets_algo(""), m_jet_bJets_gen_label(""),

      m_gen_rec_map(cfg),

      m_debug(debug)
{
}

// ------------ matching Method ------------

void ParticleMatcher::matchObjects(EventView const *GenEvtView, EventView const *RecEvtView,
                                   std::string const &defaultLinkName, bool const customMatch) const
{
    // containers to keep the filtered gen/rec particles
    vector<Particle *> gen_particles;
    vector<Particle *> rec_particles;
    pxl::ParticleFilter particleFilter;

    for (GenRecNameMap::const_iterator objType = m_gen_rec_map.begin(); objType != m_gen_rec_map.end(); ++objType)
    {
        gen_particles.clear();
        rec_particles.clear();

        // Choose name filter criterion
        ParticlePtEtaNameCriterion const critRec((*objType).second.RecName);
        ParticlePtEtaNameCriterion const critGen((*objType).second.GenName);

        particleFilter.apply(RecEvtView->getObjectOwner(), rec_particles, critRec);

        if ((*objType).second.GenName == "gen")
        {
            // get Pdg ID of particle
            int pdg_id = 0;
            for (auto &pdg_name : Tools::pdg_id_type_map(m_jet_bJets_use))
            {
                if (pdg_name.second == (*objType).second.RecName)
                {
                    pdg_id = pdg_name.first;
                    break;
                }
            }
            std::vector<pxl::Particle *> tmp_particles;
            particleFilter.apply(GenEvtView->getObjectOwner(), tmp_particles, critGen);

            for (auto &tmp_particle : tmp_particles)
            {
                if (abs(tmp_particle->getPdgNumber()) == pdg_id)
                {
                    gen_particles.push_back(tmp_particle);
                }
            }
        }
        else
        {
            particleFilter.apply(GenEvtView->getObjectOwner(), gen_particles, critGen);
        }

        makeMatching(gen_particles, rec_particles, "Match", "hctaM", defaultLinkName);
    }

    // jet-subtype-matching:
    if (false)
    {
        // Get all Gen b-jets.
        gen_particles.clear();
        JetSubtypeCriterion const critBJetGen(m_gen_rec_map.get("Jet").GenName, m_jet_bJets_gen_label);
        particleFilter.apply(GenEvtView->getObjectOwner(), gen_particles, critBJetGen);

        // Get all Rec b-jets.
        rec_particles.clear();
        JetSubtypeCriterion const critBJetRec(m_gen_rec_map.get("Jet").RecName, m_jet_bJets_algo);
        particleFilter.apply(RecEvtView->getObjectOwner(), rec_particles, critBJetRec);

        makeMatching(gen_particles, rec_particles, "bJet-Match", "bJet-hctaM", "bJet-priv-gen-rec");

        // nonBJet(rec)-nonBjet(gen)-matching:
        gen_particles.clear();
        JetSubtypeCriterion const critJetGen(m_gen_rec_map.get("Jet").GenName, "nonB");
        particleFilter.apply(GenEvtView->getObjectOwner(), gen_particles, critJetGen);

        rec_particles.clear();
        JetSubtypeCriterion const critJetRec(m_gen_rec_map.get("Jet").RecName, "nonB");
        particleFilter.apply(RecEvtView->getObjectOwner(), rec_particles, critJetRec);

        makeMatching(gen_particles, rec_particles, "nonBJet-Match", "nonBJet-hctaM", "nonBJet-priv-gen-rec");
    }

    if (customMatch)
    {
        // Make matching for estimation of fake rate for gammas
        rec_particles.clear();
        ParticlePtEtaNameCriterion const critGamRec(m_gen_rec_map.get("Gamma").RecName);
        particleFilter.apply(RecEvtView->getObjectOwner(), rec_particles, critGamRec);

        gen_particles.clear();
        ParticlePtEtaNameCriterion const critEleGen(m_gen_rec_map.get("Ele").GenName);
        particleFilter.apply(GenEvtView->getObjectOwner(), gen_particles, critEleGen);

        makeMatching(gen_particles, rec_particles, "MatchGammaEle", "hctaMGammaEle", "priv-genEle-recGamma");

        gen_particles.clear();
        ParticlePtEtaNameCriterion const critJetGen(m_gen_rec_map.get("Jet").GenName);
        particleFilter.apply(GenEvtView->getObjectOwner(), gen_particles, critJetGen);

        makeMatching(gen_particles, rec_particles, "MatchGamma" + m_gen_rec_map.get("Jet").RecName,
                     "hctaMGamma" + m_gen_rec_map.get("Jet").RecName,
                     "priv-gen" + m_gen_rec_map.get("Jet").RecName + "-recGamma");

        // match SIM converted photons to GEN photons
        gen_particles.clear();
        ParticlePtEtaNameCriterion const critGamGen(m_gen_rec_map.get("Gamma").GenName);
        particleFilter.apply(GenEvtView->getObjectOwner(), gen_particles, critGamGen);

        // Use Rec particles container here.
        rec_particles.clear();
        ParticlePtEtaNameCriterion const critSIMConv("SIMConvGamma");
        particleFilter.apply(GenEvtView->getObjectOwner(), rec_particles, critSIMConv);

        makeMatching(gen_particles, rec_particles, "MatchGammaSIM", "hctaMGammaSIM", "priv-genGamma-SIMGammaConv");

        // match SIM converted photons to REC photons
        rec_particles.clear();
        particleFilter.apply(RecEvtView->getObjectOwner(), rec_particles, critGamRec);

        gen_particles.clear();
        particleFilter.apply(GenEvtView->getObjectOwner(), gen_particles, critSIMConv);

        makeMatching(gen_particles, rec_particles, "MatchGammaRecSIM", "hctaMGammaRecSIM",
                     "priv-recGamma-SIMGammaConv");
    }
}

// ------------ implementation of the matching Gen <--> Rec ------------
void ParticleMatcher::makeMatching(std::vector<Particle *> &gen_particles, std::vector<Particle *> &rec_particles,
                                   const string &Match, const string &hctaM, const string &linkname) const
{
    // First set for Gen all Matches to -1 and reset bools:
    // make sure that default value for ChargeMatch user record is set
    for (std::vector<Particle *>::iterator gen_iter = gen_particles.begin(); gen_iter != gen_particles.end();
         gen_iter++)
    {
        (*gen_iter)->setUserRecord(Match, -1);
        (*gen_iter)->setUserRecord(hctaM, false);
        (*gen_iter)->setUserRecord("Charge" + Match, -1);
    }
    // same for Rec
    for (std::vector<Particle *>::iterator rec_iter = rec_particles.begin(); rec_iter != rec_particles.end();
         rec_iter++)
    {
        (*rec_iter)->setUserRecord(Match, -1);
        (*rec_iter)->setUserRecord(hctaM, false);
        (*rec_iter)->setUserRecord("Charge" + Match, -1);
    }
    unsigned int num_gen = gen_particles.size();
    unsigned int num_rec = rec_particles.size();

    // we need at least one Gen and one Rec to perform matching!
    if (num_gen > 0 && num_rec > 0)
    {
        unsigned int col = 0;
        unsigned int row = 0;
        std::string particle;

        if (m_debug > 1)
        {
            cerr << "[INFO] (ParticleMatcher):" << endl;
            cerr << "Found " << num_gen << " Gen Objects and " << num_rec << " Rec Objects." << endl;
        }

        TMatrixT<double> DistanzMatrix(num_gen, num_rec);
        TMatrixT<double> DeltaPtoPtMatrix(num_gen, num_rec);
        TMatrixT<double> DeltaChargeMatrix(num_gen, num_rec);

        for (std::vector<Particle *>::iterator gen_iter = gen_particles.begin(); gen_iter != gen_particles.end();
             gen_iter++)
        {
            col = 0;
            for (std::vector<Particle *>::iterator rec_iter = rec_particles.begin(); rec_iter != rec_particles.end();
                 rec_iter++)
            {
                pxl::LorentzVector const &genVec = (*gen_iter)->getVector();
                pxl::LorentzVector const &recVec = (*rec_iter)->getVector();
                // Calculate the distance
                double const deltaR = genVec.deltaR(&recVec);
                double const deltaPtoPt = fabs((recVec.getPt() / genVec.getPt()) - 1);
                double const deltaQ = fabs((*rec_iter)->getCharge() - (*gen_iter)->getCharge());
                if (m_debug > 2)
                {
                    cerr << "[DEBUG] (ParticleMatcher): Matching information:" << endl;
                    cerr << "Gen: ";
                    (*gen_iter)->print(0, cerr);
                    cerr << "Rec: ";
                    (*rec_iter)->print(0, cerr);
                    cerr << "deltaR     = " << deltaR << endl;
                    cerr << "deltaPtoPt = " << deltaPtoPt << endl;
                    cerr << "deltaQ     = " << deltaQ << endl;
                }
                DistanzMatrix(row, col) = deltaR;
                DeltaPtoPtMatrix(row, col) = deltaPtoPt;
                DeltaChargeMatrix(row, col) = deltaQ;
                col++;
            }
            row++;
        }

        if (m_debug > 3)
        {
            cerr << "[DEBUG] (ParticleMatcher): Full DistanzMatrix:" << endl;
            DistanzMatrix.Print();
        }

        // define value in dR used as matching criterion
        double DeltaRMatching = m_DeltaR_Particles;
        // define value in DeltaPtoPt used as matching criterion
        double DeltaPtoPtMatching = m_DeltaPtoPt;
        // def value in Delta Charge used as matching criterion
        double DeltaChargeMatching = m_DeltaCharge;

        particle = (gen_particles.front())->getName();
        if (particle == m_gen_rec_map.get("MET").GenName)
            DeltaRMatching = m_DeltaR_MET;

        // go through every row and pushback index of Rec with smallest Distance
        for (unsigned int irow = 0; irow < num_gen; irow++)
        {
            int matched = SmallestRowElement(&DistanzMatrix, &DeltaPtoPtMatrix, &DeltaChargeMatrix, irow,
                                             DeltaRMatching, DeltaChargeMatching, DeltaPtoPtMatching);
            gen_particles[irow]->setUserRecord(Match, matched);
            if (m_debug > 1)
            {
                cerr << "[INFO] (ParticleMatcher):" << endl;
                cerr << "GenObject " << irow << " is matched with " << matched << endl;
            }

            if (matched != -1)
            {
                gen_particles[irow]->setUserRecord("Charge" + Match, DeltaChargeMatrix(irow, matched));
                // redundant information with softlink, should replace the UserRecords after testing
                gen_particles[irow]->linkSoft(rec_particles[matched], linkname);

                rec_particles[matched]->setUserRecord(hctaM, true);
                if (m_debug > 1)
                {
                    cerr << "[INFO] (ParticleMatcher):" << endl;
                    cerr << "RecObject " << matched << " has matching Gen " << endl;
                }
            }
        }

        for (unsigned int icol = 0; icol < num_rec; icol++)
        {
            // define value in dR which defines matching
            int matched = SmallestColumnElement(&DistanzMatrix, &DeltaPtoPtMatrix, &DeltaChargeMatrix, icol,
                                                DeltaRMatching, DeltaChargeMatching, DeltaPtoPtMatching);
            rec_particles[icol]->setUserRecord(Match, matched);
            if (m_debug > 1)
            {
                cerr << "[INFO] (ParticleMatcher):" << endl;
                cerr << "RecObject " << icol << " is matched with " << matched << endl;
            }

            if (matched != -1)
            {
                rec_particles[icol]->setUserRecord("Charge" + Match, DeltaChargeMatrix(matched, icol));
                // redundant information with softlink, should replace the UserRecords after testing
                rec_particles[icol]->linkSoft(gen_particles[matched], linkname);
                gen_particles[matched]->setUserRecord(hctaM, true);
                if (m_debug > 1)
                {
                    cerr << "[INFO] (ParticleMatcher):" << endl;
                    cerr << "GenObject " << matched << " has matching Rec " << endl;
                }
            }
        }
    }
}

// ---------------------- Helper Method ------------------------------

int ParticleMatcher::SmallestRowElement(TMatrixT<double> *matrixDR, TMatrixT<double> *matrixDp,
                                        TMatrixT<double> *matrixDC, const unsigned int &row,
                                        const double &DeltaRMatching, const double &DeltaChargeMatching,
                                        const double &DeltaPtoPtMatching) const
{

    // loop over row and return index of smallest element
    double elementDR = (*matrixDR)(row, 0);
    double elementDp = (*matrixDp)(row, 0);
    double elementDC = (*matrixDC)(row, 0);
    int index = 0;
    for (int i = 1; i < matrixDR->GetNcols(); i++)
    {
        if ((*matrixDR)(row, i) < elementDR)
        {
            elementDR = (*matrixDR)(row, i);
            elementDp = (*matrixDp)(row, i);
            elementDC = (*matrixDC)(row, i);
            index = i;
        }
    }
    if ((elementDR > DeltaRMatching) || (elementDp > DeltaPtoPtMatching) || (elementDC > DeltaChargeMatching))
        index = -1;
    return index;
}

// ---------------------- Helper Method ------------------------------

int ParticleMatcher::SmallestColumnElement(TMatrixT<double> *matrixDR, TMatrixT<double> *matrixDp,
                                           TMatrixT<double> *matrixDC, const unsigned int &col,
                                           const double &DeltaRMatching, const double &DeltaChargeMatching,
                                           const double &DeltaPtoPtMatching) const
{

    // loop over row and return index of smallest element
    double elementDR = (*matrixDR)(0, col);
    double elementDp = (*matrixDp)(0, col);
    double elementDC = (*matrixDC)(0, col);
    int index = 0;
    for (int i = 1; i < matrixDR->GetNrows(); i++)
    {
        if ((*matrixDR)(i, col) < elementDR)
        {
            elementDR = (*matrixDR)(i, col);
            elementDp = (*matrixDp)(i, col);
            elementDC = (*matrixDC)(i, col);
            index = i;
        }
    }
    if ((elementDR > DeltaRMatching) || (elementDp > DeltaPtoPtMatching) || (elementDC > DeltaChargeMatching))
        index = -1;
    return index;
}
