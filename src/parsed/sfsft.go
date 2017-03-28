package main

import (
	"io/ioutil"
	"log"
	"net/http"
	"net/url"
	"regexp"
	"time"
)

func parseBySFSFT(u string, r chan *CmdResponse) {
	retry := 0

	getValues := url.Values{
		"url": {u},
	}

	parseURL := "http://www.sfsft.com/index.php?"

	req, err := http.NewRequest("GET", parseURL+getValues.Encode(), nil)
	if err != nil {
		log.Println("Could not parse SFSFT request:", err)
		r <- nil
		return
	}

	req.Header.Set("Referer", "http://www.sfsft.com")
	req.Header.Set("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:45.0) Gecko/20100101 Firefox/45.0")
	req.Header.Set("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8")
	req.Header.Set("accept-language", `en-US,en;q=0.8`)
	req.Header.Set("Upgrade-Insecure-Requests", "1")
doRequest:
	resp, err := client.Do(req)
	if err != nil {
		log.Println("Could not send SFSFT request:", err)
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			goto doRequest
		}
		r <- nil
		return
	}

	defer resp.Body.Close()
	if resp.StatusCode != 200 {
		log.Println("SFSFT request not 200")
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			goto doRequest
		}
		r <- nil
		return
	}
	content, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		log.Println("cannot read SFSFT content", err)
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			goto doRequest
		}
		r <- nil
		return
	}

	regex := regexp.MustCompile(`url\s*:\s*'([\w\/]+)'`)
	ss := regex.FindAllSubmatch(content, -1)
	for _, match := range ss {
		urlParam := string(match[1])
		postBody := url.Values{
			"url": {urlParam},
			"up":  {"0"},
		}
		streams := postRequest(postBody.Encode(), "http://www.sfsft.com/", "api.php")
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
	}

	r <- nil
}

func getSFSFTParseCmdResult(u string, res chan *CmdResult) {
	r := make(chan *CmdResponse)
	go parseBySFSFT(u, r)
	result := &CmdResult{Service: "SFSFT"}
	result.Result = <-r
	res <- result
}
