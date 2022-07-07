DIR := ${CURDIR}

all: skimmer utils roi pxlanalyzer 

skimmer: setenv
	cd $(DIR)/CMSSW_10_6_29/src
	cmsenv
	scram build -j 8
	scram build

utils: setenv
	cd $(DIR)/tapas/MUSiC-Utils/
	make

roi: setenv
	cd $DIR/tapas/MUSiC-RoIScanner/
	make 
	make lut

pxlanalyzer: setenv
	cd $DIR/tapas/PxlAnalyzer/
	make

clean: skimmer_clean utils_clean roi_clean pxlanalyzer_clean

skimmer_clean: setenv
	cd $(DIR)/CMSSW_10_6_29/src
	cmsenv
	scram build clean

utils_clean: setenv
	cd $(DIR)/tapas/MUSiC-Utils/
	make clean

roi_clean: setenv
	cd $DIR/tapas/MUSiC-RoIScanner/
	make  clean


pxlanalyzer_clean: setenv
	cd $DIR/tapas/PxlAnalyzer/
	make clean

setenv:
	source setenv_music.sh
