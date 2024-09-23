#include "TH1.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "tree/dk2nu.h"
#include "tree/dk2nu.cc"

int main(){
    TFile *f = new TFile("nu_tree.root");
    TTree *t = (TTree*)f->Get("dk2nuTree");
    bsim::Dk2Nu*  dk2nu  = new bsim::Dk2Nu;
    t->SetBranchAddress("dk2nu", &dk2nu);
    const double pi = 3.14159;
    std::string flux_title = std::string("#nu_{#tau} Flux; Neutrino Energy [GeV]; #nu/100cm^2/GeV (1e7 POT)");
    std::cout << dk2nu->potnum;
    TH1D* h1 = new TH1D("Flux",flux_title.c_str(),60,0,120);
    TH1D* h2 = new TH1D("Angle","#theta_{#nu}; #theta [rad]; Count",20,0,1);
    
    double px;
    double py;
    double pz;
    double pt;
    double theta;

    int loc = 1;
    int n = t->GetEntries();
    // Particle Loop
    for (int i=0;i<n;i++) {
        t->GetEntry(i);
        h1->Fill(dk2nu->nuray[loc].E,dk2nu->nuray[loc].wgt/pi);
        px = dk2nu->nuray[loc].px;
        py = dk2nu->nuray[loc].py;
        pz = dk2nu->nuray[loc].pz;
        pt = TMath::Sqrt(px*px+py*py);
        theta = TMath::ATan2(pt,pz);
        h2->Fill(theta);
        std::cout << dk2nu->nuray[loc].E << ", " << dk2nu->nuray[loc].wgt/pi << "\n";
        std::cout << theta << "\n";
    }
    TCanvas* c1 = new TCanvas("Flux");
    h1->SetStats(0);
    h1->Draw();
    TCanvas* c2 = new TCanvas("Angle");
    h2->SetStats(0);
    h2->Draw();
    c1->SaveAs("flux.png");
    c2->SaveAs("angle.png");
    }