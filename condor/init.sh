#!/usr/bin/bash
seed=$(($1+1)) # seed needs to be > 0
source setup.sh
tar -xzf  dk2nu.tar.gz
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./dk2nu/lib
echo $seed
./sim sim.cmnd $seed
