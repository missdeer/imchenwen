package main

import (
	"encoding/json"
	"io/ioutil"
	"log"
	"os/exec"
	"strconv"
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
	err = json.Unmarshal(convertByteArray(content), &res)
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

	var res YouGetJSON
	err = json.Unmarshal(convertByteArray(content), &res)
	if err != nil {
		log.Println("unmarshalling ykdl output JSON failed", err)
		r <- nil
		return
	}
	resp := &CmdResponse{
		Site:    res.Site,
		Title:   res.Title,
		Streams: []*Stream{},
	}
	for _, s := range res.Streams {
		stream := &Stream{
			ITag:         s.ITag,
			Quality:      s.Quality,
			VideoProfile: s.VideoProfile,
			Size:         strconv.Itoa(s.Size),
			Container:    s.Container,
			RealURLs:     s.URLs,
		}
		if s.URL != "" {
			stream.RealURLs = append(stream.RealURLs, s.URL)
		}
		resp.Streams = append(resp.Streams, stream)
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
