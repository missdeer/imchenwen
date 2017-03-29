package main

import (
	"errors"
	"os"
	"path/filepath"
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
	paths := strings.Split(pathEnv, ";")
	for _, p := range paths {
		if b, _ := fileExists(filepath.Join(p, name+".exe")); b {
			return filepath.Join(p, name+".exe")
		}
	}
	if executable, err := osext.Executable(); err == nil {
		p := filepath.Dir(executable)
		if b, _ := fileExists(filepath.Join(p, name+".exe")); b {
			return filepath.Join(p, name+".exe")
		}
	}

	return name
}

func findInApplicationDirectory(name string) string {
	if executable, err := osext.Executable(); err == nil {
		p := filepath.Dir(executable)
		if b, _ := fileExists(filepath.Join(p, name)); b {
			return filepath.Join(p, name)
		}
	}
	return name
}
