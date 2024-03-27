package cmd

import (
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"sync"

	"github.com/pelletier/go-toml"
	"github.com/urfave/cli"
)

type SubmitTask struct {
	Sample         string
	XSec           float64
	KFactor        float64
	FilterEff      float64
	XSecOrder      string
	ProcessGroup   string
	Is_signal      bool
	Is_data        bool
	Das_name_2018  []string
	Crab_task_name []string
	Output_files   []string
}

type SubmitTask struct {
	Sample         string
	XSec           float64
	KFactor        float64
	FilterEff      float64
	XSecOrder      string
	ProcessGroup   string
	Is_signal      bool
	Is_data        bool
	Das_name_2018  []string
	Crab_task_name []string
	Output_files   []string
}

func make_tasks_list(c *cli.Context) map[string]SubmitTask {
	var cfg map[string]SubmitTask

	content, err_read_file := ioutil.ReadFile(c.String("config-file"))
	if err_read_file != nil {
		panic(err_read_file)
	}

	err := toml.Unmarshal(content, &cfg)
	if err != nil {
		panic(err)
	}

	for sample, task := range cfg {
		cfg[sample].Sample = sample
	}

	return cfg
}

func submit_task_imp(task SubmitTask) {
	fmt.Printf("submiting with executable: %s...\n", task.executable)

	cmd := exec.Command("ls", "-lah", "/home/home1/institut_3a/silva/projects/music/NanoMUSiC/NanoMUSiC/Classification/clai")
	out, err := cmd.CombinedOutput()

	if err != nil {
		fmt.Printf("ERROR: Could not submit task.\n %s \n", string(out))
		log.Fatal(err)
		os.Exit(-1)
	}
	if task.is_debug {
		fmt.Println(string(out))
	}
}

func submit_task(ch chan SubmitTask, wg *sync.WaitGroup) {
	// cnosume a line
	for task := range ch {
		// do work
		submit_task_imp(task)
	}
	// we've exited the loop when the dispatcher closed the channel,
	// so now we can just signal the workGroup we're done
	wg.Done()
}

func Submit(c *cli.Context, is_debug bool) error {
	tasks := make_tasks_list(c)

	// create a channel for work "tasks"
	ch := make(chan SubmitTask)

	wg := sync.WaitGroup{}

	// start the workers
	for t := 0; t < c.Int("jobs"); t++ {
		wg.Add(1)
		go submit_task(ch, &wg)
	}

	// push the lines to the queue channel for processing
	for _, task := range tasks {
		ch <- task
	}

	// this will cause the workers to stop and exit their receive loop
	close(ch)

	// make sure they all exit
	wg.Wait()

	return nil
}
