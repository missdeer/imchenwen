package main

import (
	"bufio"
	"log"
	"net/url"
	"os/exec"
	"strings"
	"time"
)

func sniffByPhantomJS(u string) (postBody string, headers map[string]string) {
	tryCount := 1
startProcess:
	cmd := exec.Command(findExecutable("phantomjs"), findInApplicationDirectory("sniff.js"), u)
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Println("ykdl stdout pipe failed", err)
		return
	}
	err = cmd.Start()
	if err != nil {
		log.Println("starting phantomjs failed", err)
		return
	}

	scanner := bufio.NewScanner(stdout)
	scanner.Split(bufio.ScanLines)
	for scanner.Scan() {
		line := convertString(scanner.Text())
		if strings.HasPrefix(line, "Set-Cookie: ") {
			if len(headers) == 0 {
				headers = make(map[string]string)
			}
			line = line[len("Set-Cookie: "):]
			headers["Cookie"] = strings.Split(line, ";")[0]
		} else {
			postBody = line
		}
	}
	err = cmd.Wait()
	if err != nil {
		if tryCount < 3 {
			tryCount++
			goto startProcess
		}
		log.Println("waiting for phantomjs exiting failed", err)
		return
	}

	return
}

func parseByAikanTV(u string, r chan *CmdResponse) {
	getValues := url.Values{
		"url": {u},
	}

	parseURL := "https://aikan-tv.com/tong.php?"
	if strings.HasPrefix(u, "http://www.iqiyi.com") {
		parseURL = "https://aikan-tv.com/qy.php?"
	}
	parseURL = parseURL + getValues.Encode()

	retry := 0
startSniff:
	// let PhantomJS get post data
	postBody, headers := sniffByPhantomJS(parseURL)
	if postBody == "" {
		log.Println("get empty output from PhantomJS")
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			goto startSniff
		}
		r <- nil
		return
	}
	log.Println(postBody)
	streams := postRequest("https://aikan-tv.com/qq396774785.php", postBody, headers)
	if len(streams) > 0 {
		req, _ := url.Parse(u)
		resp := &CmdResponse{
			Site:    req.Host,
			Title:   "VIP video",
			Streams: streams,
		}
		r <- resp
		return
	}

	r <- nil
}

func getAikanTVParseCmdResult(u string, res chan *CmdResult) {
	r := make(chan *CmdResponse)
	go parseByAikanTV(u, r)
	result := &CmdResult{Service: "AikanTV"}
	result.Result = <-r
	res <- result
}
