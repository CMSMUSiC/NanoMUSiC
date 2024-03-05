package main

import "fmt"
import "strings"
import "os"
import "math/rand"
import "os/exec"

import toml "github.com/pelletier/go-toml/v2"

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

func input_files(inputs []string) string {
	var sb strings.Builder
	for _, f := range inputs {
		sb.WriteString(f + ":Events ")
	}
	return strings.TrimRight(sb.String(), " ")
}

func main() {
	inclusion_list := "nMuon,Muon_*,HLT_*"

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

	cmd := exec.Command("rm", "-rf", output_file)
	fmt.Printf("Cleanning: %s \n", cmd.String())
	_, err = cmd.Output()
	if err != nil {
		panic(err)
	}

	hadd_args := []string{"-ff", output_file}
	for _, f := range cfg.Input_files {
		this_output_file := fmt.Sprintf("nano_music_%s.root", fmt.Sprint(rand.Int63()))
		cmd = exec.Command("rooteventselector", "--recreate", "--compress", "99999999", "-e", "*", "-i", inclusion_list, fmt.Sprintf("%s:Events", f), this_output_file)
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		fmt.Printf("Skimming: %s \n", cmd.String())
		err := cmd.Run()
		if err != nil {
			panic(err)
		}
		hadd_args = append(hadd_args, this_output_file)
	}

	cmd = exec.Command("hadd", hadd_args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	fmt.Printf("Merging: %s \n", cmd.String())
	err = cmd.Run()
	if err != nil {
		panic(err)
	}

	cmd = exec.Command("ls", "-lha", output_file)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	fmt.Printf("Listing: %s \n", cmd.String())
	err = cmd.Run()
	if err != nil {
		panic(err)
	}

}
