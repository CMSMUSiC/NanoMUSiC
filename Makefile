ifndef MUSIC_IS_SET_ENV
$(error MUSiC envs are not set. First run: source setenv_music.sh)
endif


SHELL := /bin/bash
MAKEFILE_DIR := ${CURDIR}

all: skimmer utils roi pxlanalyzer 

skimmer: 
	cd $(MAKEFILE_DIR)/CMSSW_10_6_29/src ; cmsenv ; scram build -j 8 ; scram build

utils: 
	cd $(MAKEFILE_DIR)/tapas/MUSiC-Utils/ ; make

roi: 
	cd $(MAKEFILE_DIR)/tapas/MUSiC-RoIScanner/ ; make 

roi_lut: 
	cd $(MAKEFILE_DIR)/tapas/MUSiC-RoIScanner/ ; make ; make lut

pxlanalyzer: 
	cd $(MAKEFILE_DIR)/tapas/PxlAnalyzer/ ; make

clean: skimmer_clean utils_clean roi_clean pxlanalyzer_clean

skimmer_clean: 
	cd $(MAKEFILE_DIR)/CMSSW_10_6_29/src ; cmsenv ; scram build clean

utils_clean: 
	cd $(MAKEFILE_DIR)/tapas/MUSiC-Utils/ ; make clean

roi_clean: 
	cd $(MAKEFILE_DIR)/tapas/MUSiC-RoIScanner/ ; make  clean

pxlanalyzer_clean: 
	cd $(MAKEFILE_DIR)/tapas/PxlAnalyzer/ ; make clean
