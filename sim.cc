// Header file to access Pythia 8 program elements.
#include "Pythia8/Pythia.h"

// ROOT headers
#include "TH1.h"
#include "TFile.h"
#include "TTree.h"
#include "tree/dk2nu.h"
#include "tree/dk2nu.cc"
#include "tree/dkmeta.h"
#include "tree/calcLocationWeights.h"
#include "tree/readWeightLocations.h"


using namespace Pythia8;
int main(int argc, char* argv[]) {
    int tmp_n = 0;
    double tmp_dtot = 0;
    double tmp_ttot = 0;
    Pythia pythia;
    // Pythia Init
    pythia.readFile(argv[1]);
    pythia.readString("Random:setSeed = on");
    std::string seed = std::string("Random:seed = ") + std::string(argv[3]);
    pythia.readString(seed.c_str());
    pythia.init();

    // Init TTree
    std::string file = std::string(argv[2])+std::string("/nu_tree")+std::string(argv[3])+std::string(".root");
    TFile f(file.c_str(), "RECREATE");
    bsim::Dk2Nu dk2nu;
    bsim::DkMeta dkmeta;
    bsim::Decay decay;
    TTree dk2nuTree("dk2nuTree","Event Data");
    TTree dkmetaTree("dkmetaTree","Meta Data");
    dk2nuTree.Branch("dk2nu","bsim::Dk2Nu",&dk2nu,32000,99);
    dkmetaTree.Branch("dkmeta","bsim::DkMeta",&dkmeta,32000,99);

    //Fill Detector Locations
    std::string locfilename = "locations.txt";
    bsim::readWeightLocations(locfilename,&dkmeta);

    size_t nloc = dkmeta.location.size();
    std::cout << "Read " << nloc << " locations read from \"" 
                << locfilename << "\"" << std::endl;
    for (size_t iloc = 0; iloc < nloc; ++iloc ) {
        std::cout << "[ " << std::setw(2) << iloc << "] "
                << dkmeta.location[iloc] << std::endl;
    }
    
    int nEvent = 1e5; // 1e8 Runs about 12hr per job
    int job = std::stoi(std::string(argv[3]));
    // Event Loop
    for (int iEvent = 0; iEvent < nEvent; ++iEvent){
        if (!pythia.next()) continue;
        // Particle Loop
        for (int i = 0; i < pythia.event.size(); ++i) {
	    int id = pythia.event[i].id();
            if (id == 16 || id == -16 || id == 14 || id == -14 || id == 12 || id == -12) {
                //Setup
                dk2nu.job = job;
                dk2nu.potnum = iEvent;
                decay.ntype = id;

                //Neutrino Vertex
                decay.vx = pythia.event[i].xProd();
                decay.vy = pythia.event[i].yProd();
                decay.vz = pythia.event[i].zProd();
                
                //Neutrino momentum
                if ( dkmeta.location[0].name == "random decay" ) {
                    bsim::NuRay nurndm(pythia.event[i].px(),pythia.event[i].py(),pythia.event[i].pz(),pythia.event[i].e(),1.0);
                    dk2nu.nuray.push_back(nurndm);
                }

                //Parent Data
                int iP = pythia.event[i].mother1();
                decay.ptype = pythia.event[iP].id();
                decay.pdpx = pythia.event[iP].px(); 
                decay.pdpy = pythia.event[iP].py();
                decay.pdpz = pythia.event[iP].pz();

                decay.ppdydz = pythia.event[iP].py()/pythia.event[iP].pz();
                decay.ppdxdz = pythia.event[iP].px()/pythia.event[iP].pz();
                decay.pppz = pythia.event[iP].pz();
                decay.ppenergy = pythia.event[iP].e();

		string products = "";
		vector<int> daughters = pythia.event[iP].daughterList();
		for (const int& iD : daughters) {
		    products = products + pythia.event[iD].name();
		    products = products + " ";
		} 
                //std::cout << pythia.event[iP].name() << " -> " << products <<"\n";

		//Calculating Neutrino's CM Energy
                double m_par = pythia.event[iP].m();
                double e_prod = pythia.event[iP].e()*pythia.event[i].e();
		double p_prod = pythia.event[iP].px()*pythia.event[i].px()+pythia.event[iP].py()*pythia.event[i].py()+pythia.event[iP].pz()*pythia.event[i].pz();
		decay.necm = (e_prod-p_prod)/m_par; 
		//Grandparent Data
                int iM = pythia.event[iP].mother1();
                decay.muparpx = pythia.event[iM].px();
                decay.muparpy = pythia.event[iM].py();
                decay.muparpz = pythia.event[iM].pz();
                decay.mupare = pythia.event[iM].e();

                //Weight Calc and Fill
                dk2nu.decay = decay;
                //bsim::calcLocationWeights(&dkmeta,&dk2nu);  Can't do this more than once, moved to plot.cc for testing                
                dk2nuTree.Fill();
                dk2nu.clear();

		//DkMeta Fill
		dkmeta.job = job;
		dkmeta.pots = nEvent;
		dkmetaTree.Fill(); 
            }
        }
    }
    // f.cd();
    dk2nuTree.Write();
    dkmetaTree.Write();
    // Outputs
    cout << "Number of Events: " << nEvent;
    //pythia.stat();
    return 0;
}
