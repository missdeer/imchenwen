package main

import (
	"io/ioutil"
	"log"
	"net/url"
	"os/exec"
	"strings"
	"time"
)

func sniffByPhantomJS(u string) string {
	tryCount := 1
startProcess:
	cmd := exec.Command(findExecutable("phantomjs"), findInApplicationDirectory("sniff.js"), u)
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Println("ykdl stdout pipe failed", err)
		return ""
	}
	err = cmd.Start()
	if err != nil {
		log.Println("starting phantomjs failed", err)
		return ""
	}

	content, err := ioutil.ReadAll(stdout)
	err = cmd.Wait()
	if err != nil {
		if tryCount < 3 {
			tryCount++
			goto startProcess
		}
		log.Println("waiting for phantomjs exiting failed", err)
		return ""
	}

	return string(convertByteArray(content))
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
	postBody := sniffByPhantomJS(parseURL)
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
	streams := postRequest(postBody, "https://aikan-tv.com/qq396774785.php")
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
