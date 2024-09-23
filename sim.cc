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
    Pythia pythia;
    // Pythia Init
    pythia.readFile(argv[1]);
    pythia.init();

    // Init TTree
    TFile f("nu_tree.root", "RECREATE");
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
    dkmetaTree.Fill();
    
    int nEvent = 1000000;
    int job = 1;
    // Event Loop
    for (int iEvent = 0; iEvent < nEvent; ++iEvent){
        if (!pythia.next()) continue;
        // Particle Loop
        for (int i = 0; i < pythia.event.size(); ++i) {
            if (pythia.event[i].id() == 16 && pythia.event[pythia.event[i].mother1()].id() == 431) {
                //Setup
                dk2nu.job = job;
                dk2nu.potnum = iEvent;
                decay.ndecay= 16; 
                decay.ntype = 16;

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
                if (decay.ptype != 431) {
                    std::cout << "Neutrino NOT from Ds Decay! Actually from " << decay.ptype;
                }
                std::cout << pythia.event[pythia.event[iP].daughter1()].id() << "  " << pythia.event[pythia.event[iP].daughter2()].id() <<"\n";
                decay.pdpx = pythia.event[iP].px(); //Prod vertexes are essentially the same
                decay.pdpy = pythia.event[iP].py();
                decay.pdpz = pythia.event[iP].pz();

                decay.ppdydz = pythia.event[iP].py()/pythia.event[iP].pz();
                decay.ppdxdz = pythia.event[iP].px()/pythia.event[iP].pz();
                decay.pppz = pythia.event[iP].pz();
                decay.ppenergy = pythia.event[iP].e();

                //Calculating Neutrino's CM Energy
                double m_ds = pythia.event[iP].m();
                int iTau = pythia.event[iP].daughter1();
                double m_tau = pythia.event[iTau].m();
                decay.necm = (m_ds*m_ds-m_tau*m_tau)/(2*m_ds);
                
                //Grandparent Data
                int iM = pythia.event[iP].mother1();
                decay.muparpx = pythia.event[iM].px();
                decay.muparpy = pythia.event[iM].py();
                decay.muparpz = pythia.event[iM].pz();
                decay.mupare = pythia.event[iM].e();

                //Weight Calc and Fill
                dk2nu.decay = decay;
                bsim::calcLocationWeights(&dkmeta,&dk2nu);
                dk2nuTree.Fill();
                dk2nu.clear();
            }
            if (pythia.event[i].id() == -16) {
                // iNuTBar += 1;
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