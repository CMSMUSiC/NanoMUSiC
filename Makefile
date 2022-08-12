ifndef MUSIC_IS_SET_ENV
$(error MUSiC envs are not set. First run: source setenv_music.sh)
endif

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

all: utils roi music

utils: 
	@echo "${YELLOW}Building $@ ...${NC}"
	cd ./NanoMUSiC/MUSiC-Utils/ && $(MAKE)
	@echo "${GREEN} $@ done ...${NC}"

roi:  
	@echo "${YELLOW}Building $@ ...${NC}"
	$(MAKE) -C ./NanoMUSiC/MUSiC-RoIScanner/ 
	@echo "${GREEN} $@ done ...${NC}"

lut:  
	@echo "${YELLOW}Building $@ ...${NC}"
	$(MAKE) -C ./NanoMUSiC/MUSiC-RoIScanner/ 
	$(MAKE) -C ./NanoMUSiC/MUSiC-RoIScanner/ lut
	@echo "${GREEN} $@ done ...${NC}"

music:  
	@echo "${YELLOW}Building $@ ...${NC}"
	$(MAKE) -C ./NanoMUSiC/ 
	@echo "${GREEN} $@ done ...${NC}"
	
clean: utils_clean roi_clean music_clean

utils_clean:  
	@echo "${YELLOW}Cleanning $@ ...${NC}"
	$(MAKE) -C ./NanoMUSiC/MUSiC-Utils/ clean
	@echo "${GREEN} $@ done ...${NC}"

roi_clean:   
	@echo "${YELLOW}Cleanning $@ ...${NC}"
	$(MAKE) -C ./NanoMUSiC/MUSiC-RoIScanner/ clean
	@echo "${GREEN} $@ done ...${NC}"

music_clean:  
	@echo "${YELLOW}Cleanning $@ ...${NC}"
	$(MAKE) -C ./NanoMUSiC clean
	@echo "${GREEN} $@ done ...${NC}"
