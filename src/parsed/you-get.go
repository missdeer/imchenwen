package main

import (
	"encoding/json"
	"io/ioutil"
	"log"
	"os/exec"
	"strconv"
)

type YouGetStream struct {
	Container    string   `json:"container"`
	VideoProfile string   `json:"video_profile"`
	Size         int      `json:"size"`
	URLs         []string `json:"src"`
	ITag         string   `json:"itag"`
	Mime         string   `json:"mime"`
	Quality      string   `json:"quality"`
	Type         string   `json:"type"`
	URL          string   `json:"url"`
}

type YouGetJSON struct {
	Site    string                  `json:"site"`
	Title   string                  `json:"title"`
	URL     string                  `json:"url"`
	Streams map[string]YouGetStream `json:"streams"`
}

func parseByYouGetJSON(u string, r chan interface{}) {
	tryCount := 1
startProcess:
	cmd := exec.Command("you-get", "-i", "--json", u)
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Println("you-get stdout pipe failed", err)
		r <- nil
		return
	}
	err = cmd.Start()
	if err != nil {
		log.Println("starting you-get failed", err)
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
		log.Println("waiting for you-get exiting failed", err)
		r <- nil
		return
	}

	var res interface{}
	err = json.Unmarshal(content, &res)
	if err != nil {
		log.Println("unmarshalling you-get output JSON failed", err)
		r <- nil
		return
	}
	r <- res
}

func parseByYouGet(u string, r chan *CmdResponse) {
	tryCount := 1
startProcess:
	cmd := exec.Command("you-get", "-i", "--json", u)
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Println("you-get stdout pipe failed", err)
		r <- nil
		return
	}
	err = cmd.Start()
	if err != nil {
		log.Println("starting you-get failed", err)
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
		log.Println("waiting for you-get exiting failed", err)
		r <- nil
		return
	}

	var res YouGetJSON
	err = json.Unmarshal(content, &res)
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

func getYouGetParseNativeJSONResult(u string, res chan *NativeJSONResult) {
	r := make(chan interface{})
	go parseByYouGetJSON(u, r)
	result := &NativeJSONResult{Service: "YouGet"}
	result.Result = <-r
	res <- result
}

func getYouGetParseCmdResult(u string, res chan *CmdResult) {
	r := make(chan *CmdResponse)
	go parseByYouGet(u, r)
	result := &CmdResult{Service: "YouGet"}
	result.Result = <-r
	res <- result
}
