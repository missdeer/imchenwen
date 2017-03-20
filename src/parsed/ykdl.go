package main

import (
	"bufio"
	"encoding/json"
	"io/ioutil"
	"log"
	"os/exec"
	"strings"
)

func parseByYKDLJSON(u string, r chan interface{}) {
	tryCount := 1
startProcess:
	cmd := exec.Command("ykdl", "-i", "--json", u)
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Println("ykdl stdout pipe failed", err)
		r <- nil
		return
	}
	err = cmd.Start()
	if err != nil {
		log.Println("starting ykdl failed", err)
		r <- nil
		return
	}

	content, err := ioutil.ReadAll(stdout)
	err = cmd.Wait()
	if err != nil {
		if tryCount < 3 {
			tryCount++
			goto startProcess
		}
		log.Println("waiting for ykdl exiting failed", err)
		r <- nil
		return
	}

	var res interface{}
	err = json.Unmarshal(content, &res)
	if err != nil {
		log.Println("unmarshalling ykdl output JSON failed", err)
		r <- nil
		return
	}
	r <- res
}

func parseByYKDL(u string, r chan *CmdResponse) {
	tryCount := 1
startProcess:
	cmd := exec.Command("ykdl.py", "-i", u)
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Println("ykdl stdout pipe failed", err)
		r <- nil
		return
	}
	err = cmd.Start()
	if err != nil {
		log.Println("starting ykdl failed", err)
		r <- nil
		return
	}

	resp := &CmdResponse{}
	var stream *Stream
	scanner := bufio.NewScanner(stdout)
	scanner.Split(bufio.ScanLines)
	status := 0
	for scanner.Scan() {
		line := scanner.Text()
		switch status {
		case 0:
			if strings.HasPrefix(line, "site:") {
				pos := strings.IndexByte(line, byte(' '))
				for line[pos] == ' ' {
					pos++
				}
				resp.Site = line[pos:]
				status = 1
			}
		case 1:
			if strings.HasPrefix(line, "title:") {
				pos := strings.IndexByte(line, byte(' '))
				for line[pos] == ' ' {
					pos++
				}
				resp.Title = line[pos:]
				status = 2
			}
		case 2:
			if strings.HasPrefix(line, "artist:") {
				pos := strings.IndexByte(line, byte(' '))
				for line[pos] == ' ' {
					pos++
				}
				resp.Artist = line[pos:]
				status = 3
			}
		case 3:
			if strings.HasPrefix(line, "streams:") {
				status = 4
			}
		case 4:
			if strings.Contains(line, "- format:") {
				pos := strings.IndexByte(line, byte(':')) + 1
				for line[pos] == ' ' {
					pos++
				}
				stream = &Stream{
					Format: line[pos:],
				}
				resp.Streams = append(resp.Streams, stream)
				status = 5
			}
		case 5:
			if strings.Contains(line, "container:") {
				pos := strings.IndexByte(line, byte(':')) + 1
				for line[pos] == ' ' {
					pos++
				}
				stream.Container = line[pos:]
				status = 6
			}
		case 6:
			if strings.Contains(line, "video-profile:") {
				pos := strings.IndexByte(line, byte(':')) + 1
				for line[pos] == ' ' {
					pos++
				}
				stream.VideoProfile = line[pos:]
				status = 7
			}
		case 7:
			if strings.Contains(line, "# download-with:") {
				pos := strings.IndexByte(line, byte(':')) + 1
				for line[pos] == ' ' {
					pos++
				}
				stream.DownloadWith = line[pos:]
				status = 8
			}
		case 8:
			if strings.HasPrefix(line, "Real urls:") {
				stream.RealURLs = []string{}
				status = 9
			}
		case 9:
			if strings.Contains(line, "- format:") {
				pos := strings.IndexByte(line, byte(':')) + 1
				for line[pos] == ' ' {
					pos++
				}
				stream = &Stream{
					Format: line[pos:],
				}
				resp.Streams = append(resp.Streams, stream)
				status = 5
				break
			}
			stream.RealURLs = append(stream.RealURLs, line)
		}
	}
	err = cmd.Wait()
	if err != nil {
		if tryCount < 3 {
			tryCount++
			goto startProcess
		}
		log.Println("waiting for ykdl exiting failed", err)
		r <- nil
		return
	}

	r <- resp
}

func getYKDLParseNativeJSONResult(u string, res chan *NativeJSONResult) {
	r := make(chan interface{})
	go parseByYKDLJSON(u, r)
	result := &NativeJSONResult{Service: "YKDL"}
	result.Result = <-r
	res <- result
}

func getYKDLParseCmdResult(u string, res chan *CmdResult) {
	r := make(chan *CmdResponse)
	go parseByYKDL(u, r)
	result := &CmdResult{Service: "YKDL"}
	result.Result = <-r
	res <- result
}
