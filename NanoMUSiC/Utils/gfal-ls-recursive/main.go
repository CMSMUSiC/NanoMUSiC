package main

import "fmt"
import "os"
import "os/exec"
import "strings"
import "sync"

func gfal_ls(wg *sync.WaitGroup, input_path string) {
	defer wg.Done()
	if string(input_path[len(input_path)-1]) != "/" {
		input_path += "/"
	}

	cmd := exec.Command("gfal-ls", "-l", input_path)
	out, err := cmd.CombinedOutput()
	if err != nil {
		fmt.Println(string(out))
		panic(err)
	}

	lines := strings.Split(string(out), "\n")
	for _, l := range lines {
		props := strings.Fields(l)
		if len(props) > 0 {
			is_dir := string(props[0][0]) == "d"
			name := props[len(props)-1]
			this_path := input_path + name
			fmt.Println(this_path)
			if is_dir {
				wg.Add(1)
				go gfal_ls(wg, this_path)
			}
		}
	}
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("ERROR: Could not list files. Input path is empty.")
		fmt.Printf("Usage: %s INPUT_PATH\n", os.Args[0])
		os.Exit(-1)
	}

	input_path := os.Args[1]

	wg := new(sync.WaitGroup)
	wg.Add(1)
	go gfal_ls(wg, input_path)
	wg.Wait()

}
