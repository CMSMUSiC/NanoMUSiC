package main

import (
	"fmt"
	"math/rand"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"
	"time"

	toml "github.com/pelletier/go-toml/v2"
)

type Config struct {
	Output      string
	Is_data     bool
	Year        string
	Era         string
	Process     string
	Dataset     string
	Is_crab_job bool
	Input_files []string
}

func make_include_string() string {
	include_patterns := []string{
		"nCorrT1METJet",
		"CorrT1METJet_*",
		"nElectron",
		"Electron_pt",
		"Electron_eta",
		"Electron_phi",
		"Electron_deltaEtaSC",
		"Electron_cutBased",
		"Electron_cutBased_HEEP",
		"Electron_scEtOverPt",
		"Electron_dEscaleUp",
		"Electron_dEscaleDown",
		"Electron_dEsigmaUp",
		"Electron_dEsigmaDown",
		"Electron_genPartIdx",
		"Flag_*",
		"nGenJet",
		"GenJet_pt",
		"GenJet_eta",
		"GenJet_phi",
		"GenPart_*",
		"nGenPart",
		"Generator_*",
		"HLT_*",
		"nJet",
		"Jet_pt",
		"Jet_eta",
		"Jet_phi",
		"Jet_mass",
		"Jet_jetId",
		"Jet_btagDeepFlavB",
		"Jet_rawFactor",
		"Jet_area",
		"Jet_genJetIdx",
		"L1PreFiringWeight_Up",
		"L1PreFiringWeight_Dn",
		"L1PreFiringWeight_Nom",
		"LHE_*",
		"nLHEPart",
		"LHEPart_*",
		"LHEPdfWeight",
		"nLHEPdfWeight",
		"LHEReweightingWeight",
		"nLHEReweightingWeight",
		"LHEScaleWeight",
		"nLHEScaleWeight",
		"LHEWeight_originalXWGTUP",
		"nMuon",
		"Muon_pt",
		"Muon_eta",
		"Muon_phi",
		"Muon_tightId",
		"Muon_highPtId",
		"Muon_pfRelIso04_all",
		"Muon_tkRelIso",
		"Muon_tunepRelPt",
		"Muon_highPurity",
		"Muon_genPartIdx",
		"nPSWeight",
		"PSWeight",
		"nPhoton",
		"Photon_pt",
		"Photon_eta",
		"Photon_phi",
		"Photon_isScEtaEB",
		"Photon_isScEtaEE",
		"Photon_cutBased",
		"Photon_pixelSeed",
		"Photon_dEscaleUp",
		"Photon_dEscaleDown",
		"Photon_dEsigmaUp",
		"Photon_dEsigmaDown",
		"Photon_genPartIdx",
		"MET_*",
		"RawMET_*",
		"Pileup_nTrueInt",
		"Tau_pt",
		"Tau_eta",
		"Tau_phi",
		"Tau_dz",
		"Tau_mass",
		"Tau_idDeepTau2017v2p1VSe",
		"Tau_idDeepTau2017v2p1VSjet",
		"Tau_idDeepTau2017v2p1VSmu",
		"Tau_decayMode",
		"Tau_genPartIdx",
		"Tau_genPartFlav",
		"btagWeight_DeepCSVB",
		"event",
		"fixedGridRhoFastjetAll",
		"genWeight",
		"luminosityBlock",
		"run",
	}

	return strings.Join(include_patterns[:], ",")
}

func main() {
	fmt.Printf("\n\n[NanoAOD Skimmer - %s] Starting ... \n", time.Now().UTC())

	inclusion_list := make_include_string()

	data, err := os.ReadFile("config.toml")
	if err != nil {
		panic(err)
	}

	fmt.Println("\n\nConfig file:")
	fmt.Println(string(data))

	var cfg Config
	err = toml.Unmarshal(data, &cfg)
	if err != nil {
		panic(err)
	}

	output_file := "nano_music.root"

	matches, err := filepath.Glob("./*nano_music*.root")
	if err != nil {
		panic(err)
	}

	if len(matches) > 0 {
		cmd := exec.Command("rm", append([]string{"-rf"}, matches...)...)
		fmt.Printf("\n\n[NanoAOD Skimmer - %s] Cleanning: %s \n", time.Now().UTC(), cmd.String())
		_, err = cmd.Output()
		if err != nil {
			panic(err)
		}
	}

	// copy files
	var wg_copy sync.WaitGroup
	temp_raw_files_ch := make(chan string, len(cfg.Input_files))
	for _, f := range cfg.Input_files {
		wg_copy.Add(1)
		go func(f string, out chan<- string) {
			defer wg_copy.Done()
			this_output_file := fmt.Sprintf("raw_nano_music_%s.root", fmt.Sprint(rand.Int63()))

			// 209 is the maximum compression level
			cmd := exec.Command("xrdcp", "--silent", f, this_output_file)

			cmd.Stdout = os.Stdout
			cmd.Stderr = os.Stderr
			fmt.Printf("\n\n[NanoAOD Skimmer - %s] Copying to job scratch area: %s \n", time.Now().UTC(), cmd.String())
			err := cmd.Run()
			if err != nil {
				fmt.Printf("Panicing when copying file: %s \n", f)
				panic(err)
			}
			out <- this_output_file
		}(f, temp_raw_files_ch)
	}

	raw_files := []string{}
	for i := 0; i < len(cfg.Input_files); i++ {
		raw_files = append(raw_files, <-temp_raw_files_ch)
	}

	fmt.Printf("\n\n[NanoAOD Skimmer - %s] Awaiting ... \n", time.Now().UTC())
	wg_copy.Wait()

	cmd := exec.Command("ls", "-lha")
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	fmt.Printf("\n\n[NanoAOD Skimmer - %s]Listing: %s \n", time.Now().UTC(), cmd.String())
	err = cmd.Run()
	if err != nil {
		panic(err)
	}

	// skim files
	temp_files := []string{}
	for _, f := range raw_files {
		func(f string) {
			this_output_file := fmt.Sprintf("nano_music_%s.root", fmt.Sprint(rand.Int63()))

			// 209 is the maximum compression level
			cmd := exec.Command("rooteventselector", "--recreate", "--compress", "209", "-e", "*", "-i", inclusion_list, fmt.Sprintf("%s:Events", f), this_output_file)

			cmd.Stdout = os.Stdout
			cmd.Stderr = os.Stderr
			fmt.Printf("\n\n[NanoAOD Skimmer - %s] Skimming: %s \n", time.Now().UTC(), cmd.String())
			err := cmd.Run()
			if err != nil {
				fmt.Printf("ERROR: Could not process file: %s \n", f)
				panic(err)
			}
			temp_files = append(temp_files, this_output_file)

			// cleanning
			cmd = exec.Command("rm", f)
			_, err = cmd.Output()
			if err != nil {
				fmt.Printf("ERROR: Could not remove raw file: %s \n", f)
				panic(err)
			}
		}(f)
	}

	hadd_args := []string{"-ff", output_file}
	hadd_args = append(hadd_args, temp_files...)

	fmt.Printf("\n\n[NanoAOD Skimmer - %s] Awaiting ... \n", time.Now().UTC())

	cmd = exec.Command("hadd", hadd_args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	fmt.Printf("\n\n[NanoAOD Skimmer - %s] Merging: %s \n", time.Now().UTC(), cmd.String())
	err = cmd.Run()
	if err != nil {
		panic(err)
	}

	cmd = exec.Command("ls", "-lha")
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	fmt.Printf("\n\n[NanoAOD Skimmer - %s]Listing: %s \n", time.Now().UTC(), cmd.String())
	err = cmd.Run()
	if err != nil {
		panic(err)
	}

	fmt.Printf("[NanoAOD Skimmer - %s] Done \n", time.Now().UTC())
}
