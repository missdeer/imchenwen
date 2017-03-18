package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os/exec"
	"strconv"
)

func getYoutubeDLParseNativeJSONResult(u string, res chan *NativeJSONResult) {
	r := make(chan interface{})
	go parseByYoutubeDLJSON(u, r)
	result := &NativeJSONResult{Service: "YoutubeDL"}
	result.Result = <-r
	res <- result
}

func getYoutubeDLParseCmdResult(u string, res chan *CmdResult) {
	r := make(chan *CmdResponse)
	go parseByYoutubeDL(u, r)
	result := &CmdResult{Service: "YoutubeDL"}
	result.Result = <-r
	res <- result
}

func parseByYoutubeDLJSON(u string, r chan interface{}) {
	tryCount := 1
startProcess:
	cmd := exec.Command("youtube-dl", "--skip-download", "--print-json", u)
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Println("youtube-dl stdout pipe failed", err)
		r <- nil
		return
	}
	err = cmd.Start()
	if err != nil {
		log.Println("starting youtube-dl failed", err)
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
		log.Println("waiting for youtube-dl exiting failed", err)
		r <- nil
		return
	}

	var res interface{}
	err = json.Unmarshal(content, &res)
	if err != nil {
		log.Println("unmarshalling youtube-dl output JSON failed", err)
		r <- nil
		return
	}
	r <- res
}

type YoutubeDLFormat struct {
	Format     string `json:"format"`
	URL        string `json:"url"`
	FormatNote string `json:"format_note"`
	Ext        string `json:"ext"`
	FileSize   int    `json:"filesize"`
	FormatID   string `json:"format_id"`
	Height     int    `json:"height"`
	Width      int    `json:"width"`
	FPS        int    `json:"fps"`
	VideoCodec string `json:"vcodec"`
}

type YoutubeDLJSON struct {
	RequestedFormats []YoutubeDLFormat `json:"requested_formats"`
	Formats          []YoutubeDLFormat `json:"formats"`
	Description      string            `json:"description"`
	Title            string            `json:"title"`
	FullTitle        string            `json:"fulltitle"`
	Extractor        string            `json:"extractor"`
	ExtractorKey     string            `json:"extractor_key"`
}

func parseByYoutubeDL(u string, r chan *CmdResponse) {
	tryCount := 1
startProcess:
	cmd := exec.Command("youtube-dl", "--skip-download", "--print-json", u)
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Println("youtube-dl stdout pipe failed", err)
		r <- nil
		return
	}
	err = cmd.Start()
	if err != nil {
		log.Println("starting youtube-dl failed", err)
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
		log.Println("waiting for youtube-dl exiting failed", err)
		r <- nil
		return
	}

	var res YoutubeDLJSON
	err = json.Unmarshal(content, &res)
	if err != nil {
		log.Println("unmarshalling youtube-dl output JSON failed", err)
		r <- nil
		return
	}
	resp := &CmdResponse{
		Site:    res.ExtractorKey,
		Title:   res.Title,
		Streams: []*Stream{},
	}
	for _, f := range res.Formats {
		if f.VideoCodec == "none" {
			continue
		}
		stream := &Stream{
			Format:    f.Format,
			ITag:      f.FormatID,
			Size:      strconv.Itoa(f.FileSize),
			Container: f.Ext,
			Quality:   fmt.Sprintf("%d x %d @%d", f.Width, f.Height, f.FPS),
			RealURLs:  []string{f.URL},
		}
		resp.Streams = append(resp.Streams, stream)
	}
	r <- resp
}
