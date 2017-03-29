package main

import (
	"log"
	"net/url"
	"time"
)

func parseBy47KS(u string, r chan *CmdResponse) {
	getValues := url.Values{
		"v": {u},
	}

	parseURL := "https://api.47ks.com/webcloud/?"
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

	streams := postRequest("https://api.47ks.com/config/webmain.php", postBody, headers)
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

func get47KSParseCmdResult(u string, res chan *CmdResult) {
	r := make(chan *CmdResponse)
	go parseBy47KS(u, r)
	result := &CmdResult{Service: "47KS"}
	result.Result = <-r
	res <- result
}
