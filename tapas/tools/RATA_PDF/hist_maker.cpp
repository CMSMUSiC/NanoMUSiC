/** \file hist_maker.cpp
 *
 * \brief Functions to calculate the PDF uncertainties.
 * The functions will be called by tha RATA_PDF package.
 *
 */

#include "hist_maker.h"

#include "LHAPDF/LHAPDF.h"
#include "HistClass.hh"
#include <cmath>

using namespace std;
using namespace LHAPDF;
using namespace HistClass;

 //Command to export the Library path:
 //LD_LIBRARY_PATH=/usr/local/lib; export LD_LIBRARY_PATH
 //LD_LIBRARY_PATH=/home/serdweg/Documents/root/lib; export LD_LIBRARY_PATH
 //LD_LIBRARY_PATH=/home/serdweg/programs/RATA_PDF; export LD_LIBRARY_PATH

void raw_input(TString question)
{
    TString answer;
    cout << question << endl;
    cin >> answer;
}

TTree* read_tree(char* filename, char* tree_name, char* tree_cuts)
{
    DEBUG("now reading tree " << tree_name << " with cuts: " << tree_cuts << " from file: " << filename);
    TFile* file = new TFile(filename,"READ");
    TTree* tree = (TTree*)file -> Get(tree_name);
    TFile* fileTmp = new TFile("tmpFile_.root","RECREATE");
    TTree* smallerTree= tree->CopyTree(tree_cuts);
    file -> Close();
    delete file;
    return smallerTree;
}

double get_scalefactor(char* f_name, double lumi, double data_mc_scf, double xs)
{
    DEBUG("now calculating scale factor for " << f_name << " with lumi: " << lumi << ", data/MC scf: " << data_mc_scf << " and cross section: " << xs);
    TFile *file = new TFile(f_name,"READ");
    TH1F* hist;
    hist = (TH1F*)file->Get("h_counters");
    //double Nev = max(hist->GetBinContent(1),hist->GetBinContent(2));
    double Nev =   1;
    if(hist->GetBinContent(2)!=0){
        Nev = hist->GetBinContent(2);
    }else{
        Nev = hist->GetBinContent(1);
    }

    double scf = lumi * xs * data_mc_scf / Nev;
    return scf;
}

extern "C" void init_bg(char* PDFpath, int n_pdfs, char** PDFsets, int loglevel)
{
    gLogLevel = loglevel;
    INFO("PDFPath:   " << PDFpath);
    //try{
        string PDFPath_n = PDFpath;
        //char** PDFsets_n = PDFsets;
        //TString histname_n = histname;
        LHAPDF::setVerbosity(0);
        LHAPDF::setPDFPath(PDFPath_n);

        if(gLogLevel<2)cerr.setstate(std::ios::failbit);
        for( int i=0;i<n_pdfs;i++){
            vector< LHAPDF::PDF* > weightPdfSets = LHAPDF::mkPDFs(PDFsets[i]);
            allPdfsets.push_back(weightPdfSets);
            INFO("   " << PDFsets[i] << " mem: " << allPdfsets[i].size());
        }
        ///   reference PDF set which is used to produce the Monte Carlos
        //return true;
    //}
    //catch(...){
        //ERROR("could not init all stuff")
        ////return false;
    //}
}

bool analyser(TTree* tree, char** branches, char* histname, int n_pdfs, char** PDFsets, double scalefactor)
{
    try{
        TString histname_n = histname;

        float pdf_scale;
        float pdf_id1;
        float pdf_id2;
        float pdf_x1;
        float pdf_x2;
        float mt;
        float weight;

        TBranch* b_pdf_scale;
        TBranch* b_pdf_id1;
        TBranch* b_pdf_id2;
        TBranch* b_pdf_x1;
        TBranch* b_pdf_x2;
        TBranch* b_mt;
        TBranch* b_weight;

        tree->SetBranchAddress((TString)branches[0], &pdf_scale, &b_pdf_scale);
        tree->SetBranchAddress((TString)branches[1], &pdf_id1, &b_pdf_id1);
        tree->SetBranchAddress((TString)branches[2], &pdf_id2, &b_pdf_id2);
        tree->SetBranchAddress((TString)branches[3], &pdf_x1, &b_pdf_x1);
        tree->SetBranchAddress((TString)branches[4], &pdf_x2, &b_pdf_x2);
        tree->SetBranchAddress((TString)branches[5], &mt, &b_mt);
        tree->SetBranchAddress((TString)branches[6], &weight, &b_weight);

        Long64_t nentries = tree->GetEntries();
        INFO("Analysing " << nentries << " Events");
        for (Long64_t jentry=0; jentry<nentries;jentry++) {
            Long64_t ientry = tree->LoadTree(jentry);
            double nb = tree->GetEntry(jentry);
            //DEBUG("-----------  " << jentry);
            //if(jentry % 1000 == 0){
                //INFO("Event: " << jentry);
                //gSystem->GetProcInfo(&info);
                //INFO(" -> Memory in MB : " << info.fMemResident/1000. << " (resident) " << info.fMemVirtual/1000. << " (virtual) ");
            //}
            //DEBUG(" -: " << mt << "\t" << weight << "\t" << scalefactor);
            //Fill(0,histname,mt,weight*scalefactor);

            double ori;
            ori = Mainpdf->xfxQ(pdf_id1,pdf_x1,pdf_scale)*Mainpdf->xfxQ(pdf_id2,pdf_x2,pdf_scale);
            //DEBUG("ori: " << ori);
            for( int i=0;i<n_pdfs;i++){
                for (int cpdf=0; cpdf < allPdfsets[i].size(); cpdf++) {
                    double pdfweight = allPdfsets[i][cpdf]->xfxQ(pdf_id1,pdf_x1,pdf_scale)*allPdfsets[i][cpdf]->xfxQ(pdf_id2,pdf_x2,pdf_scale)/ori;
                    pdfweight*=weight*scalefactor;
                    //DEBUG(PDFsets[i] << "\t" << cpdf << "\t" << pdfweight);
                    //do not use if the weight is inf!!
                    if(std::isfinite(pdfweight)){
                        Fill(cpdf,histname_n+"_"+PDFsets[i],mt,pdfweight);
                    }else{
                        DEBUG(PDFsets[i] << "\t" << cpdf << "\t" << pdfweight);
                        INFO("This probably means the ref pdf set is wrong!!!");
                        exit(20);
                    }
                }
            }
        }
        return true;
    }
    catch(...){
        ERROR("could not analyse all events")
        return false;
    }
}

bool writer(char* histname, int n_pdfs, char** PDFsets)
{
    try{
        TString histname_n = histname;
        Write(0,histname);

        for( int i=0;i<n_pdfs;i++){
            for (int cpdf=0; cpdf < allPdfsets[i].size(); cpdf++) {
                SetToZero(cpdf,histname_n+"_"+PDFsets[i]);
                Write(cpdf,histname_n+"_"+PDFsets[i]);
            }
        }
        return true;
    }
    catch(...){
        ERROR("could not write output files")
        return false;
    }
}

extern "C" void make_hists(char* input_file, char* tree_name, char* tree_cuts, char** branches, double lumi, double cross_section, int n_pdfs, char** PDFsets, char* PDFnorm, char* histname, char* output_file, int n_binning, double* binning)
{
    // process and timing information
    TStopwatch timer;
    timer.Start();

    // get date and time
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime ( &rawtime );

    // user greeting
    INFO("");
    INFO("######################################################################");
    INFO("#                                                                    #");
    INFO("# Start executing analyzer at " << asctime(timeinfo)                   );
    INFO("#                                                                    #");
    INFO("######################################################################");
    INFO("");

    //gLogLevel = 4;

    INFO("");
    INFO("-----------------------------------------------------------------");
    DEBUG("test");
    INFO("");
    INFO("input file:   " << input_file);
    INFO("tree name:    " << tree_name);
    INFO("tree cuts:    " << tree_cuts);
    for(int i = 0; i <7; i++){
        INFO("\t" << branches[i]);
    }
    INFO("lumi:  " << lumi);
    INFO("cross_section:  " << cross_section);
    INFO("n_pdfs:   " << n_pdfs);
    for(int i = 0; i <n_pdfs; i++){
        INFO("\t" << PDFsets[i]);
    }
    INFO("Norm PDF:  " << PDFnorm);
    INFO("histname:  " << histname);
    INFO("output file: " << output_file);
    INFO("n bins:    " << n_binning);
    for(int i = 0; i <n_binning; i++){
        INFO("\t" << binning[i]);
    }
    INFO("");
    INFO("-----------------------------------------------------------------");
    INFO("");

    INFO("start of program");
    gSystem->GetProcInfo(&info);
    INFO(" -> Memory in MB : " << info.fMemResident/1000. << " (resident) " << info.fMemVirtual/1000. << " (virtual) ");

    TFile* f1 = new TFile(output_file,"RECREATE");

    char** PDFsets_n = PDFsets;
    TString histname_n = histname;

    Mainpdf = LHAPDF::mkPDF(PDFnorm,0);
    if(gLogLevel<2)cerr.clear();
    DEBUG("All PDF sets initialized!");

    CreateHisto(1,histname,8000,0,8000,"M_{T}  [GeV]");
    DEBUG("Created main histogram!"<<" "<<histname<<" "<<n_binning-1);
    RebinHisto(1,histname,n_binning-1,binning);
    DEBUG("Rebinned main histogram!");
    for( int i=0;i<n_pdfs;i++){
        CreateHisto(allPdfsets[i].size()+1,histname_n+"_"+PDFsets_n[i],8000,0,8000,"M_{T}  [GeV]");
        DEBUG("Created " << PDFsets_n[i] << " histogram!");
        RebinHisto(allPdfsets[i].size()+1,histname_n+"_"+PDFsets_n[i],n_binning-1,binning);
        DEBUG("Rebinned " << PDFsets_n[i] << " histogram!");
    }
    DEBUG("controll: All initialization done!");
    bool b_init = true;

    INFO("init done");
    INFO("start of loop");
    gSystem->GetProcInfo(&info);
    INFO(" -> Memory in MB : " << info.fMemResident/1000. << " (resident) " << info.fMemVirtual/1000. << " (virtual) ");

    bool b_ana  = analyser(read_tree(input_file,tree_name,tree_cuts),branches,histname,n_pdfs,PDFsets,get_scalefactor(input_file,lumi,1.0,cross_section));

    INFO("loop done");
    INFO("start to write")
    gSystem->GetProcInfo(&info);
    INFO(" -> Memory in MB : " << info.fMemResident/1000. << " (resident) " << info.fMemVirtual/1000. << " (virtual) ");

    f1->cd();
    bool b_writ = writer(histname,n_pdfs,PDFsets);

    INFO("writing done")
    INFO("start deleting")
    gSystem->GetProcInfo(&info);
    INFO(" -> Memory in MB : " << info.fMemResident/1000. << " (resident) " << info.fMemVirtual/1000. << " (virtual) ");

    delete Mainpdf;
    DeleteHisto(1,histname);
    for( int i=0;i<n_pdfs;i++){
        DeleteHisto(allPdfsets[i].size()+1,histname_n+"_"+PDFsets_n[i]);
    }

    INFO("deleting done")
    INFO("start closing files")
    gSystem->GetProcInfo(&info);
    INFO(" -> Memory in MB : " << info.fMemResident/1000. << " (resident) " << info.fMemVirtual/1000. << " (virtual) ");

    f1 -> Close();
    delete f1;

    timer.Stop();
    gSystem->GetProcInfo(&info);
    INFO("");
    INFO("real time (s)     : " << timer.RealTime());
    INFO("CPU time (s)      : " << timer.CpuTime());
    INFO("resident mem (MB) : " << info.fMemResident/1000.);
    INFO("virtual  mem (MB) : " << info.fMemVirtual/1000.);
    INFO("");

    // info at end of program
    time(&rawtime);
    timeinfo = localtime ( &rawtime );
    INFO("");
    INFO("######################################################################");
    INFO("#                                                                    #");
    INFO("# End executing analyzer at " << asctime(timeinfo)                     );
    INFO("#                                                                    #");
    INFO("######################################################################");
    INFO("");

    //if(b_init && b_ana && b_writ){
        //return true;
    //}else{
        //return false;
    //}
}

vector< vector<Float_t> > hessian_pdf_asErr(vector< vector< TH1D* > > hist_scaled, double pdf_correction, double as_plus_correction, double as_minus_correction, int* as_plus_number, int* as_minus_number)
{
    Float_t S_as_plus[hist_scaled[0][0]->GetNbinsX()+1];
    Float_t S_as_minus[hist_scaled[0][0]->GetNbinsX()+1];

    vector< vector<Float_t> > v_as_errors;
    vector<Float_t> v_as_error_plus;
    vector<Float_t> v_as_error_minus;
    vector<Float_t> v_as_mean;
    vector<Float_t> v_as_mean_err;

    //tmpVariables:
    double CorrectionFactorPDF=pdf_correction;
    int numberOfErrorHist=(hist_scaled[0].size()-1)/2;

    for (int bin=1; bin<=hist_scaled[0][0]->GetNbinsX(); bin++) {
        S_as_plus[bin] = 0.;
        S_as_minus[bin] = 0.;

        float pdf_plus = 0;
        float pdf_minus = 0;

        double baseValue = hist_scaled[0][0]->GetBinContent(bin);

        for(int i = 1; i < numberOfErrorHist; i++){
            double up   = hist_scaled[0][2*i-1]->GetBinContent(bin);
            double down = hist_scaled[0][2*i]->GetBinContent(bin);
            pdf_plus += pow(TMath::Max(TMath::Max(up-baseValue, down-baseValue),0.),2.);
            pdf_minus += pow(TMath::Max(TMath::Max(baseValue-up,baseValue-down),0.),2.);
        }
        pdf_plus = (1./CorrectionFactorPDF)*sqrt(pdf_plus);
        pdf_minus = (1./CorrectionFactorPDF)*sqrt(pdf_minus);

        float as_plus = 0;
        float as_minus = 0;

        as_plus = (1./as_plus_correction)*(hist_scaled[as_plus_number[0]][as_plus_number[1]]->GetBinContent(bin) - baseValue);
        as_minus = (1./as_minus_correction)*(hist_scaled[as_minus_number[0]][as_minus_number[1]]->GetBinContent(bin) - baseValue);

        S_as_plus[bin] = sqrt(pow(pdf_plus,2) + pow(as_plus,2));
        S_as_minus[bin] = sqrt(pow(pdf_minus,2) + pow(as_minus,2));

        v_as_error_plus.push_back(S_as_plus[bin]);
        v_as_error_minus.push_back(S_as_minus[bin]);
        v_as_mean.push_back(baseValue);
        v_as_mean_err.push_back(hist_scaled[0][0]->GetBinError(bin));
    }

    v_as_errors.push_back(v_as_error_plus);
    v_as_errors.push_back(v_as_error_minus);
    v_as_errors.push_back(v_as_mean);
    v_as_errors.push_back(v_as_mean_err);

    return v_as_errors;
}

extern "C" void pdf_calcer_hessian(int n_pdfSet, char** pdf_sets, char* outfile, char* out_par, char* infile, char* main_hist, char* shortname, double pdf_correction, double as_plus_correction, double as_minus_correction, int* as_plus_number, int* as_minus_number)
{
    vector< vector<TH1D*> > h1_PDF_scaled;

    vector<string> histos;
    for(int i = 0; i < n_pdfSet; i++){
        histos.push_back(pdf_sets[i]);
    }

    TString histname_n = main_hist;
    TFile* Tinfile = new TFile(infile,"READ");
    TH1D* h1_scaled_mt;
    for(int i = 0; i < n_pdfSet; i++){
        TIter nextkey( Tinfile->GetListOfKeys() );
        TKey *key;
        vector< TH1D* > temphist;
        while ( (key = (TKey*)nextkey()) ) {
            Tinfile->cd();
            TObject *myobj = key->ReadObj();
            if ( myobj->InheritsFrom( "TH1" )) {
                TString histname(myobj->GetName());
                if (histname.EndsWith(histname_n+"_"+histos[i])) {
                    TH1D* hist = (TH1D*) myobj->Clone(myobj->GetName());
                    //hist -> Rebin(1);
                    temphist.push_back(hist);
                }
                if (histname.EndsWith(histname_n) && i == 0) {
                    TH1D* hist = (TH1D*) myobj->Clone(myobj->GetName());
                    //hist -> Rebin(1);
                    h1_scaled_mt = hist;
                }
            }
        }
        h1_PDF_scaled.push_back(temphist);
    }
    /// Calculate uncertainty envelopes for CTEQ & MSTW, both with the scaling factors (C_90, C_59, C_79) and without

    TH1D* h1_err_0;
    TH1D* h1_err_1;                      /// for upper & lower uncertainty histograms
    TH1D* h1_mean;
    int nBins=h1_scaled_mt->GetNbinsX();
    float xLow=h1_scaled_mt->GetBinLowEdge(1);
    float xHigh=h1_scaled_mt->GetBinLowEdge(h1_scaled_mt->GetNbinsX()+1);
    TString n_shortname = shortname;

    h1_err_0 = (TH1D*) h1_scaled_mt-> Clone(n_shortname+"_up");
    h1_err_1 = (TH1D*) h1_scaled_mt-> Clone(n_shortname+"_down");
    h1_mean = (TH1D*) h1_scaled_mt-> Clone(n_shortname+"_mean");

    vector< vector<Float_t> > errors;
    vector<Float_t>  error_plus;
    vector<Float_t>  error_minus;
    vector<Float_t>  mean;
    vector<Float_t>  mean_err;

    errors = hessian_pdf_asErr(h1_PDF_scaled,pdf_correction,as_plus_correction,as_minus_correction,as_plus_number,as_minus_number);
    for (int i=1; i<=h1_err_0->GetNbinsX(); i++) {
        error_plus.push_back(errors.at(0).at(i-1));
        error_minus.push_back(errors.at(1).at(i-1));
        mean.push_back(errors.at(2).at(i-1));
        mean_err.push_back(errors.at(3).at(i-1));

        h1_err_0 -> SetBinContent(i, TMath::Max(mean[i-1] + error_plus[i-1],(float) 0.));
        h1_err_1 -> SetBinContent(i, TMath::Max(mean[i-1] - error_minus[i-1],(float) 0.));
        h1_mean -> SetBinContent(i,mean[i-1]);

        h1_err_0 -> SetBinError(i, mean_err[i-1]);
        h1_err_1 -> SetBinError(i,mean_err[i-1]);

        h1_mean -> SetBinError(i,mean_err[i-1]);
    }

    TFile* Toutfile = new TFile(outfile,out_par);
    Toutfile -> cd();
    //Toutfile->mkdir("pdf");
    //Toutfile -> cd("pdf/");
    h1_err_0 -> Write();
    h1_err_1 -> Write();
    h1_mean -> Write();

    Toutfile -> Close();
    Tinfile -> Close();
    //return true;
}

vector< vector<Float_t> > NNPDF_weighted_mean( vector< vector< TH1D* > > hist_scaled)
{
/// Validate the number of replicas used from each set
/// The number of replicas that are to be used from each set, (0,4,25,71,100,71,25,4,0) as mentioned in the PDF4LHC Interim Report, are assumed to be
/// gaussianly distributed around a central value of alpha_s = 0.119. The following part of code varrifies the choosen number of replicas.

/// Combined PDF & alpha_s weighted mean value & standard deviation = PDF uncertainty
/// Instead of randomly selecting which of the 100 replicas from each of the 7 sets to use, the following code uses all replicas from all sets
/// (=100*7=700 replicas) and estimates a weighted mean. The weights are set to suit the gaussian assumption made above.
    int BinNumber=hist_scaled[0][0]->GetNbinsX();
    double replica_weights[7];
    if(hist_scaled.size()==7){
        replica_weights[0] = 4.;            /// alpha_s = 0.116 ->  file [5]
        replica_weights[1] = 25.;           /// alpha_s = 0.117 ->  file [6]
        replica_weights[2] = 71.;           /// alpha_s = 0.118 ->  file [7]
        replica_weights[3] = 100.;          /// alpha_s = 0.119 ->  file [8]
        replica_weights[4] = 71.;           /// alpha_s = 0.120 ->  file [9]
        replica_weights[5] = 25.;           /// alpha_s = 0.121 ->  file [10]
        replica_weights[6] = 4.;            /// alpha_s = 0.122 ->  file [11]
    }else if(hist_scaled.size()==5){
        //replica_weights[0] = 4.;            /// alpha_s = 0.116 ->  file [5]
        replica_weights[0] = 25.;           /// alpha_s = 0.117 ->  file [6]
        replica_weights[1] = 71.;           /// alpha_s = 0.118 ->  file [7]
        replica_weights[2] = 100.;          /// alpha_s = 0.119 ->  file [8]
        replica_weights[3] = 71.;           /// alpha_s = 0.120 ->  file [9]
        replica_weights[4] = 25.;           /// alpha_s = 0.121 ->  file [10]
        //replica_weights[6] = 4.;            /// alpha_s = 0.122 ->  file [11]
    }

    float events_nnpdf_[7][100];          ///[replicas][bins]
    //Variables used in the loop
    float sum_events_nnpdf_;           ///
    float sum_diffsq_events_nnpdf_;
    float f_sum_events = 0;
    float f_sum_replica_weights = 0;
    float f_nnpdf_mean =0;
    float f_sum_diffsq_events = 0.;

    vector<float> v_percent_nnpdf_stddev;
    vector<float> v_percent_nnpdf_mean;
    vector<float> v_percent_nnpdf_mean_err;
    vector< vector<Float_t> > v_nnpdf_error_and_mean;

    for (int bin=1; bin <=BinNumber; bin++) {
        f_sum_events = 0;
        f_sum_replica_weights = 0;
        for (int n=0; n<hist_scaled.size(); n++){                                                             /// sum over all 7 sets
            sum_events_nnpdf_ = 0.;
            for (int i=0; i<100; i++) {                                                         /// sum over all 100 replicas in one set (1 to 100)
                events_nnpdf_[n][i] = hist_scaled[n][i+1]->GetBinContent(bin);
                sum_events_nnpdf_ += replica_weights[n] * hist_scaled[n][i+1]->GetBinContent(bin);
            }
            f_sum_replica_weights += (replica_weights[n]*100);/// should be 300*100=30000 in this case
//             f_sum_replica_weights*=100;
            f_sum_events += sum_events_nnpdf_;
        }

        /// --> weighted mean
        f_nnpdf_mean = f_sum_events / (f_sum_replica_weights);
        f_sum_diffsq_events = 0.;
        for (int n=0; n<hist_scaled.size(); n++) {
            sum_diffsq_events_nnpdf_ = 0.;
            for (int i=0; i<100; i++) {
                sum_diffsq_events_nnpdf_ += replica_weights[n] * pow(events_nnpdf_[n][i] - f_nnpdf_mean,2);
            }
            f_sum_diffsq_events += sum_diffsq_events_nnpdf_;
        }

        /// --> standard deviation
        f_sum_diffsq_events = sqrt( f_sum_diffsq_events / (f_sum_replica_weights));
        v_percent_nnpdf_stddev.push_back(f_sum_diffsq_events);
        v_percent_nnpdf_mean.push_back(f_nnpdf_mean);
        if(hist_scaled.size()==7){
            v_percent_nnpdf_mean_err.push_back(hist_scaled[3][0]->GetBinError(bin));
        }else if(hist_scaled.size()==5){
            v_percent_nnpdf_mean_err.push_back(hist_scaled[2][0]->GetBinError(bin));
        }
    }
    v_nnpdf_error_and_mean.push_back(v_percent_nnpdf_stddev);
    v_nnpdf_error_and_mean.push_back(v_percent_nnpdf_mean);
    v_nnpdf_error_and_mean.push_back(v_percent_nnpdf_mean_err);
    return v_nnpdf_error_and_mean;
}

extern "C" void pdf_calcer_MC(int n_pdfSet, char** pdf_sets, char* outfile, char* out_par, char* infile, char* main_hist, char* shortname)
{
    vector< vector<TH1D*> > h1_PDF_scaled;

    vector<string> histos;
    for(int i = 0; i < n_pdfSet; i++){
        histos.push_back(pdf_sets[i]);
    }
    TString histname_n = main_hist;
    TFile* Tinfile = new TFile(infile,"READ");
    TH1D* h1_scaled_mt;
    for(int i = 0; i < n_pdfSet; i++){
        TIter nextkey( Tinfile->GetListOfKeys() );
        TKey *key;
        vector< TH1D* > temphist;
        while ( (key = (TKey*)nextkey()) ) {
            Tinfile->cd();
            TObject *myobj = key->ReadObj();
            if ( myobj->InheritsFrom( "TH1" )) {
                TString histname(myobj->GetName());
                if (histname.EndsWith(histname_n+"_"+histos[i])) {
                    TH1D* hist = (TH1D*) myobj->Clone(myobj->GetName());
                    hist -> Rebin(1);
                    temphist.push_back(hist);
                }
                if (histname.EndsWith(histname_n) && i == 0) {
                    TH1D* hist = (TH1D*) myobj->Clone(myobj->GetName());
                    hist -> Rebin(1);
                    h1_scaled_mt = hist;
                }
            }
        }
        h1_PDF_scaled.push_back(temphist);
    }

    TH1D* h1_err_0;
    TH1D* h1_err_1;                      /// for upper & lower uncertainty histograms
    TH1D* h1_mean;
    int nBins=h1_scaled_mt->GetNbinsX();
    float xLow=h1_scaled_mt->GetBinLowEdge(1);
    float xHigh=h1_scaled_mt->GetBinLowEdge(h1_scaled_mt->GetNbinsX()+1);
    TString n_shortname = shortname;

    h1_err_0 = (TH1D*) h1_scaled_mt-> Clone(n_shortname+"_up");
    h1_err_1 = (TH1D*) h1_scaled_mt-> Clone(n_shortname+"_down");
    h1_mean = (TH1D*) h1_scaled_mt-> Clone(n_shortname+"_mean");

    vector<float> NNPDF_error;
    vector<float> NNPDF_mean;
    vector<float> NNPDF_mean_error;
    vector< vector<Float_t> > nnpdf_error_and_mean;
    nnpdf_error_and_mean= NNPDF_weighted_mean(h1_PDF_scaled);
    NNPDF_error = nnpdf_error_and_mean.at(0);
    NNPDF_mean = nnpdf_error_and_mean.at(1);
    NNPDF_mean_error = nnpdf_error_and_mean.at(2);

    for (int i=1; i<=h1_err_0->GetNbinsX(); i++) {
        h1_err_0 -> SetBinContent(i, NNPDF_mean[i-1] + NNPDF_error[i-1]);
        h1_err_1 -> SetBinContent(i, NNPDF_mean[i-1] - NNPDF_error[i-1]);
        h1_mean -> SetBinContent(i, NNPDF_mean[i-1]);
        h1_err_0 -> SetBinError(i, NNPDF_mean_error[i-1]);
        h1_err_1 -> SetBinError(i, NNPDF_mean_error[i-1]);
        h1_mean -> SetBinError(i, NNPDF_mean_error[i-1]);
    }

    TFile* Toutfile = new TFile(outfile,out_par);
    Toutfile -> cd();
    //Toutfile->mkdir("pdf");
    //Toutfile -> cd("pdf/");
    h1_err_0 -> Write();
    h1_err_1 -> Write();
    h1_mean -> Write();

    Toutfile -> Close();
    Tinfile -> Close();
    //return true;
}

/// ////////////////////////////////////////////////////
/// from here on: only test functions, that
///               are not necessary for the
///               PDF uncertainties

extern "C" void test_TString(TString test)
{
	cout << test << endl;
}

extern "C" void test_string(string test)
{
	cout << test << endl;
}

extern "C" void test_double(double test)
{
	cout << test << endl;
}

extern "C" void test_double_array(int size, double* test)
{
	cout << size << "\t";
	for(int i= 0; i < size; i++){
		cout << test[i] <<  "\t";
	}
	cout << endl;
}

extern "C" void test_int(int test)
{
	cout << test << endl;
}

extern "C" void test_char(char* test)
{
	cout << test << endl;
}

extern "C" void test_string_array(int size, char** test)
{
	cout << size << "\t";
	for(int i= 0; i < size; i++){
		cout << test[i] <<  "\t";
	}
	cout << endl;
}
