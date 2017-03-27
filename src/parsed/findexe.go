// +build !windows

package main

import (
	"errors"
	"os"
	"path"
	"strings"

	"github.com/kardianos/osext"
)

func fileExists(path string) (bool, error) {
	stat, err := os.Stat(path)
	if err == nil {
		if stat.Mode()&os.ModeType == 0 {
			return true, nil
		}
		return false, errors.New(path + " exists but is not regular file")
	}
	if os.IsNotExist(err) {
		return false, nil
	}
	return false, err
}

func findExecutable(name string) string {
	pathEnv := os.Getenv("PATH")
	paths := strings.Split(pathEnv, ":")
	for _, p := range paths {
		if b, _ := fileExists(path.Join(p, name)); b {
			return path.Join(p, name)
		}
	}
	if executable, err := osext.Executable(); err == nil {
		p := path.Dir(executable)
		if b, _ := fileExists(path.Join(p, name)); b {
			return path.Join(p, name)
		}
	}

	return name
}
