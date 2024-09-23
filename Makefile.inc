# Install directory prefixes.
PYTHIA=/home/ptemplem/dunetau/pythia/pythia8310
PREFIX_BIN=$(PYTHIA)/bin
PREFIX_INCLUDE=$(PYTHIA)/include
PREFIX_LIB=$(PYTHIA)/lib
PREFIX_SHARE=$(PYTHIA)/share/Pythia8

# Compilation flags
CXX=g++
CXX_COMMON=-O2 -std=c++11 -pedantic -W -Wall -Wshadow -fPIC -pthread
CXX_SHARED=-shared
CXX_SONAME=-Wl,-soname,
LIB_SUFFIX=.so
OBJ_COMMON=

ROOT=/home/ptemplem/.conda/envs/pyroot
ROOT_CONFIG=root-config
ROOT_BIN=$(ROOT)/bin
ROOT_INCLUDE=-I$(ROOT)/include
ROOT_LIB=-L$(ROOT)/lib -Wl,-rpath,$(ROOT)/lib -lCore

DK2NU=/home/ptemplem/dk2nu
DK2NU_INCLUDE=-I$(DK2NU)
DK2NU_LIB=-L$(DK2NU)/lib -ldk2nuTree -Wl,-rpath,$(DK2NU)/lib
