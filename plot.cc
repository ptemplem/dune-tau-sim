#include <fstream>
#include "TH1.h"
#include "TH2.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TChain.h"
#include "THStack.h"
#include "TLegend.h"
#include "TGaxis.h"
#include "TStyle.h"
#include "tree/dk2nu.h"
#include "tree/dk2nu.cc"
#include "tree/dkmeta.h"
#include "tree/calcLocationWeights.h"
#include "tree/calcLocationWeights.cxx"

double angle(bsim::Dk2Nu* dk2nu){
  double px = dk2nu->nuray[0].px;
  double py = dk2nu->nuray[0].py;
  double pz = dk2nu->nuray[0].pz;
  double pt = pow(px*px+py*py,0.5);
  return std::atan2(pt,pz);
}
class dat_xsec {
  public:
    double arr_e[1500][2];
    double arr_xsec[1500][2];   
    double GetXSec(double e, int nu){
      int i=1;
      while (arr_e[i][nu]<e){
        i++;
      }
      return arr_xsec[i-1][nu];
    }  
};
void set_style(){
  TStyle* tauStyle = new TStyle("tauStyle","dune-tau-sim style");
  tauStyle->SetPalette(1,0); // avoid horrible default color scheme
  tauStyle->SetOptStat(0);
  tauStyle->SetLabelSize(0.03,"xyz"); // size of axis value font
  tauStyle->SetTitleSize(0.06,"xyz");
  tauStyle->SetTitleFont(22,"xyz"); // font option
  tauStyle->SetLabelFont(22,"xyz");
  tauStyle->SetTitleOffset(0.6,"x");
  tauStyle->SetTitleOffset(0.8,"y");
  tauStyle->SetTitleX(0.5);
  tauStyle->SetTitleAlign(23);
  gROOT->SetStyle("tauStyle");
  TGaxis::SetMaxDigits(3);
  return;
}
void set_axis_style(TH2D* hist){
  hist->GetXaxis()->SetLabelFont(22);
  hist->GetXaxis()->SetLabelSize(0.03);
  hist->GetXaxis()->SetTitleFont(22);
  hist->GetXaxis()->SetTitleSize(0.06);
  hist->GetXaxis()->SetTitleOffset(0.6);
  hist->GetYaxis()->SetLabelFont(22);
  hist->GetYaxis()->SetLabelSize(0.03);
  hist->GetYaxis()->SetTitleFont(22);
  hist->GetYaxis()->SetTitleSize(0.06);
  hist->GetYaxis()->SetTitleOffset(0.8);
}


int main(int argc, char* argv[]){
    TChain *t = new TChain("dk2nuTree");
    TChain *tmeta = new TChain("dkmetaTree");
    TChain *t2 = new TChain("dk2nuTree");
    TChain *t_iso = new TChain("dk2nuTree");
    TChain *t_p = new TChain("dk2nuTree");
    TChain *tmeta_p = new TChain("dkmetaTree");
    long pot = 0;
    for (int iFile = 1; iFile < argc - 1; ++iFile) {
	TFile *f_tmp = new TFile(argv[iFile]);
        TTree* tmeta_tmp=(TTree*)f_tmp->Get("dkmetaTree");
        bsim::DkMeta* dkmeta_tmp = new bsim::DkMeta;
        tmeta_tmp->SetBranchAddress("dkmeta",&dkmeta_tmp);
        tmeta_tmp->GetEntry(0);
        pot = pot + dkmeta_tmp->pots;
        dkmeta_tmp->clear();
        tmeta_tmp->Delete();
        f_tmp->Close();
        t->Add(argv[iFile]);
        t_iso->Add(argv[iFile]);
        tmeta->Add(argv[iFile]);
    }
    // Trees for PYTHIA nu_e, nu_mu
    TFile *f_tmp = new TFile(argv[argc-1]);
    TTree* tmeta_tmp=(TTree*)f_tmp->Get("dkmetaTree");
    bsim::DkMeta* dkmeta_tmp = new bsim::DkMeta;
    tmeta_tmp->SetBranchAddress("dkmeta",&dkmeta_tmp);
    tmeta_tmp->GetEntry(0);
    long pot_p = dkmeta_tmp->pots;
    dkmeta_tmp->clear();
    tmeta_tmp->Delete();
    f_tmp->Close();
    t_p->Add(argv[argc-1]);
    tmeta_p->Add(argv[argc-1]);
    // Copy chains for nonpolarized
    std::string pattern = "/pnfs/dune/persistent/physicsgroups/dunebeam/ikotler/DUNE_PRISM/OnAxis/neutrino/dk2nu/";
    pattern = pattern + "g4lbne_v3r5p10_QGSP_BERT_OfficialEngDesignSept2021_OnAxis_neutrino_0000*.dk2nu.root";
    t2->Add(pattern.c_str());

    // Event rate coefficients
    std::vector<double> det_mass = {0,67e-3,40}; // mass in kton
    double e_coeff = 1e-39*1e6/1.66e-27*1.1e21; // xsec_exponent * kg/kton / nucleon_mass / POT
    double tau_event[3] = {0,0,0};
    double taubar_event[3] = {0,0,0};
    double flux;
    double event;

    // Read xsec files
    std::string base = getenv("G4LBNEWORKDIR");
    base = base + "/ProductionScripts/data/argon_genie2.8.4/";
    std::string f_xsec[2];
    f_xsec[0] = base + "xsec_cc_nutau.dat";
    f_xsec[1] = base + "xsec_cc_nutaubar.dat";
    dat_xsec dat;
    std::ifstream fdat_file[2];
    for (int i=0; i<2; i++) {
      fdat_file[i].open(f_xsec[i].c_str());
      std::cout << " Opened "<< f_xsec[i] << "."<< std::endl;
      double row[2];
      int fnlines = 0;
      while ( fdat_file[i] >> row[0] >> row[1] ) {
          dat.arr_e[fnlines][i] = row[0];
          dat.arr_xsec[fnlines][i] = row[1];
          fnlines++;
      }
      fdat_file[i].close();
    }

    int pot2 = 1e7; //1e6 POT per files (10 files)
    bsim::Dk2Nu* dk2nu = new bsim::Dk2Nu;
    bsim::DkMeta* dkmeta = new bsim::DkMeta;
    bsim::Dk2Nu* dk2nu2 = new bsim::Dk2Nu;
    bsim::Dk2Nu* dk2nu_p = new bsim::Dk2Nu;
    bsim::DkMeta* dkmeta_p = new bsim::DkMeta;
    t->SetBranchAddress("dk2nu", &dk2nu);
    tmeta->SetBranchAddress("dkmeta", &dkmeta);
    t2->SetBranchAddress("dk2nu", &dk2nu2);
    t_p->SetBranchAddress("dk2nu", &dk2nu_p);
    tmeta_p->SetBranchAddress("dkmeta", &dkmeta_p);
    bsim::Dk2Nu* dk_iso = new bsim::Dk2Nu;
    t_iso->SetBranchAddress("dk2nu",&dk_iso);
    
    tmeta->GetEntry(0);
    int nL = dkmeta->location.size();
    int nP = t->GetEntries();
    int nP2 = t2->GetEntries();
    int nP_p = t_p->GetEntries();
    std::cout << "Analyzing " << pot << " tau neutrino pot and " << pot2 << " other pot." << std::endl; 
    const double pi = 3.14159;
    const double area = pi * 100 * 100; // 100 cm radius
    double ang;
    std::vector<double> tau_flux = {0,0,0};
    std::vector<double> taubar_flux = {0,0,0};

    // Setup Histograms
    TCanvas* c = new TCanvas("c","c",800,600);
    c->SetRightMargin(0.15);
    TH1D** h_nue = new TH1D*[nL];
    TH1D** h_nuebar = new TH1D*[nL];
    TH1D** h_numu = new TH1D*[nL];
    TH1D** h_numubar = new TH1D*[nL];
    TH1D** h_nue_pyth = new TH1D*[nL];
    TH1D** h_nuebar_pyth = new TH1D*[nL];
    TH1D** h_numu_pyth = new TH1D*[nL];
    TH1D** h_numubar_pyth = new TH1D*[nL];
    TH2D** h_nutau2d = new TH2D*[nL];
    TH2D** h_nutaubar2d = new TH2D*[nL];
    TH1D** h_nutau = new TH1D*[nL];
    TH1D** h_nutaubar = new TH1D*[nL];
    TH1D** h_nutau_iso = new TH1D*[nL];
    TH1D** h_nutaubar_iso = new TH1D*[nL];
    TH1D** h_nutau_ds = new TH1D*[nL];
    TH1D** h_nutaubar_ds = new TH1D*[nL];
    TH1D** h_nutau_tau = new TH1D*[nL];
    TH1D** h_nutaubar_tau = new TH1D*[nL];
    TH1D** h_nutau_event = new TH1D*[nL];
    TH1D** h_nutaubar_event = new TH1D*[nL];
    TLegend** leg = new TLegend*[nL];
    TLegend** leg_pol = new TLegend*[nL];
    TLegend** leg_br = new TLegend*[nL];
    TLegend** leg_event = new TLegend*[nL];
    TLegend** leg_pyth = new TLegend*[nL];
    double max = 20;
    std::vector<std::string> title {"Random Decay","DUNE Near Detector","DUNE Far Detector"};
    for (int iL=0;iL<nL;iL++) {
	std::string title_2d = title[iL] + std::string("; Neutrino Energy [GeV]; Angle [rad]");
        h_nue[iL] = new TH1D(("h_nue"+std::to_string(iL)).c_str(),"",60,0,max);
        h_nuebar[iL] = new TH1D(("h_nuebar"+std::to_string(iL)).c_str(),"",60,0,max);
        h_numu[iL] = new TH1D(("h_numu"+std::to_string(iL)).c_str(),"",60,0,max);
        h_numubar[iL] = new TH1D(("h_numubar"+std::to_string(iL)).c_str(),"",60,0,max);
        h_nue_pyth[iL] = new TH1D(("h_nue_pyth"+std::to_string(iL)).c_str(),"",60,0,max);
        h_nuebar_pyth[iL] = new TH1D(("h_nuebar_pyth"+std::to_string(iL)).c_str(),"",60,0,max);
        h_numu_pyth[iL] = new TH1D(("h_numu_pyth"+std::to_string(iL)).c_str(),"",60,0,max);
        h_numubar_pyth[iL] = new TH1D(("h_numubar_pyth"+std::to_string(iL)).c_str(),"",60,0,max);
        h_nutau[iL] = new TH1D(("h_nutau"+std::to_string(iL)).c_str(),"",60,0,max);
        h_nutaubar[iL] = new TH1D(("h_nutaubar"+std::to_string(iL)).c_str(),"",60,0,max);
        h_nutau2d[iL] = new TH2D(("h_nutau2d"+std::to_string(iL)).c_str(),title_2d.c_str(),60,0,max,150,0,pi);
        h_nutaubar2d[iL] = new TH2D(("h_nutaubar2d"+std::to_string(iL)).c_str(),title_2d.c_str(),60,0,max,150,0,pi);
        h_nutau_iso[iL] = new TH1D(("h_nutau_iso"+std::to_string(iL)).c_str(),"",60,0,max);
        h_nutaubar_iso[iL] = new TH1D(("h_nutaubar_iso"+std::to_string(iL)).c_str(),"",60,0,max);
        h_nutau_tau[iL] = new TH1D(("h_nutau_tau"+std::to_string(iL)).c_str(),"",60,0,max);
        h_nutaubar_tau[iL] = new TH1D(("h_nutaubar_tau"+std::to_string(iL)).c_str(),"",60,0,max);
        h_nutau_ds[iL] = new TH1D(("h_nutau_ds"+std::to_string(iL)).c_str(),"",60,0,max);
        h_nutaubar_ds[iL] = new TH1D(("h_nutaubar_ds"+std::to_string(iL)).c_str(),"",60,0,max);
        h_nutau_event[iL] = new TH1D(("h_nutau_event"+std::to_string(iL)).c_str(),"",60,0,60);
        h_nutaubar_event[iL] = new TH1D(("h_nutaubar_event"+std::to_string(iL)).c_str(),"",60,0,60);
        h_nue[iL]->SetLineColor(2);
        h_nuebar[iL]->SetLineColor(95);
        h_numu[iL]->SetLineColor(4);
        h_numubar[iL]->SetLineColor(3);
        h_nue_pyth[iL]->SetLineColor(2);
        h_nuebar_pyth[iL]->SetLineColor(95);
        h_numu_pyth[iL]->SetLineColor(4);
        h_numubar_pyth[iL]->SetLineColor(3);
        h_nutau[iL]->SetLineColor(6);
        h_nutaubar[iL]->SetLineColor(7);
        h_nutau_iso[iL]->SetLineColor(2);
        h_nutaubar_iso[iL]->SetLineColor(95);
        h_nutau_tau[iL]->SetLineColor(2);
        h_nutaubar_tau[iL]->SetLineColor(95);
        h_nutau_ds[iL]->SetLineColor(4);
        h_nutaubar_ds[iL]->SetLineColor(3);
	h_nutau_event[iL]->SetLineColor(2);
        h_nutaubar_event[iL]->SetLineColor(4);
        h_nue[iL]->Scale(1,"width");
        h_nuebar[iL]->Scale(1,"width");
        h_numu[iL]->Scale(1,"width");
        h_numubar[iL]->Scale(1,"width");
        h_nue_pyth[iL]->Scale(1,"width");
        h_nuebar_pyth[iL]->Scale(1,"width");
        h_numu_pyth[iL]->Scale(1,"width");
        h_numubar_pyth[iL]->Scale(1,"width");
        h_nutau[iL]->Scale(1,"width");
        h_nutaubar[iL]->Scale(1,"width");
        h_nutau_iso[iL]->Scale(1,"width");
        h_nutaubar_iso[iL]->Scale(1,"width");
        h_nutau_tau[iL]->Scale(1,"width");
        h_nutaubar_tau[iL]->Scale(1,"width");
        h_nutau_ds[iL]->Scale(1,"width");
        h_nutaubar_ds[iL]->Scale(1,"width");
        h_nutau_event[iL]->Scale(1,"width");
        h_nutaubar_event[iL]->Scale(1,"width");
        h_nutau2d[iL]->SetStats(0);
        h_nutaubar2d[iL]->SetStats(0);

        // Create Legends
        if (iL == 0) {
          leg[iL] = new TLegend(0.6,0.6,0.8,0.8);
	  leg_pyth[iL] = new TLegend(0.6,0.6,0.8,0.8);
        }
        else {
          leg[iL] = new TLegend(0.2,0.13,0.4,0.33);
          leg_pyth[iL] = new TLegend(0.2,0.13,0.4,0.33);
        }
        leg_pol[iL] = new TLegend(0.5,0.6,0.7,0.8);
        leg_br[iL] = new TLegend(0.15,0.15,0.45,0.35);
        leg_event[iL] = new TLegend(0.6,0.6,0.8,0.8);

        leg[iL]->SetNColumns(2);
        leg[iL]->SetTextSize(0.04);
        leg[iL]->SetBorderSize(0);
        leg[iL]->SetHeader("DUNE Simulation");
        leg[iL]->AddEntry(h_nue[iL],"#nu_{e}");
        leg[iL]->AddEntry(h_nuebar[iL],"#bar{#nu}_{e}");
        leg[iL]->AddEntry(h_numu[iL],"#nu_{#mu}");
        leg[iL]->AddEntry(h_numubar[iL],"#bar{#nu}_{#mu}");
        leg[iL]->AddEntry(h_nutau[iL],"#nu_{#tau}");
        leg[iL]->AddEntry(h_nutaubar[iL],"#bar{#nu}_{#tau}");
        leg_pyth[iL]->SetNColumns(2);
        leg_pyth[iL]->SetTextSize(0.04);
        leg_pyth[iL]->SetBorderSize(0);
        leg_pyth[iL]->SetHeader("DUNE Simulation");
        leg_pyth[iL]->AddEntry(h_nue_pyth[iL],"#nu_{e}");
        leg_pyth[iL]->AddEntry(h_nuebar_pyth[iL],"#bar{#nu}_{e}");
        leg_pyth[iL]->AddEntry(h_numu_pyth[iL],"#nu_{#mu}");
        leg_pyth[iL]->AddEntry(h_numubar_pyth[iL],"#bar{#nu}_{#mu}");
        leg_pyth[iL]->AddEntry(h_nutau[iL],"#nu_{#tau}");
        leg_pyth[iL]->AddEntry(h_nutaubar[iL],"#bar{#nu}_{#tau}");
        leg_pol[iL]->SetTextSize(0.04);
        leg_pol[iL]->SetBorderSize(0);
        leg_pol[iL]->SetHeader("DUNE Simulation");
        leg_pol[iL]->AddEntry(h_nutau_iso[iL],"#nu_{#tau} Isotropic Decay");
        leg_pol[iL]->AddEntry(h_nutaubar_iso[iL],"#bar{#nu}_{#tau} Isotropic Decay");
        leg_pol[iL]->AddEntry(h_nutau[iL],"#nu_{#tau} Muon-like Decay");
        leg_pol[iL]->AddEntry(h_nutaubar[iL],"#bar{#nu}_{#tau} Muon-like Decay");
        leg_br[iL]->SetHeader("DUNE Simulation");
        leg_br[iL]->SetNColumns(2);
        leg_br[iL]->SetTextSize(0.04);
        leg_br[iL]->SetBorderSize(0);
        leg_br[iL]->AddEntry(h_nutau[iL],"#nu_{#tau} Total");
        leg_br[iL]->AddEntry(h_nutaubar[iL],"#bar{#nu}_{#tau} Total");
        leg_br[iL]->AddEntry(h_nutau_ds[iL],"D_{s} #rightarrow #nu_{#tau}");
        leg_br[iL]->AddEntry(h_nutaubar_ds[iL],"D_{s} #rightarrow #bar{#nu}_{#tau}");
        leg_br[iL]->AddEntry(h_nutau_tau[iL],"#tau #rightarrow #nu_{#tau}");
        leg_br[iL]->AddEntry(h_nutaubar_tau[iL],"#bar{#tau} #rightarrow #bar{#nu}_{#tau}");
        leg_event[iL]->SetTextSize(0.04);
        leg_event[iL]->SetBorderSize(0);
        leg_event[iL]->SetHeader("DUNE Simulation");
        leg_event[iL]->AddEntry(h_nutau_event[iL],"#nu_{#tau}");
        leg_event[iL]->AddEntry(h_nutaubar_event[iL],"#bar{#nu}_{#tau}");
    }
    // Tau Neutrino Particle Loop
    for (int iP=0;iP<nP;iP++) {
        t->GetEntry(iP);
        tmeta->GetEntry(iP);
        t_iso->GetEntry(iP);
        if (dk2nu->decay.ntype == 16 || dk2nu->decay.ntype == -16) {
            switch (dk2nu->decay.ptype) {
            case 431:
            case -431:
                dk2nu->decay.ndecay = bsim::dkp_ds;
                break;
            case 15:
            case -15:
                dk2nu->decay.ndecay = bsim::dkp_tau;
                break;
            }
            dk_iso->decay.ndecay = 0;
            bsim::calcLocationWeights(dkmeta,dk2nu);
            bsim::calcLocationWeights(dkmeta,dk_iso);   
        }
        switch (dk2nu->decay.ntype) {
        case 16:
            ang = angle(dk2nu);
            for (int iL=0;iL<nL;iL++) {
                flux = dk2nu->nuray[iL].wgt/pot/area;
                tau_flux[iL] = tau_flux[iL] + flux;
                event = dat.GetXSec(dk2nu->nuray[iL].E,0)*e_coeff*det_mass[iL]*flux;
                tau_event[iL] = tau_event[iL] + event;
                h_nutau2d[iL]->Fill(dk2nu->nuray[iL].E,ang,flux);
                h_nutau[iL]->Fill(dk2nu->nuray[iL].E,flux);
                h_nutau_iso[iL]->Fill(dk_iso->nuray[iL].E,dk_iso->nuray[iL].wgt/pot/area);
		h_nutau_event[iL]->Fill(dk2nu->nuray[iL].E,event);
                switch (dk2nu->decay.ndecay) {
                case bsim::dkp_ds:
                    h_nutau_ds[iL]->Fill(dk2nu->nuray[iL].E,dk2nu->nuray[iL].wgt/pot/area);
                    break;
                case bsim::dkp_tau:
                    h_nutau_tau[iL]->Fill(dk2nu->nuray[iL].E,dk2nu->nuray[iL].wgt/pot/area);
                    break;
                }
            }
            break;
        case -16:
            ang = angle(dk2nu);
            for (int iL=0;iL<nL;iL++) {
                flux = dk2nu->nuray[iL].wgt/pot/area;
                taubar_flux[iL] = taubar_flux[iL] + flux;
                event = dat.GetXSec(dk2nu->nuray[iL].E,1)*e_coeff*det_mass[iL]*flux;
                taubar_event[iL] = taubar_event[iL] + event;
                h_nutaubar2d[iL]->Fill(dk2nu->nuray[iL].E,ang,flux);
                h_nutaubar[iL]->Fill(dk2nu->nuray[iL].E,flux);
                h_nutaubar_iso[iL]->Fill(dk_iso->nuray[iL].E,dk_iso->nuray[iL].wgt/pot/area);
                h_nutaubar_event[iL]->Fill(dk2nu->nuray[iL].E,event);
                switch (dk2nu->decay.ndecay) {
                case bsim::dkp_ds:
                    h_nutaubar_ds[iL]->Fill(dk2nu->nuray[iL].E,dk2nu->nuray[iL].wgt/pot/area);
                    break;
                case bsim::dkp_tau:
                    h_nutaubar_tau[iL]->Fill(dk2nu->nuray[iL].E,dk2nu->nuray[iL].wgt/pot/area);
                    break;
                }
            }
            break;
        }
    }
    // Particle Loop for PYTHIA nu_e, nu_mu
    for (int iP=0;iP<nP_p;iP++) {
      t_p->GetEntry(iP);
      tmeta_p->GetEntry(iP);
      bsim::calcLocationWeights(dkmeta_p,dk2nu_p);
      // Location Loop
      for (int iL=0;iL<nL;iL++) {
        switch(dk2nu_p->decay.ntype) {
        case 12:
            h_nue_pyth[iL]->Fill(dk2nu_p->nuray[iL].E,dk2nu_p->nuray[iL].wgt/pot_p/area);
            break;
        case -12:
            h_nuebar_pyth[iL]->Fill(dk2nu_p->nuray[iL].E,dk2nu_p->nuray[iL].wgt/pot_p/area);
            break;
        case 14:
            h_numu_pyth[iL]->Fill(dk2nu_p->nuray[iL].E,dk2nu_p->nuray[iL].wgt/pot_p/area);
            break;
        case -14:
            h_numubar_pyth[iL]->Fill(dk2nu_p->nuray[iL].E,dk2nu_p->nuray[iL].wgt/pot_p/area);
            break;
	}
      }
    }
    // Particle Loop for G4 nu_e, nu_mu
    for (int iP=0;iP<nP2;iP++) {
        t2->GetEntry(iP);
        // Location Loop
        for (int iL=0;iL<nL;iL++) {
            switch(dk2nu2->decay.ntype) {
            case 12:
                h_nue[iL]->Fill(dk2nu2->nuray[iL].E,dk2nu2->nuray[iL].wgt*dk2nu2->decay.nimpwt/pot2/area);
                break;
            case -12:
                h_nuebar[iL]->Fill(dk2nu2->nuray[iL].E,dk2nu2->nuray[iL].wgt*dk2nu2->decay.nimpwt/pot2/area);
                break;
            case 14:
                h_numu[iL]->Fill(dk2nu2->nuray[iL].E,dk2nu2->nuray[iL].wgt*dk2nu2->decay.nimpwt/pot2/area);
                break;
            case -14:
                h_numubar[iL]->Fill(dk2nu2->nuray[iL].E,dk2nu2->nuray[iL].wgt*dk2nu2->decay.nimpwt/pot2/area);
                break;
            }
        }
    }
    // Plotting
    set_style();
    for (int iL=0;iL<nL;iL++) {
        // Final Fluxes
        c->SetLogy();
        THStack *hstack = new THStack("hstack",(title[iL]+"; Neutrino Energy [GeV]; #nu's/cm^{2}/POT per 1 GeV").c_str());
        hstack->Add(h_nue[iL]);
        hstack->Add(h_nuebar[iL]);
        hstack->Add(h_numu[iL]);
        hstack->Add(h_numubar[iL]);
        hstack->Add(h_nutau[iL]);
        hstack->Add(h_nutaubar[iL]);
        hstack->Draw("nostack");
        leg[iL]->Draw();
        std::string file = std::string("flux")+std::to_string(iL)+std::string(".png");
        c->SaveAs(file.c_str());
        c->Clear();

	// Pythia Only Fluxes
        THStack *hstack_p = new THStack("hstack_p",(title[iL]+"; Neutrino Energy [GeV]; #nu's/cm^{2}/POT per 1 GeV").c_str());
        hstack_p->Add(h_nue_pyth[iL]);
        hstack_p->Add(h_nuebar_pyth[iL]);
        hstack_p->Add(h_numu_pyth[iL]);
        hstack_p->Add(h_numubar_pyth[iL]);
        hstack_p->Add(h_nutau[iL]);
        hstack_p->Add(h_nutaubar[iL]);
        hstack_p->Draw("nostack");
        leg[iL]->Draw();
        file = std::string("pyth")+std::to_string(iL)+std::string(".png");
        c->SaveAs(file.c_str());
        c->Clear();

	// Angular Dependence
	h_nutau2d[iL]->Draw("COLZ");
	set_axis_style(h_nutau2d[iL]);
	file = std::string("ang_nutau")+std::to_string(iL)+std::string(".png");
	c->SaveAs(file.c_str());
	c->Clear();
        h_nutaubar2d[iL]->Draw("COLZ");
	set_axis_style(h_nutaubar2d[iL]);
        file = std::string("ang_nutaubar")+std::to_string(iL)+std::string(".png");
        c->SaveAs(file.c_str());
        c->Clear();
	
        // Polarization comparison
        c->SetLogy(0);
        THStack *hstack2 = new THStack("hstack2",(title[iL]+"; Neutrino Energy [GeV]; #nu's/cm^{2}/POT per 1 GeV").c_str());
        hstack2->Add(h_nutau_iso[iL]);
        hstack2->Add(h_nutaubar_iso[iL]);
        hstack2->Add(h_nutau[iL]);
        hstack2->Add(h_nutaubar[iL]);
        hstack2->Draw("nostack");
        leg_pol[iL]->Draw();
        file = std::string("pols")+std::to_string(iL)+std::string(".png");
        c->SaveAs(file.c_str());
        c->Clear(); 
        // Branching comparison
        c->SetLogy();
        THStack *hstack3 = new THStack("hstack3",(title[iL]+"; Neutrino Energy [GeV]; #nu's/cm^{2}/POT per 1 GeV").c_str());
        hstack3->Add(h_nutau[iL]);
        hstack3->Add(h_nutaubar[iL]);
        hstack3->Add(h_nutau_tau[iL]);
        hstack3->Add(h_nutaubar_tau[iL]);
        hstack3->Add(h_nutau_ds[iL]);
        hstack3->Add(h_nutaubar_ds[iL]);
        hstack3->Draw("nostack");
        leg_br[iL]->Draw();
        file = std::string("procs")+std::to_string(iL)+std::string(".png");
        c->SaveAs(file.c_str());
	// Event Rate
	c->SetLogy(0);
        THStack *hstack4 = new THStack("hstack4",(title[iL]+"; Neutrino Energy [GeV]; Events/year per 1 GeV").c_str());
        hstack4->Add(h_nutau_event[iL]);
        hstack4->Add(h_nutaubar_event[iL]);
        hstack4->Draw("nostack");
        leg_event[iL]->Draw();
        file = std::string("event")+std::to_string(iL)+std::string(".png");
        c->SaveAs(file.c_str());
   }
  // Print Neutrino Count
  std::cout << "----nu_tau---" << std::endl;
  std::cout << "Flux:       ND = " << tau_flux[1] << "    FD = " << tau_flux[2] << std::endl;
  std::cout << "Event Rate: ND = " << tau_event[1] << "    FD = " << tau_event[2] << std::endl;
  std::cout << "----nu_taubar---" << std::endl;
  std::cout << "Flux:       ND = " << taubar_flux[1] << "    FD = " << taubar_flux[2] << std::endl;
  std::cout << "Event Rate: ND = " << taubar_event[1] << "    FD = " << taubar_event[2] << std::endl;
}

