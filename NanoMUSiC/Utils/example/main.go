package main

import "fmt"
import toml "github.com/pelletier/go-toml/v2"

type MyConfig struct {
	Version int
	Name    string
	Tags    []string
}

func main() {
	doc := `
version = 2
#name = "go-toml"
tags = ["go", "2"]
`

	var cfg MyConfig
	err := toml.Unmarshal([]byte(doc), &cfg)
	if err != nil {
		panic(err)
	}
	fmt.Println("version:", cfg.Version)
	fmt.Println("name:", cfg.Name)
	fmt.Println("tags:", cfg.Tags)

	for el:= range cfg.Tags { 
        fmt.Println(el) 
    } 
}
