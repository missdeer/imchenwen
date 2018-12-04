package main

import (
	"bytes"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

func main() {
	if (len(os.Args)) != 2 {
		fmt.Println("Usage:")
		fmt.Println("\tmacdeploy imchenwen.app")
		return
	}

	err := filepath.Walk(os.Args[1]+"/Contents/Frameworks", func(path string, info os.FileInfo, err error) error {
		if err != nil {
			fmt.Printf("prevent panic by handling failure accessing a path %q: %v\n", path, err)
			return err
		}
		if info.IsDir() {
			return nil
		}
		if !strings.HasSuffix(path, ".dylib") {
			return nil
		}
		cmd := exec.Command("otool", "-L", path)
		stdout, err := cmd.StdoutPipe()
		if err != nil {
			log.Fatal(err)
		}
		if err := cmd.Start(); err != nil {
			log.Fatal(err)
		}
		output, err := ioutil.ReadAll(stdout)
		if err != nil {
			log.Fatal(err)
		}
		if err := cmd.Wait(); err != nil {
			log.Fatal(err)
		}
		lines := bytes.Split(output, []byte("\n"))
		cmds := []string{}
		for _, line := range lines {
			l := bytes.TrimSpace(line)
			if bytes.HasPrefix(l, []byte("/usr/local")) {
				refs := bytes.Split(l, []byte(" "))
				if len(refs) < 2 {
					continue
				}
				name := bytes.Split(refs[0], []byte("/"))
				fmt.Printf("invoke install_name_tool -change %s @executable_path/../Frameworks/%s %s\n", string(refs[0]), string(name[len(name)-1]), path)
				cmds = append(cmds, fmt.Sprintf("-change %s @executable_path/../Frameworks/%s %s", string(refs[0]), string(name[len(name)-1]), path))
			}
		}
		if len(cmds) == 0 {
			fmt.Println(path, "is ok")
		}
		for _, cmd := range cmds {
			args := strings.Split(cmd, " ")
			cmd := exec.Command("install_name_tool", args...)
			err := cmd.Run()
			if err != nil {
				fmt.Println(err)
			} else {
				fmt.Println(path, "is processed")
			}
		}
		return nil
	})
	if err != nil {
		fmt.Printf("error walking the path %q: %v\n", os.Args[1]+"/Contents/Frameworks", err)
		return
	}
}
