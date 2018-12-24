package main

import (
	"bytes"
	"io"
	"os"
	"sync/atomic"
)

// LocalPipe read data from internet, provide io.Reader for gin.Context and save data to local file
type LocalPipe struct {
	b   bytes.Buffer
	f   *os.File
	end atomic.Value
}

// End mark as all data received
func (lp *LocalPipe) End() {
	lp.end.Store(true)
}

// File set file path to save data
func (lp *LocalPipe) File(p string) error {
	f, e := os.Create(p)
	if e != nil {
		return e
	}
	lp.f = f
	return nil
}

// Read for gin.Context.DataFromReader
func (lp *LocalPipe) Read(p []byte) (n int, err error) {
	n, err = lp.b.Read(p)
	if err == io.EOF {
		if b := lp.end.Load().(bool); !b {
			return n, nil
		}
	}
	return n, err
}

// Write cache data for later Read request and save it to local file
func (lp *LocalPipe) Write(p []byte) (n int, err error) {
	n, err = lp.b.Write(p)
	lp.f.Write(p)
	return n, err
}

// Close close buffer
func (lp *LocalPipe) Close() error {
	return lp.f.Close()
}
