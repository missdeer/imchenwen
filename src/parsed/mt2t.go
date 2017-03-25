package main

import (
	"bufio"
	"encoding/json"
	"encoding/xml"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"net/url"
	"regexp"
	"strings"
	"time"
)

var (
	client = &http.Client{Timeout: 120 * time.Second}
)

func extractURLsFromXML(u string) (res []string) {
	retry := 0
doRequest:
	resp, err := http.Get(u)
	if err != nil {
		log.Println("Could not send login request:", err)
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			goto doRequest
		}
		return
	}

	defer resp.Body.Close()

	content, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		log.Println("cannot read xml content", err)
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			goto doRequest
		}
		return
	}
	type Video struct {
		File   string `xml:"file"`
		Size   int    `xml:"size"`
		Length int    `xml:"second"`
	}
	type Result struct {
		XMLName   xml.Name `xml:"ckplayer"`
		FlashVars string   `xml:"flashvars"`
		Videos    []Video  `xml:"video"`
	}

	var vv Result
	if err = xml.Unmarshal(content, &vv); err != nil {
		log.Println("unmarshalling XML content failed", err)
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			goto doRequest
		}
		return
	}
	for _, v := range vv.Videos {
		res = append(res, v.File)
	}
	return
}

func extractURLsFromM3U8(u string) (res []string) {
	retry := 0
doRequest:
	resp, err := http.Get(u)
	if err != nil {
		log.Println("Could not send login request:", err)
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			goto doRequest
		}
		return
	}

	defer resp.Body.Close()
	req, _ := url.Parse(u)

	scanner := bufio.NewScanner(resp.Body)
	for scanner.Scan() {
		line := scanner.Text()
		if strings.HasPrefix(line, "http://") || strings.HasPrefix(line, "https://") {
			res = append(res, line)
			continue
		}
		if !strings.HasPrefix(line, "#") {
			l := fmt.Sprintf("%s://%s/%s", req.Scheme, req.Host, line)
			res = append(res, l)
		}
	}
	return
}

func postRequest(postBody string, host string, path string) (res []string) {
	req, err := http.NewRequest("POST", host+path, strings.NewReader(postBody))
	if err != nil {
		log.Println("Could not parse login request:", err)
		return
	}

	req.Header.Set("Referer", host+path)
	req.Header.Set("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:45.0) Gecko/20100101 Firefox/45.0")
	req.Header.Set("Accept", "application/json, text/javascript, */*")
	req.Header.Set("Content-Type", "application/x-www-form-urlencoded")

	retry := 0
doRequest:
	resp, err := client.Do(req)
	if err != nil {
		log.Println("Could not send post request:", err)
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			goto doRequest
		}
		return
	}

	defer resp.Body.Close()
	content, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		log.Println("cannot read post content", err)
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			goto doRequest
		}
		return
	}

	type YunAPIResult struct {
		URL     string `json:"url"`
		Ext     string `json:"ext"`
		Message string `json:"msg"`
	}
	var result YunAPIResult
	if err = json.Unmarshal(content, &result); err != nil {
		log.Println("unmarshalling post content failed", err)
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			goto doRequest
		}
		return
	}

	fmt.Println(result)
	if result.Message == "200" || result.Message == "ok" {
		r, _ := url.QueryUnescape(result.URL)
		if result.Ext == "xml" {
			return extractURLsFromXML(r)
		}
		if result.Ext == "m3u8" || result.Ext == "m3u8_list" {
			return extractURLsFromM3U8(r)
		}
		return []string{r}
	}

	return
}

func parseByMT2T(u string, r chan *CmdResponse) {
	retry := 0
	getValues := url.Values{
		"url": {u},
	}

	req, err := http.NewRequest("GET", "http://mt2t.com/yun?"+getValues.Encode(), nil)
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

	regex := regexp.MustCompile(`\$\.post\("([\w\/]+)",\s+\{\s+"url":\s+"[^"]+","key":"(\w+)","from":"\w+"\s\}`)
	ss := regex.FindAllSubmatch(content, -1)
	for _, match := range ss {
		path := string(match[1])
		key := string(match[2])

		postBody := url.Values{
			"url":  {u},
			"key":  {key},
			"from": {"mt2t"},
		}
		m3u8 := postRequest(postBody.Encode(), "http://mt2t.com", path)
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

func getMT2TParseCmdResult(u string, res chan *CmdResult) {
	r := make(chan *CmdResponse)
	go parseByMT2T(u, r)
	result := &CmdResult{Service: "MT2T"}
	result.Result = <-r
	res <- result
}
