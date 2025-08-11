#!/usr/bin/bash
export UPS_OVERRIDE="-H Linux64bit+3.10-2.17"
source /cvmfs/fermilab.opensciencegrid.org/products/genie/bootstrap_genie_ups.sh
setup root v6_28_12 -q e26:p3915:prof
setup pythia8 v8_3_10 -q e26:p3915:prof
export G4LBNEWORKDIR=/exp/dune/app/users/ptemplem/g4lbne
export DATA=/exp/dune/app/users/ptemplem/sim_data
