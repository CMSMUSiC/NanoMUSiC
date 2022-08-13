### definition of the compiler options ###
#       -I location of directory containing include files
#       -L location of directory containing libraries
#       -lname include the library from -L location called libname.a
#          -lg2c is the library containing info on converting fortran to C
#          -lf   is the library containing the intrinsic for HPUX only.
#       -shared make a shared library as output
#       -fPIC produce position independent code
#        necessary on some platforms (including HPUX) for -shared
#       -fpic ^^ same(?)
#       -O optimizes
#       -g produces output for the debugger
#       -pg produces output for gprof profiler
#       note: if you want to see all warnings and ensure ansi standard
#             compatibility, use:
#             -pipe -ansi -pedantic -fnonnull-objects \
#             -W -Wall -Wwrite-strings -Wpointer-arith -Wnested-externs \
#             -Woverloaded-virtual -Wbad-function-cast -fnonnull-objects
#       The proper order for cernlib libraries is:
#       -lpawlib -lgraflib -lgrafX11 -lmathlib -lkernlib -lpacklib -ljetset74
#
# makefile syntax:
#        for target thedir/target.suf from source anotherdir/source.suf2
#        ${*D}  = thedir
#        ${*F}  = target
#        $^     = name of all prerequisites
#        $+     = like $^ with duplicated prerequisites if mentioned more than once
#        $*     = thedir/target
#        $@     = thedir/target.suf
#        $<     = anotherdir/source.suf2
#

MUSIC:=MUSiC/music

PRINTER:=MUSiC/nanoaod_content_printer

########################################
# directories
# by default, the validator is compiled.
# usually one will change this to their personal analysis.

ifndef MYPXLANA
MYPXLANA:=Validator
endif

# define all directories with source code
DIRS := MUSiC

# define PXL library directories
ifeq ($(wildcard PxlAnalyzer/Pxl/*),)
$(error ERROR: PXL does not exist - Call setup.sh)
endif
PXLDIR := Pxl/Pxl

# define source files
# SOURCES := $(wildcard *.cc)
SOURCES += $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cc))

# define header, object and dependency files based on source files
HEADERS := $(SOURCES:.cc=.h)
OBJECTS := $(SOURCES:.cc=.o)
DEPENDS := $(SOURCES:.cc=.d)


########################################
# compiler and flags

# ROOT libraries:
ROOT_CFLAGS  := $(shell root-config --cflags)
ROOT_LDFLAGS := $(shell root-config --ldflags)
ROOT_GLIBS   := $(shell root-config --libs)

# Paths for CMSSW libraries:
ifndef CMSSW_RELEASE_BASE
$(error ERROR: CMSSW libraries not found - Source CMSSW)
endif
CMSSW_DCAP_BASE  := $(shell cat $(CMSSW_RELEASE_BASE)/config/toolbox/$(SCRAM_ARCH)/tools/selected/dcap.xml | grep 'name="DCAP_BASE"' | sed -e 's/.*default="//' | sed -e 's/"\/>//')
CMSSW_BOOST_BASE := $(shell cat $(CMSSW_RELEASE_BASE)/config/toolbox/$(SCRAM_ARCH)/tools/selected/boost.xml | grep 'name="BOOST_BASE"' | sed -e 's/.*default="//' | sed -e 's/"\/>//')
CMSSW_GSL_BASE   := $(shell cat $(CMSSW_RELEASE_BASE)/config/toolbox/$(SCRAM_ARCH)/tools/selected/gsl.xml | grep 'name="GSL_BASE"' | sed -e 's/.*default="//' | sed -e 's/"\/>//')

# If you're using a patched CMSSW release, some of the libs are still in the base release, so you also have to look there.
CMSSW_RELEASE_BASE_NOPATCH := $(shell echo $(CMSSW_RELEASE_BASE) | sed -e 's/-patch//' -e 's/_patch.//')

CMSSW_LIB_PATHS := -L$(CMSSW_BASE)/lib/$(SCRAM_ARCH)
CMSSW_LIB_PATHS += -L$(CMSSW_RELEASE_BASE)/lib/$(SCRAM_ARCH)
CMSSW_LIB_PATHS += -L$(CMSSW_RELEASE_BASE_NOPATCH)/lib/$(SCRAM_ARCH)
CMSSW_LIB_PATHS += -L$(CMSSW_DCAP_BASE)/lib
CMSSW_LIB_PATHS += -L$(CMSSW_GSL_BASE)/lib
CMSSW_LIB_PATHS += -L$(CMSSW_BOOST_BASE)/lib

CMSSW_LIBS += -lCondFormatsJetMETObjects
CMSSW_LIBS += -lJetMETCorrectionsModules
CMSSW_LIBS += -lPhysicsToolsUtilities
CMSSW_LIBS += -lCondFormatsBTauObjects
CMSSW_LIBS += -lCondToolsBTau
CMSSW_LIBS += -ldcap
CMSSW_LIBS += -lgsl
CMSSW_LIBS += -lgslcblas
#The first two lines below are needed for CMSSW_10_6_X. Maybe not for previous versions
CMSSW_LIBS += -lboost_system
CMSSW_LIBS += -lboost_thread
CMSSW_LIBS += -lboost_filesystem
CMSSW_LIBS += -lboost_date_time
CMSSW_LIBS += -lboost_serialization
CMSSW_LIBS += -lboost_program_options
CMSSW_LIBS += -lFWCoreUtilities

# For the headers there are symlinks.
CMSSW_INC_PATHS := -isystem$(CMSSW_BASE)/src
CMSSW_INC_PATHS += -isystem$(CMSSW_RELEASE_BASE)/src
CMSSW_INC_PATHS += -isystem$(CMSSW_DCAP_BASE)/include
CMSSW_INC_PATHS += -isystem$(CMSSW_BOOST_BASE)/include
CMSSW_INC_PATHS += -isystem$(CMSSW_GSL_BASE)/include
CMSSW_INC_PATHS += -isystem$(LIBS3A)/include

#no nice way to get this automated
ifeq ($(SCRAM_ARCH),slc7_amd64_gcc700)
LHAPDF_BASE := /cvmfs/cms.cern.ch/slc7_amd64_gcc700/external/lhapdf/6.2.1-omkpbe2
else ifeq ($(SCRAM_ARCH),slc6_amd64_gcc700)
LHAPDF_BASE := /cvmfs/cms.cern.ch/slc6_amd64_gcc700/external/lhapdf/6.2.1-omkpbe2
else ifeq ($(SCRAM_ARCH),slc7_amd64_gcc530)
LHAPDF_BASE :=/cvmfs/cms.cern.ch/slc7_amd64_gcc493/external/lhapdf/6.1.5-kpegke
else ifeq ($(SCRAM_ARCH),slc6_amd64_gcc530)
LHAPDF_BASE :=/cvmfs/cms.cern.ch/slc6_amd64_gcc493/external/lhapdf/6.1.5-kpegke2
else ifeq ($(SCRAM_ARCH),slc7_amd64_gcc820)
LHAPDF_BASE :=/cvmfs/cms.cern.ch/slc7_amd64_gcc820/external/lhapdf/6.2.1-nmpfii3
else
$(error ERROR: Please specify a LHA path for this scram arch (probably /cvmfs/cms.cern.ch/SCRAM_ARCH/external/lhapdf/VERSION))
endif

LHAPDF_LIB_PATH := -L$(LHAPDF_BASE)/lib
LHAPDF_INC_PATH := -I$(LHAPDF_BASE)/include
LHAPDF_LIB := -lLHAPDF

# PXL interfaces
PXL_INC_PATH := -I$(PXLANA)/$(PXLDIR)/src

# TAPAS tools
TOOLS_INC_PATH := -I$(PXLANA)/Tools
TOOLS_INC_PATH += -I$(PXLANA)/include

# Gather all additional flags
EXTRA_CFLAGS  := -ffloat-store $(PXL_INC_PATH) $(TOOLS_INC_PATH) $(CMSSW_INC_PATHS) $(LHAPDF_INC_PATH) -DPXL_ENABLE_DCAP -DSTANDALONE
EXTRA_LDFLAGS := $(CMSSW_LIB_PATHS) $(CMSSW_LIBS) $(LHAPDF_LIB_PATH) $(LHAPDF_LIB)

MUSIC_LDFLAGS := -L$(MUSIC_UTILS)/lib -lTEventClass -L$(CORRECTIONLIB)/lib  -lcorrectionlib

CC := g++
CFLAGS	:= -O3 -Wall -std=gnu++17

LD := g++
LDFLAGS := -O3fast -lz

# Debug flags?
ifdef DEBUG
CFLAGS = -Og -Wall -fPIC -g -pg -fprofile-generate
LDFLAGS = -Og -g -pg -lz -fprofile-generate
endif

CFLAGS  += -DMYPXLANA=$(MYPXLANA)/AnalysisComposer.hh
CFLAGS  += -I. -I$(PXLANA) -I$(CORRECTIONLIB)/include $(ROOT_CFLAGS) $(EXTRA_CFLAGS)
LDFLAGS += $(ROOT_LDFLAGS) $(ROOT_GLIBS) $(SYSLIBS) -L. $(EXTRA_LDFLAGS) $(MUSIC_LDFLAGS)


########################################
# additional includes
# if you have additional includes, you can put them in the includes.mk file in
# your analysis directory
ifneq (,$(wildcard $(PXLANA)/$(MYPXLANA)/includes.mk))
$(info Makefile fragment $(GREEN)include.mk$(NC) found. Including ...)
-include $(PXLANA)/$(MYPXLANA)/includes.mk
endif


########################################
# targets

# target: dependency
#	@shellcommand
#	command

BLACK        := $(shell tput -Txterm setaf 0)
RED          := $(shell tput -Txterm setaf 1)
GREEN        := $(shell tput -Txterm setaf 2)
YELLOW       := $(shell tput -Txterm setaf 3)
LIGHTPURPLE  := $(shell tput -Txterm setaf 4)
PURPLE       := $(shell tput -Txterm setaf 5)
BLUE         := $(shell tput -Txterm setaf 6)
WHITE        := $(shell tput -Txterm setaf 7)
NC := $(shell tput -Txterm sgr0)


# define all directories with source code
PXLANA_DIRS := $(PXLANA)/$(MYPXLANA)
PXLANA_DIRS += $(PXLANA)/Main
PXLANA_DIRS += $(PXLANA)/Tools
PXLANA_DIRS +=$(PXLANA)/$(PXLDIR)/src
PXLANA_DIRS +=$(PXLANA)/$(PXLDIR)/interface

PXLANA_SOURCES += $(foreach dir, $(PXLANA_DIRS), $(wildcard $(dir)/*.cc))
PXLANA_HEADERS := $(PXLANA_SOURCES:.cc=.h)
PXLANA_OBJECTS := $(PXLANA_SOURCES:.cc=.o)
PXLANA_DEPENDS := $(PXLANA_SOURCES:.cc=.d)

SHELL := /bin/bash
MAKEFILE_DIR := ${CURDIR}


all: $(MUSIC) $(PRINTER)

# .PHONY: PxlAnalyzer
# pxlanalyzer: 
PxlAnalyzer/PxlAnalyzer.d:  $(PXLANA_SOURCES)
	@echo "${YELLOW}Building PxlAnalyzer ...${NC}"
	$(MAKE) -C ./PxlAnalyzer/
	@echo "${GREEN} PxlAnalyzer done ...${NC}"

$(MUSIC): PxlAnalyzer/PxlAnalyzer.d MUSiC/music.o
	@echo "${YELLOW}Building $@ ...${NC}"
	$(LD) MUSiC/music.o $(PXLANA_OBJECTS) $(LDFLAGS)  -o $@
	@echo "${GREEN}$@ done${NC}"
	@mkdir -p bin
	@cp $(MUSIC) bin/

$(PRINTER): PxlAnalyzer/PxlAnalyzer.d MUSiC/nanoaod_content_printer.o
	@echo "${YELLOW}Building $@ ...${NC}"
	$(LD) MUSiC/nanoaod_content_printer.o $(PXLANA_OBJECTS) $(LDFLAGS)  -o $@
	@echo "${GREEN}$@ done${NC}"
	@mkdir -p bin
	@cp $(PRINTER) bin/

clean: 
	@echo "${GREEN}Cleanning PxlAnalyzer ...${NC}"
	$(MAKE) -C ./PxlAnalyzer/ clean
	@echo "${GREEN}Cleanning MUSiC ...${NC}"
	@rm -f  $(MUSIC) $(OBJECTS) $(DEPENDS) 
	# @echo "${GREEN}Cleanning PxlAnalyzer ...${NC}"
	# @rm -f  $(PXLANA_OBJECTS) $(PXLANA_DEPENDS)
	@echo "${GREEN}Cleanning bin ...${NC}"
	@rm -rf bin/*


########################################
# additional targets
# if you have additional targets, you can put them in the targets.mk file in
# your analysis directory

ifneq (,$(wildcard $(MYPXLANA)/targets.mk))
$(info Makefile fragment $(GREEN)targets.mk$(NC) found. Including ...)
-include $(MYPXLANA)/targets.mk
endif


########################################
# rules

%.o : %.cc
	@echo "${YELLOW}Building $@ ...${NC}"
	$(CC) -MD -MP $(CFLAGS) -g -c -o $@ $<
	@echo "${GREEN} $@ done ...${NC}"

-include $(DEPENDS)
