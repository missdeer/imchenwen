package main

import (
	"bufio"
	"encoding/json"
	"log"
	"os/exec"
	"strings"
	"sync"
)

func getRealURLsByYouGet(u string, s *Stream, wg *sync.WaitGroup) {
	defer wg.Done()
	downloadURL := strings.Replace(s.DownloadWith, "[URL]", u, -1)
	c := strings.Split(downloadURL, " ")
	c[0] = "-u"
	cmd := exec.Command("you-get", c...)
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Println("you-get stdout pipe failed", err)
		return
	}
	err = cmd.Start()
	if err != nil {
		log.Println("starting you-get failed", err)
		return
	}

	scanner := bufio.NewScanner(stdout)
	scanner.Split(bufio.ScanLines)
	start := false
	var rawURLs string
	for scanner.Scan() {
		line := scanner.Text()
		if start {
			rawURLs += line
			continue
		}
		if strings.HasPrefix(line, "Real URL:") {
			start = true
		}
	}
	err = cmd.Wait()
	if err != nil {
		log.Println("waiting for you-get exiting failed", err)
		return
	}
	rawURLs = strings.Replace(rawURLs, "'", "\"", -1)
	err = json.Unmarshal([]byte(rawURLs), &s.RealURLs)
	if err != nil {
		log.Println("unmarshalling json failed", err, rawURLs)
		return
	}
}

func parseByYouGet(u string, r chan *CmdResponse) {
	cmd := exec.Command("you-get", "-i", u)
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
			if strings.HasPrefix(line, "streams:") {
				status = 3
			}
		case 3:
			if strings.Contains(line, "- format:") {
				pos := strings.IndexByte(line, byte(':')) + 1
				for line[pos] == ' ' {
					pos++
				}
				stream = &Stream{
					Format: line[pos:],
				}
				resp.Streams = append(resp.Streams, stream)
				status = 4
			}
		case 4:
			if strings.Contains(line, "container:") {
				pos := strings.IndexByte(line, byte(':')) + 1
				for line[pos] == ' ' {
					pos++
				}
				stream.Container = line[pos:]
				status = 5
			}
		case 5:
			if strings.Contains(line, "video-profile:") {
				pos := strings.IndexByte(line, byte(':')) + 1
				for line[pos] == ' ' {
					pos++
				}
				stream.VideoProfile = line[pos:]
				status = 6
			}
		case 6:
			if strings.Contains(line, "size:") {
				pos := strings.IndexByte(line, byte(':')) + 1
				for line[pos] == ' ' {
					pos++
				}
				stream.Size = line[pos:]
				status = 7
			}
		case 7:
			if strings.Contains(line, "# download-with:") {
				pos := strings.IndexByte(line, byte(':')) + 1
				for line[pos] == ' ' {
					pos++
				}
				stream.DownloadWith = line[pos:]
				status = 3
			}
		}
	}

	var wg sync.WaitGroup
	wg.Add(len(resp.Streams))
	for _, s := range resp.Streams {
		go getRealURLsByYouGet(u, s, &wg)
	}
	wg.Wait()
	err = cmd.Wait()
	if err != nil {
		log.Println("waiting for you-get exiting failed", err)
		r <- nil
		return
	}
	r <- resp
}
