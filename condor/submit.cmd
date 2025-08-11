Universe   = vanilla
Executable = init.sh
Arguments  = $(Process)
Log        = sim.log
input      = /dev/null
Output     = out/sim.out.$(Cluster).$(Process)
Error      = err/sim.err.$(Cluster).$(Process)

should_transfer_files=YES
transfer_input_files=../sim,../sim.cmnd,../setup.sh,../locations.txt,dk2nu.tar.gz
when_to_transfer_output = ON_EXIT

RequestCpus = 1
RequestMemory = 1GB
Request_disk = 1GB
+SingularityImage = "/cvmfs/singularity.opensciencegrid.org/fermilab/fnal-dev-sl7:latest"
+AccountingGroup = "group_dune.ptemplem"
Queue 300
