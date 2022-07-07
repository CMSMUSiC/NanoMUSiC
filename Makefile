SHELL := /bin/bash
DIR := ${CURDIR}

all: skimmer utils roi pxlanalyzer 

skimmer: 
	cd $(DIR)/CMSSW_10_6_29/src ; cmsenv ; scram build -j 8 ; scram build

utils: 
	cd $(DIR)/tapas/MUSiC-Utils/ ; make

roi: 
	cd $DIR/tapas/MUSiC-RoIScanner/ ; make  ; make lut

pxlanalyzer: 
	cd $DIR/tapas/PxlAnalyzer/ ; make

clean: skimmer_clean utils_clean roi_clean pxlanalyzer_clean

skimmer_clean: 
	cd $(DIR)/CMSSW_10_6_29/src ; cmsenv ; scram build clean

utils_clean: 
	cd $(DIR)/tapas/MUSiC-Utils/ ; make clean

roi_clean: 
	cd $DIR/tapas/MUSiC-RoIScanner/ ; make  clean

pxlanalyzer_clean: 
	cd $DIR/tapas/PxlAnalyzer/ ; make clean


