// system include files
#include <cstdlib>
#include <memory>
#include <string>
#include <iostream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "NanoMUSiC/RoIScanner/include/scan.hpp"
//
// class declaration
//


class RoIScanner : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit RoIScanner(const edm::ParameterSet&);
  ~RoIScanner() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  // ----------member data ---------------------------
  const std::string jsonFilePath;
  const std::string outputDirectory;
  const int rounds;
  const int start_round;
  const std::string shiftsFilePath;
  const std::string lutFilePath;
  const std::string scanType;
  const std::string ec_name;
  const std::string distribution_type;
  const bool is_debug;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
RoIScanner::RoIScanner(const edm::ParameterSet& iConfig)
    : jsonFilePath(iConfig.getParameter<std::string>("jsonFilePath")) ,
     outputDirectory(iConfig.getParameter<std::string>("outputDirectory")) ,
     rounds(iConfig.getParameter<int>("rounds")) ,
     start_round(iConfig.getParameter<int>("start_round")) ,
     shiftsFilePath(iConfig.getParameter<std::string>("shiftsFilePath")) ,
     lutFilePath(iConfig.getParameter<std::string>("lutFilePath")) ,
     scanType(iConfig.getParameter<std::string>("scanType")) ,
     ec_name(iConfig.getParameter<std::string>("ec_name")) ,
     distribution_type(iConfig.getParameter<std::string>("distribution_type")) ,
     is_debug(iConfig.getParameter<bool>("is_debug")) 
{
  //now do what ever initialization is needed
}

RoIScanner::~RoIScanner() {
}

//
// member functions
//

// ------------ method called for each event  ------------
void RoIScanner::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
}

// ------------ method called once each job just before starting event loop  ------------
void RoIScanner::beginJob() {
	bool res = scan(jsonFilePath,
          outputDirectory,
          rounds,
          start_round,
          shiftsFilePath,
          lutFilePath,
          scanType,
          is_debug);

	if (not(res))
	{
		std::cerr << "ERROR: Could not run scanner." << std::endl;
		std::exit(EXIT_FAILURE);
	}

	std::string scan_is_done = "touch " + outputDirectory + "/yay.txt";
	std::system(scan_is_done.c_str());
}

// ------------ method called once each job just after ending the event loop  ------------
void RoIScanner::endJob() {
  // please remove this method if not needed
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void RoIScanner::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
	edm::ParameterSetDescription desc;
	desc.addUntracked<std::string>("input_file", "oioioi.json");
	    desc.add<std::string>("jsonFilePath");
	    desc.add<std::string>("outputDirectory");
	    desc.add<int>("rounds");
	    desc.add<int>("start_round");
	    desc.add<std::string>("shiftsFilePath");
	    desc.add<std::string>("lutFilePath");
	    desc.add<std::string>("scanType");
	    desc.add<std::string>("ec_name");
	    desc.add<std::string>("distribution_type");
	    desc.add<bool>("is_debug");
	descriptions.add("RoIScanner", desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(RoIScanner);
