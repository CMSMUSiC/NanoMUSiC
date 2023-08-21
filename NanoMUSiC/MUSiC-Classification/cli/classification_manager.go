package main

import (
	"fmt"
	"log"
	"os"

	"github.com/urfave/cli"

	"music/classification/cli/cmd"
)

// this is the dumbest and strangest thing I ever seen ...
func is_debug(c *cli.Context) bool { return !c.Bool("debug") }

func add(c *cli.Context) error {
	fmt.Println("add")
	return nil
}

func main() {
	fmt.Println("\n\n📶 [ MUSiC Classification ] 📶\n")

	app := cli.NewApp()
	app.Name = "MUSiC Classification Manager"
	app.Usage = "manages the MUSiC Classification workflow."

	is_debug_mode := false
	app.Flags = []cli.Flag{
		cli.BoolFlag{
			Name:        "debug",
			Usage:       "will run in debug mode",
			Destination: &is_debug_mode,
		},
	}

	app.Commands = []cli.Command{
		{
			Name:  "submit",
			Usage: "submit a classification task",
			Flags: []cli.Flag{cli.StringFlag{
				Name:  "executable",
				Value: "classification",
				Usage: "path to executable for classification",
			},cli.StringFlag{
				Name:  "config-file",
				Usage: "path to executable for classification",
				Required: true,
			}, cli.IntFlag{
				Name:  "jobs",
				Value: 60,
				Usage: "number of parallel jobs",
			},
			},
			Action: func(c *cli.Context) error {
				if is_debug_mode {
					fmt.Println("Will run in Debug Mode.")
				}
				return cmd.Submit(c, is_debug_mode)
			},
		},
		{
			Name: "add",
			// Aliases: []string{"a"},
			Usage: "add a task to the list",
			Action: func(c *cli.Context) error {
				return add(c)
			},
		},
	}

	err := app.Run(os.Args)
	if err != nil {
		log.Fatal(err)
	}
}
