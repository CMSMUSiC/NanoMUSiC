#ifndef VALIDATION
#define VALIDATION

#include <map>
#include <string>

// #include "Dijets.hpp"
#include "EventClassFactory.hpp"
#include "GammaPlusJet.hpp"
#include "TTBarTo1Lep2Bjet2JetMET.hpp"
#include "WToLepNuX.hpp"
#include "WToLepNu_eff.hpp"
#include "ZToLepLepX.hpp"
#include "ZToTauTauLepX.hpp"

const std::map<std::string, int> z_to_mu_mu_x_count_map = {{"Ele", 0},
                                                           {"EleEE", 0},
                                                           {"EleEB", 0},
                                                           {"Muon", 2},
                                                           {"Gamma", 0},
                                                           {"GammaEB", 0},
                                                           {"GammaEE", 0},
                                                           {"Tau", 0},
                                                           {"Jet", 0},
                                                           {"bJet", 0},
                                                           {"MET", 0}};

const std::map<std::string, int> z_to_ele_ele_x_count_map = {{"Ele", 2},
                                                             {"EleEE", 0},
                                                             {"EleEB", 0},
                                                             {"Muon", 0},
                                                             {"Gamma", 0},
                                                             {"GammaEB", 0},
                                                             {"GammaEE", 0},
                                                             {"Tau", 0},
                                                             {"Jet", 0},
                                                             {"bJet", 0},
                                                             {"MET", 0}};

const std::map<std::string, int> dijets_count_map = {{"Ele", 0},
                                                     {"EleEE", 0},
                                                     {"EleEB", 0},
                                                     {"Muon", 0},
                                                     {"Gamma", 0},
                                                     {"GammaEB", 0},
                                                     {"GammaEE", 0},
                                                     {"Tau", 0},
                                                     {"Jet", 2},
                                                     {"bJet", 0},
                                                     {"MET", 0}};

const std::map<std::string, int> gamma_plus_jet_count_map = {{"Ele", 0},
                                                             {"EleEE", 0},
                                                             {"EleEB", 0},
                                                             {"Muon", 0},
                                                             {"Gamma", 0},
                                                             {"GammaEB", 1},
                                                             {"GammaEE", 0},
                                                             {"Tau", 0},
                                                             {"Jet", 1},
                                                             {"bJet", 0},
                                                             {"MET", 0}};

const std::map<std::string, int> ttbar_to_1ele_2bjet_2jet_met_count_map = {{"Ele", 1},
                                                                           {"EleEE", 0},
                                                                           {"EleEB", 0},
                                                                           {"Muon", 0},
                                                                           {"Gamma", 0},
                                                                           {"GammaEB", 0},
                                                                           {"GammaEE", 0},
                                                                           {"Tau", 0},
                                                                           {"Jet", 2},
                                                                           {"bJet", 2},
                                                                           {"MET", 1}};

const std::map<std::string, int> ttbar_to_1mu_2bjet_2jet_met_count_map = {{"Ele", 0},
                                                                          {"EleEE", 0},
                                                                          {"EleEB", 0},
                                                                          {"Muon", 1},
                                                                          {"Gamma", 0},
                                                                          {"GammaEB", 0},
                                                                          {"GammaEE", 0},
                                                                          {"Tau", 0},
                                                                          {"Jet", 2},
                                                                          {"bJet", 2},
                                                                          {"MET", 1}};

const std::map<std::string, int> z_to_tau_tau_x_count_map = {{"Ele", 0},
                                                             {"EleEE", 0},
                                                             {"EleEB", 0},
                                                             {"Muon", 0},
                                                             {"Gamma", 0},
                                                             {"GammaEB", 0},
                                                             {"GammaEE", 0},
                                                             {"Tau", 2},
                                                             {"Jet", 0},
                                                             {"bJet", 0},
                                                             {"MET", 0}};
#endif // !VALIDATION