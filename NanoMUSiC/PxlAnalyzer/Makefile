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
DIRS := $(MYPXLANA)
DIRS += Main
DIRS += Tools

# define PXL library directories
ifeq ($(wildcard Pxl/*),)
$(error ERROR: PXL does not exist - Call setup.sh)
endif
PXLDIR := Pxl/Pxl
DIRS +=$(PXLDIR)/src
DIRS +=$(PXLDIR)/interface

# define source files
SOURCES := $(wildcard *.cc)
SOURCES += $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cc))

# define header, object and dependency files based on source files
HEADERS := $(SOURCES:.cc=.h)
OBJECTS := $(SOURCES:.cc=.o)
DEPENDS := $(SOURCES:.cc=.d)
GCDAS   := $(SOURCES:.cc=.gcda)


########################################
# compiler and flags

# ROOT libraries:
ROOT_CFLAGS  := $(shell root-config --cflags)
ROOT_LDFLAGS := $(shell root-config --ldflags)
ROOT_GLIBS   := $(shell root-config --libs)


# LCG Base dir
LCG_BASE := /cvmfs/sft.cern.ch/lcg/views/LCG_102/x86_64-centos7-gcc11-opt

LHAPDF_LIB_PATH := -L$(LCG_BASE)/lib
LHAPDF_INC_PATH := -I$(-I)/include/LHAPDF
LHAPDF_LIB := -lLHAPDF

LCG_INC_PATHS := -I$(LCG_BASE)/include
BOOST_INC_PATHS += -I$(LCG_BASE)/include/boost
GSL_INC_PATHS += -I$(LCG_BASE)/include/gsl
LIBS3A_INC_PATHS := -I$(LIBS3A)/include

LCG_LIB_PATHS := -L$(LCG_BASE)/lib
LCG_LIB_PATHS += -L$(LCG_BASE)/lib64

LCG_LIBS := -ldcap
LCG_LIBS += -lgsl
LCG_LIBS += -lgslcblas
LCG_LIBS += -lboost_system
LCG_LIBS += -lboost_thread
LCG_LIBS += -lboost_filesystem
LCG_LIBS += -lboost_date_time
LCG_LIBS += -lboost_serialization
LCG_LIBS += -lboost_program_options


# PXL interfaces
PXL_INC_PATH := -I$(PXLANA)/$(PXLDIR)/src

# TAPAS tools
TOOLS_INC_PATH := -I$(PXLANA)/Tools
TOOLS_INC_PATH += -I$(PXLANA)/include

# Gather all additional flags
EXTRA_CFLAGS  := -ffloat-store $(PXL_INC_PATH) $(TOOLS_INC_PATH) $(LCG_INC_PATHS) $(LHAPDF_INC_PATH) $(BOOST_INC_PATHS) $(GSL_INC_PATHS) $(LIBS3A_INC_PATHS) -DPXL_ENABLE_DCAP -DSTANDALONE
EXTRA_LDFLAGS := $(LCG_LIB_PATHS) $(LCG_LIBS) $(LHAPDF_LIB_PATH) $(LHAPDF_LIB)

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
CFLAGS  += -I. $(ROOT_CFLAGS) $(EXTRA_CFLAGS)
LDFLAGS += $(ROOT_LDFLAGS) $(ROOT_GLIBS) $(SYSLIBS) -L. $(EXTRA_LDFLAGS)


########################################
# define colors for better overview

BLACK        := $(shell tput -Txterm setaf 0)
RED          := $(shell tput -Txterm setaf 1)
GREEN        := $(shell tput -Txterm setaf 2)
YELLOW       := $(shell tput -Txterm setaf 3)
LIGHTPURPLE  := $(shell tput -Txterm setaf 4)
PURPLE       := $(shell tput -Txterm setaf 5)
BLUE         := $(shell tput -Txterm setaf 6)
WHITE        := $(shell tput -Txterm setaf 7)
NC := $(shell tput -Txterm sgr0)


########################################
# define all targets

TARGETS := $(MUSIC) $(PRINTER)
TARGETS += $(PRINTER)



########################################
# additional includes
# if you have additional includes, you can put them in the includes.mk file in
# your analysis directory
ifneq (,$(wildcard $(MYPXLANA)/includes.mk))
$(info Makefile fragment $(GREEN)include.mk$(NC) found. Including ...)
-include $(MYPXLANA)/includes.mk
endif


########################################
# targets

# target: dependency
#	@shellcommand
#	command

all: $(OBJECTS)
	@touch PxlAnalyzer.d

clean:
	@rm -f $(MUSIC) $(OBJECTS) $(DEPENDS) $(GCDAS)
	@rm -rf bin
	@rm -rf PxlAnalyzer.d


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
