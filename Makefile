ifndef MUSIC_IS_SET_ENV
$(error MUSiC envs are not set. First run: source setenv_music.sh)
endif

SHELL := /bin/bash
MAKEFILE_DIR := ${CURDIR}

all: utils roi pxlanalyzer 

utils: 
	cd $(MAKEFILE_DIR)/tapas/MUSiC-Utils/ ; make

roi: 
	cd $(MAKEFILE_DIR)/tapas/MUSiC-RoIScanner/ ; make 

lut: 
	cd $(MAKEFILE_DIR)/tapas/MUSiC-RoIScanner/ ; make ; make lut

pxlanalyzer: 
	cd $(MAKEFILE_DIR)/tapas/PxlAnalyzer/ ; make

clean: utils_clean roi_clean pxlanalyzer_clean

utils_clean: 
	cd $(MAKEFILE_DIR)/tapas/MUSiC-Utils/ ; make clean

roi_clean: 
	cd $(MAKEFILE_DIR)/tapas/MUSiC-RoIScanner/ ; make clean

pxlanalyzer_clean: 
	cd $(MAKEFILE_DIR)/tapas/PxlAnalyzer/ ; make clean
