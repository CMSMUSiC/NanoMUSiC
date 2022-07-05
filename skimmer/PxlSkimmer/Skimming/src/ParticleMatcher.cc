// -*- C++ -*-
// Copyright [2015] <RWTH Aachen, III. Phys. Inst. A>

#include "PxlSkimmer/Skimming/interface/ParticleMatcher.h"

#include "TMatrixT.h"
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"


// ------------ matching Method ------------

void ParticleMatcher::matchObjects(pxl::EventView *GenView, pxl::EventView *RecView, const std::vector< jet_def > &jet_infos, const std::string &_METType) {
    // FIXME: Make code more generic! Generate a list of all Particle types
    std::vector<std::string> typeList;
    typeList.push_back("Muon");
    typeList.push_back("Ele");
    typeList.push_back("Gamma");
    typeList.push_back("Tau");//typeList.push_back("slimmedTausNewID");
    typeList.push_back(_METType);
    for (std::vector< jet_def >::const_iterator jet_info = jet_infos.begin(); jet_info != jet_infos.end(); ++jet_info) {
        typeList.push_back(jet_info->name);
    }
    pxl::ParticleFilter _particleFilter;

    // containers to keep the filtered gen/rec particles
    std::vector<pxl::Particle*> gen_particles;
    std::vector<pxl::Particle*> rec_particles;
    for (std::vector<std::string>::const_iterator partType = typeList.begin(); partType != typeList.end(); ++partType) {
        // Choose name filter criterion
        gen_particles.clear();
        rec_particles.clear();
        pxl::ParticlePtEtaNameCriterion crit(*partType);
        _particleFilter.apply(GenView->getObjectOwner(), gen_particles, crit);
        _particleFilter.apply(RecView->getObjectOwner(), rec_particles, crit);
        makeMatching(gen_particles, rec_particles, _METType);
    }
}

// ------------ implementation of the matching Gen <--> Rec ------------

void ParticleMatcher::makeMatching(std::vector<pxl::Particle*>& gen_particles, std::vector<pxl::Particle*>& rec_particles, const std::string& _METType) {
    // First set for Gen all Matches to -1 and reset bools:
    for (std::vector<pxl::Particle*>::iterator gen_iter = gen_particles.begin(); gen_iter != gen_particles.end(); gen_iter++) {
        (*gen_iter)->setUserRecord("Match", -1);
        (*gen_iter)->setUserRecord("hctaM", false);
    }
    // same for Rec
    for (std::vector<pxl::Particle*>::iterator rec_iter = rec_particles.begin(); rec_iter != rec_particles.end(); rec_iter++) {
        (*rec_iter)->setUserRecord("Match", -1);
        (*rec_iter)->setUserRecord("hctaM", false);
    }
    unsigned int num_gen = gen_particles.size();
    unsigned int num_rec = rec_particles.size();

    // we need at least one Gen and one Rec to perform matching!
    if (num_gen > 0 && num_rec > 0) {
        unsigned int col = 0;
        unsigned int row = 0;
        std::string particle;

        if (_fDebug > 1) std::cout << "Found " << num_gen << " Gen Objects and " << num_rec << " Rec Objects" << std::endl;

        TMatrixT<double> DistanzMatrix(num_gen, num_rec);
        TMatrixT<double> DeltaPtoPtMatrix(num_gen, num_rec);
        TMatrixT<double> DeltaChargeMatrix(num_gen, num_rec);

        for (std::vector<pxl::Particle*>::iterator gen_iter = gen_particles.begin(); gen_iter != gen_particles.end(); gen_iter++) {
            col = 0;
            for (std::vector<pxl::Particle*>::iterator rec_iter = rec_particles.begin(); rec_iter != rec_particles.end(); rec_iter++) {
                // Calculate the distance
                if (_fDebug > 0) {
                  std::cout << "Gen: ";
                  (*gen_iter)->print(0);
                  std::cout << "Rec: ";
                  (*rec_iter)->print(0);
                    std::cout << "Distance: " << (*gen_iter)->getVector().deltaR(&((*rec_iter)->getVector())) << std::endl;
                }
                DistanzMatrix(row, col) = (*gen_iter)->getVector().deltaR(&((*rec_iter)->getVector()));
                DeltaPtoPtMatrix(row, col) = fabs(((*rec_iter)->getVector().getPt() / (*gen_iter)->getVector().getPt()) - 1);
                DeltaChargeMatrix(row, col) = fabs(((*rec_iter)->getCharge()) - ((*gen_iter)->getCharge()));
                col++;
            }
            row++;
        }


        if (_fDebug > 0) DistanzMatrix.Print();

        // define value in dR used as matching criterion
        double DeltaRMatching = _DeltaR_Particles;
        // define value in DeltaPtoPt used as matching criterion
        double DeltaPtoPtMatching = _DeltaPtoPt;
        // def value in Delta Charge used as matching criterion
        double DeltaChargeMatching = _DeltaCharge;

        particle = (gen_particles.front())->getName();
        if (particle == _METType) DeltaRMatching = _DeltaR_MET;

        // go through every row and pushback index of Rec with smallest Distance
        for (unsigned int irow = 0; irow < num_gen; irow++) {
            int matched = SmallestRowElement(&DistanzMatrix, &DeltaPtoPtMatrix, &DeltaChargeMatrix, irow, DeltaRMatching, DeltaChargeMatching, DeltaPtoPtMatching);
            gen_particles[irow]->setUserRecord("Match", matched);
            if (_fDebug > 0) std::cout << "GenObject " << irow << " is matched with " << matched << std::endl;

            /*
            // ugly piece of code that enforces on e to one matching for comparison with PAT matching
            if (matched !=-1) {
            for(unsigned int loop = 0; loop < num_gen; ++loop){
            DeltaChargeMatrix(loop, matched) = 10.0;
            }
            }
            */


            if (matched != -1) {
                // redundant information with softlink, should replace the UserRecords after testing
                gen_particles[irow]->linkSoft(rec_particles[matched], "priv-gen-rec");

                // std::cout << "pt of the private matched " << rec_particles[matched]->getName() << " rec: " << rec_particles[matched]->getPt() << std::endl; // temporary!
                // std::cout << "pt of the private matched " << gen_particles[irow]->getName() << " gen: " << gen_particles[irow]->getPt() << std::endl; // temporary!

                rec_particles[matched]->setUserRecord("hctaM", true);
                if (_fDebug > 0) std::cout << "RecObject " << matched << " has matching Gen " << std::endl;
            }
        }

        for (unsigned int icol = 0; icol < num_rec; icol++) {
            // define value in dR which defines matching
            int matched = SmallestColumnElement(&DistanzMatrix, &DeltaPtoPtMatrix, &DeltaChargeMatrix, icol, DeltaRMatching, DeltaChargeMatching, DeltaPtoPtMatching);
            rec_particles[icol]->setUserRecord("Match", matched);
            if (_fDebug > 0) std::cout << "RecObject " << icol << " is matched with " << matched << std::endl;

            if (matched != -1) {
                // redundant information with softlink, should replace the UserRecords after testing
                rec_particles[icol]->linkSoft(gen_particles[matched], "priv-rec-gen");
                gen_particles[matched]->setUserRecord("hctaM", true);
                if (_fDebug > 0) std::cout << "GenObject " << matched << " has matching Rec " << std::endl;
            }
        }
    }
}

// ---------------------- Helper Method ------------------------------

int ParticleMatcher::SmallestRowElement(TMatrixT<double>* matrixDR, TMatrixT<double>* matrixDp, TMatrixT<double>* matrixDC, const unsigned int& row, const double& DeltaRMatching, const double& DeltaChargeMatching, const double& DeltaPtoPtMatching) {
    // loop over row and return index of smallest element
    double elementDR = (*matrixDR)(row, 0);
    double elementDp = (*matrixDp)(row, 0);
    double elementDC = (*matrixDC)(row, 0);
    int index = 0;
    for (int i = 1; i < matrixDR->GetNcols(); i++) {
        if ((*matrixDR)(row, i) < elementDR) {
            elementDR = (*matrixDR)(row, i);
            elementDp = (*matrixDp)(row, i);
            elementDC = (*matrixDC)(row, i);
            index = i;
        }
    }
    if ((elementDR > DeltaRMatching) || (elementDp > DeltaPtoPtMatching) || ( elementDC > DeltaChargeMatching)) index = -1;
    return index;
}

// ---------------------- Helper Method ------------------------------

int ParticleMatcher::SmallestColumnElement(TMatrixT<double>* matrixDR, TMatrixT<double>* matrixDp, TMatrixT<double>* matrixDC, const unsigned int& col,
                                           const double& DeltaRMatching, const double& DeltaChargeMatching, const double& DeltaPtoPtMatching) {
    // loop over row and return index of smallest element
    double elementDR = (*matrixDR)(0, col);
    double elementDp = (*matrixDp)(0, col);
    double elementDC = (*matrixDC)(0, col);
    int index = 0;
    for (int i = 1; i < matrixDR->GetNrows(); i++) {
        if ((*matrixDR)(i, col) < elementDR) {
            elementDR = (*matrixDR)(i, col);
            elementDp = (*matrixDp)(i, col);
            elementDC = (*matrixDC)(i, col);
            index = i;
        }
    }
    if ((elementDR > DeltaRMatching) || (elementDp > DeltaPtoPtMatching) || ( elementDC > DeltaChargeMatching)) index = -1;
    return index;
}
