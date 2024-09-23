################################################################################
# VARIABLES: Definition of the relevant variables from the configuration script.
################################################################################

# Set the shell.
SHELL=/usr/bin/env bash

# Include the configuration.
-include Makefile.inc

CXX_COMMON:=-I$(PREFIX_INCLUDE) $(CXX_COMMON) $(GZIP_LIB)
CXX_COMMON+= -L$(PREFIX_LIB) -Wl,-rpath,$(PREFIX_LIB) -lpythia8 -ldl

################################################################################
# RULES: Definition of the rules used to build the PYTHIA examples.
################################################################################

# Rules without physical targets (secondary expansion for specific rules).
.SECONDEXPANSION:
.PHONY: all clean

# All targets (no default behavior).
all:
	$(info Make options: sim, plot, clean)
# PYTHIA library.
$(PYTHIA):
	$(error Error: PYTHIA must be built, please run "make"\
                in the top PYTHIA directory)


# Compile and run plotting
plot:
	$(CXX) plot.cc -o plot -w $(DK2NU_INCLUDE) $(DK2NU_LIB) $(ROOT_LIB) \
	$(shell $(ROOT_CONFIG) --cflags --glibs)
	./plot

# Compile and run simulation
sim:
	$(CXX) $@.cc -o $@ -w $(DK2NU_INCLUDE) $(DK2NU_LIB) $(ROOT_LIB) $(CXX_COMMON) \
	$(shell $(ROOT_CONFIG) --cflags --glibs)
	./$@ $@.cmnd > $@.log

# Clean.
clean:
	rm ./plot
	rm ./sim
