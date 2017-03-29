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
	"strconv"
	"strings"
	"time"
)

var (
	client = &http.Client{Timeout: 120 * time.Second}
)

func parseFlashVars(vars string) map[string]string {
	beginPos := strings.Index(vars, "defa->")
	endPos := strings.Index(vars[beginPos+6:], "}")
	defa := vars[beginPos+6 : endPos+beginPos+6]
	beginPos = strings.Index(vars, "deft->")
	endPos = strings.Index(vars[beginPos+6:], "}")
	deft := vars[beginPos+6 : endPos+beginPos+6]
	res := make(map[string]string)

	defas := strings.Split(defa, "|")
	defts := strings.Split(deft, "|")
	for i, d := range defas {
		res[defts[i]] = d
	}
	return res
}

type Video struct {
	File   []string `xml:"file"`
	Size   []int    `xml:"size"`
	Length []int    `xml:"second"`
}

func doExtractURLsFromXML(u string) (res Video) {
	retry := 0
	request, err := http.NewRequest("GET", u, nil)
	if err != nil {
		log.Println("Could not parse get XML request:", err)
		return
	}

	request.Header.Set("Referer", u)
	request.Header.Set("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:45.0) Gecko/20100101 Firefox/45.0")
	request.Header.Set("Accept", "application/json, text/javascript, */*")
	request.Header.Set("Content-Type", "application/x-www-form-urlencoded")
doRequest:
	resp, err := client.Do(request)
	if err != nil {
		log.Println("Could not send getting XML request:", err)
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
	type Result struct {
		XMLName xml.Name `xml:"ckplayer"`
		Videos  Video    `xml:"video"`
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

	fmt.Println(len(vv.Videos.File), "videos:", vv.Videos)

	return vv.Videos
}

func extractURLsFromXML(u string) (res []*Stream) {
	retry := 0
	request, err := http.NewRequest("GET", u, nil)
	if err != nil {
		log.Println("Could not parse get XML request:", err)
		return
	}

	request.Header.Set("Referer", u)
	request.Header.Set("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:45.0) Gecko/20100101 Firefox/45.0")
	request.Header.Set("Accept", "application/json, text/javascript, */*")
	request.Header.Set("Content-Type", "application/x-www-form-urlencoded")
doRequest:
	resp, err := client.Do(request)
	if err != nil {
		log.Println("Could not send getting XML request:", err)
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
	type Result struct {
		XMLName   xml.Name `xml:"ckplayer"`
		FlashVars string   `xml:"flashvars"`
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

	vars := parseFlashVars(vv.FlashVars)
	for k, v := range vars {
		requestURL := u + "&" + v
		videos := doExtractURLsFromXML(requestURL)
		size := 0
		urls := []string{}
		for _, video := range videos.File {
			urls = append(urls, video)
		}
		for _, s := range videos.Size {
			size += s
		}
		res = append(res, &Stream{
			Quality:  k,
			Size:     strconv.Itoa(size),
			RealURLs: urls,
		})
	}
	return
}

func extractURLsFromM3U8(u string) (res []*Stream) {
	retry := 0
	request, err := http.NewRequest("GET", u, nil)
	if err != nil {
		log.Println("Could not parse get m3u8 request:", err)
		return
	}

	request.Header.Set("Referer", u)
	request.Header.Set("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:45.0) Gecko/20100101 Firefox/45.0")
	request.Header.Set("Accept", "application/json, text/javascript, */*")
	request.Header.Set("Content-Type", "application/x-www-form-urlencoded")
doRequest:
	resp, err := client.Do(request)
	if err != nil {
		log.Println("Could not send getting m3u8 request:", err)
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
	var urls []string
	for scanner.Scan() {
		line := scanner.Text()
		if strings.HasPrefix(line, "http://") || strings.HasPrefix(line, "https://") {
			urls = append(urls, line)
			continue
		}
		if !strings.HasPrefix(line, "#") {
			l := fmt.Sprintf("%s://%s/%s", req.Scheme, req.Host, line)
			urls = append(urls, l)
		}
	}
	return append(res, &Stream{RealURLs: urls})
}

func postRequest(postBody string, u string) (res []*Stream) {
	req, err := http.NewRequest("POST", u, strings.NewReader(postBody))
	if err != nil {
		log.Println("Could not parse post request:", err)
		return
	}

	req.Header.Set("Referer", u)
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
	for len(content) != 0 && string(content[0]) != "{" {
		content = content[1:]
	}

	type YunAPIResult struct {
		URL     string `json:"url"`
		Ext     string `json:"ext"`
		Message string `json:"msg"`
	}
	var result YunAPIResult
	if err = json.Unmarshal(content, &result); err != nil {
		log.Println("unmarshalling post content failed", err, string(content))
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			//goto doRequest
		}
		return
	}

	if result.Message == "200" || result.Message == "ok" {
		r, _ := url.QueryUnescape(result.URL)
		if !strings.HasPrefix(r, "https://") && !strings.HasPrefix(r, "http://") {
			parsedURL, e := url.Parse(u)
			if e == nil {
				r = fmt.Sprintf("%s://%s/%s", parsedURL.Scheme, parsedURL.Host, r)
			}
		}
		if result.Ext == "xml" {
			return extractURLsFromXML(r)
		}
		if result.Ext == "m3u8" || result.Ext == "m3u8_list" {
			return extractURLsFromM3U8(r)
		}
		return []*Stream{&Stream{RealURLs: []string{r}}}
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
		streams := postRequest(postBody.Encode(), "http://mt2t.com"+path)
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

func getMT2TParseCmdResult(u string, res chan *CmdResult) {
	r := make(chan *CmdResponse)
	go parseByMT2T(u, r)
	result := &CmdResult{Service: "MT2T"}
	result.Result = <-r
	res <- result
}
