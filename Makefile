SHELL := /bin/bash
DIR := ${CURDIR}

all: skimmer utils roi pxlanalyzer 

skimmer: 
	source setenv_music.sh no_proxy ; cd $(DIR)/CMSSW_10_6_29/src ; cmsenv ; scram build -j 8 ; scram build

utils: 
	source setenv_music.sh no_proxy ; cd $(DIR)/tapas/MUSiC-Utils/ ; make

roi: 
	source setenv_music.sh no_proxy ; cd $DIR/tapas/MUSiC-RoIScanner/ ; make  ; make lut

pxlanalyzer: 
	source setenv_music.sh no_proxy ; cd $DIR/tapas/PxlAnalyzer/ ; make

clean: skimmer_clean utils_clean roi_clean pxlanalyzer_clean

skimmer_clean: 
	source setenv_music.sh no_proxy ; cd $(DIR)/CMSSW_10_6_29/src ; cmsenv ; scram build clean

utils_clean: 
	source setenv_music.sh no_proxy ; cd $(DIR)/tapas/MUSiC-Utils/ ; make clean

roi_clean: 
	source setenv_music.sh no_proxy ; cd $DIR/tapas/MUSiC-RoIScanner/ ; make  clean


pxlanalyzer_clean: 
	source setenv_music.sh no_proxy ; cd $DIR/tapas/PxlAnalyzer/ ; make clean
