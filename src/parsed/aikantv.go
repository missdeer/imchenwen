package main

import (
	"io/ioutil"
	"log"
	"net/http"
	"net/url"
	"regexp"
	"time"
)

func parseByAikanTV(u string, r chan *CmdResponse) {
	retry := 0

	getValues := url.Values{
		"url": {u},
	}

	req, err := http.NewRequest("GET", "https://aikan-tv.com/tong.php?"+getValues.Encode(), nil)
	if err != nil {
		log.Println("Could not parse mt2t request:", err)
		r <- nil
		return
	}

	req.Header.Set("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:45.0) Gecko/20100101 Firefox/45.0")
	req.Header.Set("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8")
	req.Header.Set("accept-language", `en-US,en;q=0.8`)
	req.Header.Set("Upgrade-Insecure-Requests", "1")
doRequest:
	resp, err := client.Do(req)
	if err != nil {
		log.Println("Could not send mt2t request:", err)
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
		log.Println("mt2t request not 200")
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
		log.Println("cannot read mt2t content", err)
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			goto doRequest
		}
		r <- nil
		return
	}

	regex := regexp.MustCompile(`\$\.post\("([^"]+)",\s+\{\s*"id":\s*"([^"]+)","type":\s*"([^"]+)","siteuser":\s*"([^"]*)","md5":\s*"([^"]+)"\s*\}`)
	ss := regex.FindAllSubmatch(content, -1)
	for _, match := range ss {
		path := string(match[1])
		id := string(match[2])
		typ := string(match[3])
		siteuser := string(match[4])
		md5 := string(match[5])
		postBody := url.Values{
			"id":       {id},
			"type":     {typ},
			"siteuser": {siteuser},
			"md5":      {md5},
		}
		m3u8 := postRequest(postBody.Encode(), "https://aikan-tv.com/", path)
		if len(m3u8) > 0 {
			req, _ := url.Parse(u)
			resp := &CmdResponse{
				Site:  req.Host,
				Title: "VIP video",
				Streams: []*Stream{
					&Stream{
						RealURLs: m3u8,
					},
				},
			}
			r <- resp
			return
		}
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
