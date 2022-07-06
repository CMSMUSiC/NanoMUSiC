#include "specialAna.hh"
#include "HistClass.hh"
#include "Tools/Tools.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include "boost/format.hpp"
#pragma GCC diagnostic pop

specialAna::specialAna( const Tools::MConfig &cfg ) :
   runOnData(       cfg.GetItem< bool >( "General.RunOnData" ) ),
   m_JetAlgo(       cfg.GetItem< string >( "Jet.Type.Rec" ) ),
   m_BJets_algo(    cfg.GetItem< string >( "Jet.BJets.Algo" ) ),
   m_METType(       cfg.GetItem< string >( "MET.Type.Rec" ) ),
   m_TauType(       cfg.GetItem< string >( "Tau.Type.Rec" ) ),

//   m_trigger_string( Tools::splitString< string >( cfg.GetItem< string >( "wprime.TriggerList" ), true  ) ),
   d_mydiscmu(  {"isPFMuon","isGlobalMuon","isTrackerMuon","isStandAloneMuon","isTightMuon","isHighPtMuon"} ),
   m_dataPeriod("8TeV"),
   config_(cfg)
{

    string safeFileName = "SpecialHistos.root";
    file1 = new TFile(safeFileName.c_str(), "RECREATE");
    events_ = 0;

    // number of events, saved in a histogram
    HistClass::CreateHistoUnchangedName("h_counters", 10, 0, 11, "N_{events}");

    for(unsigned int i=0;i<4;i++){
        //str(boost::format("N_{%s}")%particleLatex[i] )
        HistClass::CreateHisto("num",particles[i].c_str(), 40, 0, 39,            TString::Format("N_{%s}", particleSymbols[i].c_str()) );
        HistClass::CreateHisto(3,"pt",particles[i].c_str(), 5000, 0, 5000,       TString::Format("p_{T}^{%s} (GeV)", particleSymbols[i].c_str()) );
        HistClass::CreateHisto(3,"eta",particles[i].c_str(), 80, -4, 4,          TString::Format("#eta_{%s}", particleSymbols[i].c_str()) );
        HistClass::CreateHisto(3,"phi",particles[i].c_str(), 40, -3.2, 3.2,      TString::Format("#phi_{%s} (rad)", particleSymbols[i].c_str()) );

        if(not runOnData){
            HistClass::CreateHisto(1,"num_Gen",particles[i].c_str(), 40, 0, 39,        TString::Format("N_{%s}", particleSymbols[i].c_str()) );
            HistClass::CreateHisto(1,"pt_Gen",particles[i].c_str(), 5000, 0, 5000,     TString::Format("p_{T}^{%s} (GeV)", particleSymbols[i].c_str()) );
            HistClass::CreateHisto(1,"eta_Gen",particles[i].c_str(), 80, -4, 4,        TString::Format("#eta_{%s}", particleSymbols[i].c_str()) );
            HistClass::CreateHisto(1,"phi_Gen",particles[i].c_str(), 40, -3.2, 3.2,    TString::Format("#phi_{%s} (rad)", particleSymbols[i].c_str()) );
        }
    }
}

specialAna::~specialAna() {
}

void specialAna::analyseEvent( const pxl::Event* event ) {
    initEvent( event );
    //if(tail_selector(event)) return;
    if(not runOnData){
        Fill_Gen_Controll_histo();
    }

    for(uint i = 0; i < MuonList->size(); i++){
        if(MuonList->at(i)->getPt() < 25 or TMath::Abs(MuonList->at(i)->getEta()) > 2.1)continue;
        Fill_Particle_histos(0, MuonList->at(i));
    }
    HistClass::Fill("Muon_num",MuonList->size(),weight);

    for(uint i = 0; i < EleList->size(); i++){
        if(EleList->at(i)->getPt() < 25 or TMath::Abs(EleList->at(i)->getEta()) > 2.5 or (TMath::Abs(EleList->at(i)->getEta()) > 1.442 and TMath::Abs(EleList->at(i)->getEta()) < 1.56))continue;
        Fill_Particle_histos(0, EleList->at(i));
    }
    HistClass::Fill("Ele_num",EleList->size(),weight);

    for(uint i = 0; i < TauList->size(); i++){
        Fill_Particle_histos(0, TauList->at(i));
    }
    HistClass::Fill("Tau_num",TauList->size(),weight);

    for(uint i = 0; i < METList->size(); i++){
        Fill_Particle_histos(0, METList->at(i));
    }
    HistClass::Fill("MET_num",METList->size(),weight);

    if (!TriggerSelector(event)) return;

    for(uint i = 0; i < MuonList->size(); i++){
        if(MuonList->at(i)->getPt() < 25 or TMath::Abs(MuonList->at(i)->getEta()) > 2.1)continue;
        Fill_Particle_histos(1, MuonList->at(i));
    }

    for(uint i = 0; i < EleList->size(); i++){
        if(EleList->at(i)->getPt() < 25 or TMath::Abs(EleList->at(i)->getEta()) > 2.5 or (TMath::Abs(EleList->at(i)->getEta()) > 1.442 and TMath::Abs(EleList->at(i)->getEta()) < 1.56))continue;
        Fill_Particle_histos(1, EleList->at(i));
    }

    for(uint i = 0; i < TauList->size(); i++){
        Fill_Particle_histos(1, TauList->at(i));
    }

    for(uint i = 0; i < METList->size(); i++){
        Fill_Particle_histos(1, METList->at(i));
    }

    for(uint i = 0; i < MuonList->size(); i++){
        if(MuonList->at(i)->getPt() < 25 or TMath::Abs(MuonList->at(i)->getEta()) > 2.1)continue;
        if(MuonList->at(i)->getUserRecord("IDpassed").asBool()){
            Fill_Particle_histos(2, MuonList->at(i));
        }
    }

    for(uint i = 0; i < EleList->size(); i++){
        if(EleList->at(i)->getPt() < 25 or TMath::Abs(EleList->at(i)->getEta()) > 2.5 or (TMath::Abs(EleList->at(i)->getEta()) > 1.442 and TMath::Abs(EleList->at(i)->getEta()) < 1.56))continue;
        if(EleList->at(i)->getUserRecord("IDpassed").asBool()){
            Fill_Particle_histos(2, EleList->at(i));
        }
    }

    for(uint i = 0; i < TauList->size(); i++){
        //if(Check_Tau_ID(TauList->at(i))){
            Fill_Particle_histos(2, TauList->at(i));
        //}
    }

    for(uint i = 0; i < METList->size(); i++){
        Fill_Particle_histos(2, METList->at(i));
    }

    endEvent( event );
}

void specialAna::FillSystematics(const pxl::Event* event, std::string const particleName){
    FillSystematicsUpDown(event, particleName, "Up", "Scale");
    FillSystematicsUpDown(event, particleName, "Down", "Scale");
    //FillSystematicsUpDown(event, particleName, "Up", "Resolution");
    //FillSystematicsUpDown(event, particleName, "Down", "Resolution");
}

void specialAna::FillSystematicsUpDown(const pxl::Event* event, std::string const particleName, std::string const updown, std::string const shiftType){
    pxl::EventView *tempEventView;

    // extract one EventView
    // make sure the object key is the same as in Systematics.cc specified
//     tempEventView = event->getObjectOwner().findObject< pxl::EventView >(particleName + "_syst" + shiftType + updown);
    tempEventView = event->findObject< pxl::EventView >(particleName + "_syst" + shiftType + updown);



    if(tempEventView == 0){
        throw std::runtime_error("specialAna.cc: no EventView '" + particleName + "_syst" + shiftType + updown + "' found!");
    }
    // get all particles
    std::vector< pxl::Particle* > shiftedParticles;
    tempEventView->getObjectsOfType< pxl::Particle >(shiftedParticles);

    //backup OldList
    RememberMET=METList;
    METList = new vector< pxl::Particle* >;
    if(particleName=="Muon"){
        RememberPart=MuonList;
        MuonList = new vector< pxl::Particle* >;
        for( vector< pxl::Particle* >::const_iterator part_it = shiftedParticles.begin(); part_it != shiftedParticles.end(); ++part_it ) {
            pxl::Particle *part = *part_it;
            string Name = part->getName();
            if(      Name == "Muon"    ) MuonList->push_back( part );
            else if( Name == m_METType ) METList->push_back( part );
        }
    }else if(particleName=="Ele"){
        RememberPart=EleList;
        EleList = new vector< pxl::Particle* >;
        for( vector< pxl::Particle* >::const_iterator part_it = shiftedParticles.begin(); part_it != shiftedParticles.end(); ++part_it ) {
            pxl::Particle *part = *part_it;
            string Name = part->getName();
            if(      Name == "Ele"     ) EleList->push_back( part );
            else if( Name == m_METType ) METList->push_back( part );
        }
    }else if(particleName=="Tau"){
        RememberPart=TauList;
        TauList = new vector< pxl::Particle* >;
        for( vector< pxl::Particle* >::const_iterator part_it = shiftedParticles.begin(); part_it != shiftedParticles.end(); ++part_it ) {
            pxl::Particle *part = *part_it;
            string Name = part->getName();
            if(      Name == m_TauType ) TauList->push_back( part );
            else if( Name == m_METType ) METList->push_back( part );
        }
    }//else if(particleName=="JET"){
    //}else if(particleName==m_METType){}

    // reset the chosen MET and lepton
    if(METList->size()>0){
        sel_met=METList->at(0);
    }else{
        sel_met=0;
    }
    sel_lepton=0;

    //KinematicsSelector();

    if(sel_lepton && sel_met){
        //if(sel_lepton->getUserRecord("passedDeltaPhi")){
            //Fill_Particle_hisos(1, sel_lepton, shiftType + updown);
        //}
        //if(sel_lepton->getUserRecord("passedPtMet")){
            //Fill_Particle_hisos(2, sel_lepton, shiftType + updown);
        //}
        if(sel_lepton->getUserRecord("passed")){
            //Fill_Particle_histos(3, sel_lepton, particleName + shiftType + updown);
            Fill_Particle_histos(3, sel_lepton);
            // cout << "h1_3_" << sel_lepton->getName() << "_[val]_syst_" + particleName + shiftType + updown << endl;
        }
    }


    // return to backup
    delete METList;
    METList = RememberMET;
    if(particleName=="Muon"){
        delete MuonList;
        MuonList = RememberPart;
    }else if(particleName=="Ele"){
        delete EleList;
        EleList = RememberPart;
    }else if(particleName=="Tau"){
        delete TauList;
        TauList = RememberPart;
    }//else if(particleName=="JET"){
    //}else if(particleName==m_METType){}

}

bool specialAna::Check_Tau_ID(pxl::Particle* tau) {
    bool passed = false;
    bool tau_ID = tau->getUserRecord("decayModeFindingNewDMs").asBool();
    bool tau_ISO = tau->getUserRecord("byTightIsolationMVA3newDMwLT").asBool();
    bool tau_ELE = tau->getUserRecord("againstElectronMediumMVA5"/*"againstElectronTightMVA5"*/).asBool();
    bool tau_MUO = tau->getUserRecord("againstMuonMedium2"/*"againstMuonTightMVA"*/).asBool();
    if (tau_ID && tau_ISO && tau_ELE && tau_MUO) passed = true;
    return passed;
}

bool specialAna::TriggerSelector(const pxl::Event* event) {
    bool triggered = false;

    std::vector< std::string >  m_trigger_string;
    m_trigger_string.push_back("HLT_HLT_Ele90_CaloIdVT_GsfTrkIdT");
    m_trigger_string.push_back("HLT_Ele80_CaloIdVT_GsfTrkIdT");
    m_trigger_string.push_back("HLT_Ele80_CaloIdVT_TrkIdT");
    m_trigger_string.push_back("HLT_HLT_Mu40_v");
    m_trigger_string.push_back("HLT_HLT_Mu50_v");
    m_trigger_string.push_back("HLT_HLT_Mu40_eta2p1_v");
    m_trigger_string.push_back("HLT_MonoCentralPFJet80");

    for (auto const it : m_trigger_string) {

        for (auto us : m_TrigEvtView->getUserRecords()) {
            if (std::string::npos != us.first.find(it)) {
                triggered = true;
                triggers.insert(us.first);
            }
        }
    }

     return (triggered);
}

void specialAna::Fill_Gen_Controll_histo() {
    int muon_gen_num=0;
    int ele_gen_num=0;
    int tau_gen_num=0;
    for(uint i = 0; i < S3ListGen->size(); i++){
        if (S3ListGen->at(i)->getPt()<10 && not (TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 24)   ){
            continue;
        }
        if(S3ListGen->at(i)->getPdgNumber()==0){
            if(S3ListGen->at(i)->hasUserRecord("id")){
                S3ListGen->at(i)->setPdgNumber(S3ListGen->at(i)->getUserRecord("id"));
            }
        }
        if(TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 13){
            muon_gen_num++;
            HistClass::Fill(0,"Muon_pt_Gen",S3ListGen->at(i)->getPt(),m_GenEvtView->getUserRecord( "Weight" ));
            HistClass::Fill(0,"Muon_eta_Gen",S3ListGen->at(i)->getEta(),m_GenEvtView->getUserRecord( "Weight" ));
            HistClass::Fill(0,"Muon_phi_Gen",S3ListGen->at(i)->getPhi(),m_GenEvtView->getUserRecord( "Weight" ));
        }else if(TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 15){
            tau_gen_num++;
            HistClass::Fill(0,"Tau_pt_Gen",S3ListGen->at(i)->getPt(),m_GenEvtView->getUserRecord( "Weight" ));
            HistClass::Fill(0,"Tau_eta_Gen",S3ListGen->at(i)->getEta(),m_GenEvtView->getUserRecord( "Weight" ));
            HistClass::Fill(0,"Tau_phi_Gen",S3ListGen->at(i)->getPhi(),m_GenEvtView->getUserRecord( "Weight" ));
        }else if(TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 11){
            ele_gen_num++;
            HistClass::Fill(0,"Ele_pt_Gen",S3ListGen->at(i)->getPt(),m_GenEvtView->getUserRecord( "Weight" ));
            HistClass::Fill(0,"Ele_eta_Gen",S3ListGen->at(i)->getEta(),m_GenEvtView->getUserRecord( "Weight" ));
            HistClass::Fill(0,"Ele_phi_Gen",S3ListGen->at(i)->getPhi(),m_GenEvtView->getUserRecord( "Weight" ));
        }
    }

    HistClass::Fill(0,"Tau_num_Gen",tau_gen_num,m_GenEvtView->getUserRecord( "Weight" ));
    HistClass::Fill(0,"Muon_num_Gen",muon_gen_num,m_GenEvtView->getUserRecord( "Weight" ));
    HistClass::Fill(0,"Ele_num_Gen",ele_gen_num,m_GenEvtView->getUserRecord( "Weight" ));
}

void specialAna::Fill_Particle_histos(int hist_number, pxl::Particle* lepton){
    string name=lepton->getName();
    if(lepton->getName()==m_TauType){
        name="Tau";
    }
    if(lepton->getName()==m_METType){
        name="MET";
    }
    HistClass::Fill(hist_number,str(boost::format("%s_pt")%name ),lepton->getPt(),weight);
    HistClass::Fill(hist_number,str(boost::format("%s_eta")%name ),lepton->getEta(),weight);
    HistClass::Fill(hist_number,str(boost::format("%s_phi")%name ),lepton->getPhi(),weight);
}

double specialAna::DeltaPhi(double a, double b) {
  double temp = fabs(a-b);
  if (temp <= TMath::Pi())
    return temp;
  else
    return  2.*TMath::Pi() - temp;
}

double specialAna::DeltaPhi(pxl::Particle* lepton, pxl::Particle* met) {
    double a=lepton->getPhi();
    double b=met->getPhi();
    double temp = fabs(a-b);
    if (temp <= TMath::Pi())
        return temp;
    else
        return  2.*TMath::Pi() - temp;
}

double specialAna::MT(pxl::Particle* lepton, pxl::Particle* met) {
    double mm = 2 * lepton->getPt() * met->getPt() * ( 1. - cos(lepton->getPhi() - met->getPhi()) );
    return sqrt(mm);
}

double specialAna::getPtHat(){
    double pthat=0;
    pxl::Particle* w=0;
    pxl::Particle* lepton=0;
    for(uint i = 0; i < S3ListGen->size(); i++){
        if(TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 24){
            w=S3ListGen->at(i);
        }
        //take the neutrio to avoid showering and so on!!
        if((TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 12 || TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 14 || TMath::Abs(S3ListGen->at(i)->getPdgNumber()) == 16) && lepton==0){
            lepton=S3ListGen->at(i);
        }
        if(w!=0 && lepton!=0){
            break;
        }
    }

    if(w!=0 && lepton!=0){
        //boost in the w restframe
        lepton->boost( -(w->getBoostVector()) );
        pthat=lepton->getPt();
    }else{
        pthat=-1;
    }
    return pthat;
}

void specialAna::endJob( const Serializable* ) {

    file1->cd();
    HistClass::WriteAll("counters");
    if(not runOnData){
        file1->mkdir("MC");
        file1->cd("MC/");
        HistClass::WriteAll("_Gen");
    }
    file1->cd();
    file1->mkdir("Taus");
    file1->cd("Taus/");
    HistClass::WriteAll("Tau_");
    //HistClass::Write2("Tau_eta_phi");
    file1->cd();
    file1->mkdir("Muons");
    file1->cd("Muons/");
    HistClass::WriteAll("Muon_");
    file1->cd();
    file1->mkdir("METs");
    file1->cd("METs/");
    HistClass::WriteAll("MET_");
    file1->cd();
    file1->mkdir("Eles");
    file1->cd("Eles/");
    HistClass::WriteAll("Ele_");
    file1->cd();
    file1->mkdir("Trees");
    file1->cd("Trees/");
    HistClass::WriteAllTrees();
    file1->cd();
    file1->mkdir("nDim");
    file1->cd("nDim");
    HistClass::WriteN();
    file1->Close();

    delete file1;
}

void specialAna::initEvent( const pxl::Event* event ){
    HistClass::Fill("h_counters", 1, 1); // increment number of events
    events_++;

    //no pu weight at the moment!!

    weight = 1;
    m_RecEvtView = event->getObjectOwner().findObject< pxl::EventView >( "Rec" );
    m_GenEvtView = event->getObjectOwner().findObject< pxl::EventView >( "Gen" );
    if(event->getObjectOwner().findObject< pxl::EventView >( "Trig" )){
        m_TrigEvtView = event->getObjectOwner().findObject< pxl::EventView >( "Trig" );
    }else{
        m_TrigEvtView = event->getObjectOwner().findObject< pxl::EventView >( "Rec" );
    }

    temp_run = event->getUserRecord( "Run" );
    temp_ls = event->getUserRecord( "LumiSection" );
    temp_event = event->getUserRecord( "EventNum" );

    numMuon  = m_RecEvtView->getUserRecord( "NumMuon" );
    numEle   = m_RecEvtView->getUserRecord( "NumEle" );
    numGamma = m_RecEvtView->getUserRecord( "NumGamma" );
    numTau   = m_RecEvtView->getUserRecord( "Num" + m_TauType );
    numMET   = m_RecEvtView->getUserRecord( "Num" + m_METType );
    numJet   = m_RecEvtView->getUserRecord( "Num" + m_JetAlgo );
    numBJet  = m_RecEvtView->getUserRecord_def( "Num" + m_BJets_algo,-1 );

    EleList   = new vector< pxl::Particle* >;
    MuonList  = new vector< pxl::Particle* >;
    GammaList = new vector< pxl::Particle* >;
    METList   = new vector< pxl::Particle* >;
    JetList   = new vector< pxl::Particle* >;
    TauList   = new vector< pxl::Particle* >;

    // get all particles
    vector< pxl::Particle* > AllParticles;
    m_RecEvtView->getObjectsOfType< pxl::Particle >( AllParticles );
    pxl::sortParticles( AllParticles );
    // push them into the corresponding vectors
    for( vector< pxl::Particle* >::const_iterator part_it = AllParticles.begin(); part_it != AllParticles.end(); ++part_it ) {
        pxl::Particle *part = *part_it;
        string Name = part->getName();
        // Only fill the collection if we want to use the particle!
        if(      Name == "Muon"    ) MuonList->push_back( part );
        else if( Name == "Ele"     ) EleList->push_back( part );
        else if( Name == "Gamma"   ) GammaList->push_back( part );
        else if( Name == m_TauType   ) TauList->push_back( part );
        else if( Name == m_METType ) METList->push_back( part );
        else if( Name == m_JetAlgo ) JetList->push_back( part );
    }

    if(METList->size()>0){
        sel_met=METList->at(0);
    }else{
        sel_met=0;
    }
    sel_lepton=0;

    EleListGen     = new vector< pxl::Particle* >;
    MuonListGen    = new vector< pxl::Particle* >;
    GammaListGen   = new vector< pxl::Particle* >;
    METListGen     = new vector< pxl::Particle* >;
    JetListGen     = new vector< pxl::Particle* >;
    TauListGen     = new vector< pxl::Particle* >;
    S3ListGen      = new vector< pxl::Particle* >;

    if( not runOnData ){

        double event_weight = m_GenEvtView->getUserRecord( "Weight" );
        //double varKfactor_weight = m_GenEvtView->getUserRecord_def( "kfacWeight",1. );
        double pileup_weight = m_GenEvtView->getUserRecord_def( "PUWeight",1.);

        if(m_dataPeriod=="13TeV"){
            weight = event_weight ;
        }else if(m_dataPeriod=="8TeV"){
            weight = event_weight  * pileup_weight;
        }else{
            stringstream error;
            error << "The data period "<<m_dataPeriod<<" is not supported by this analysis!\n";
            throw Tools::config_error( error.str() );
        }

        // get all particles
        vector< pxl::Particle* > AllParticlesGen;
        m_GenEvtView->getObjectsOfType< pxl::Particle >( AllParticlesGen );
        pxl::sortParticles( AllParticlesGen );
        // push them into the corresponding vectors
        string genCollection="gen";
        if(m_dataPeriod=="8TeV"){
            genCollection="S3";
        }
        for( vector< pxl::Particle* >::const_iterator part_it = AllParticlesGen.begin(); part_it != AllParticlesGen.end(); ++part_it ) {
            pxl::Particle *part = *part_it;
            string Name = part->getName();
            // Only fill the collection if we want to use the particle!
            if(      Name == "Muon"    ) MuonListGen->push_back( part );
            else if( Name == "Ele"     ) EleListGen->push_back( part );
            else if( Name == "Gamma"   ) GammaListGen->push_back( part );
            else if( Name == "Tau"     ) TauListGen->push_back( part );
            else if( Name == (m_METType+"_gen") ) METListGen->push_back( part );
            else if( Name == m_JetAlgo ) JetListGen->push_back( part );
            else if( Name == genCollection) S3ListGen->push_back( part );
        }

    }
}

void specialAna::endEvent( const pxl::Event* event ){

   delete EleList;
   delete MuonList;
   delete GammaList;
   delete TauList;
   delete METList;
   delete JetList;

   EleList = 0;
   MuonList = 0;
   GammaList = 0;
   METList = 0;
   JetList = 0;
   TauList = 0;

   if( not runOnData ){

      delete EleListGen;
      delete MuonListGen;
      delete GammaListGen;
      delete METListGen;
      delete JetListGen;
      delete TauListGen;

      EleListGen = 0;
      MuonListGen = 0;
      GammaListGen = 0;
      METListGen = 0;
      JetListGen = 0;
      TauListGen = 0;
   }
}
