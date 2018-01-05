// +build windows

package main

import (
	"bufio"
	"flag"
	"io"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"regexp"
	"strings"
	"sync"
	"time"

	"github.com/kardianos/osext"
)

var (
	target string
	wg     sync.WaitGroup
	client = &http.Client{}
)

func downloadExe(u string, saveToFile string) {
	retry := 0
	request, err := http.NewRequest("GET", u, nil)
	if err != nil {
		log.Println("Could not parse getting latest release request:", err)
		return
	}

	request.Header.Set("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:45.0) Gecko/20100101 Firefox/45.0")
	request.Header.Set("Accept", "application/json, text/javascript, */*")
	request.Header.Set("Content-Type", "application/x-www-form-urlencoded")
doRequest:
	resp, err := client.Do(request)
	if err != nil {
		log.Println("Could not send getting latest release request:", err)
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			goto doRequest
		}
		return
	}

	defer resp.Body.Close()

	output, err := os.Create(saveToFile)
	if err != nil {
		log.Println("Error while creating", saveToFile, err)
		return
	}
	defer output.Close()
	log.Println("downloading", u)
	if _, err := io.Copy(output, resp.Body); err != nil {
		log.Println("Error while reading response", u, err)
		return
	}
	log.Println(u, "is saved to", saveToFile)
}

func getExeDownloadURL(u string, pattern string) (exe string) {
	retry := 0
	request, err := http.NewRequest("GET", u, nil)
	if err != nil {
		log.Println("Could not parse getting latest release page request:", err)
		return
	}

	request.Header.Set("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:45.0) Gecko/20100101 Firefox/45.0")
	request.Header.Set("Accept", "application/json, text/javascript, */*")
	request.Header.Set("Content-Type", "application/x-www-form-urlencoded")
doRequest:
	resp, err := client.Do(request)
	if err != nil {
		log.Println("Could not send getting latest release page request:", err)
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			goto doRequest
		}
		return
	}

	defer resp.Body.Close()

	regex := regexp.MustCompile(pattern)
	scanner := bufio.NewScanner(resp.Body)
	for scanner.Scan() {
		line := scanner.Text()
		ss := regex.FindAllString(line, -1)
		if len(ss) > 0 {
			return "https://github.com" + ss[0]
		}
	}
	return
}

func searchExeDownloadURL(u, tagPattern, exePattern string) (exe string) {
	retry := 0
	request, err := http.NewRequest("GET", u, nil)
	if err != nil {
		log.Println("Could not parse getting latest release page request:", err)
		return
	}

	request.Header.Set("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:45.0) Gecko/20100101 Firefox/45.0")
	request.Header.Set("Accept", "application/json, text/javascript, */*")
	request.Header.Set("Content-Type", "application/x-www-form-urlencoded")
doRequest:
	resp, err := client.Do(request)
	if err != nil {
		log.Println("Could not send getting latest release page request:", err)
		retry++
		if retry < 3 {
			time.Sleep(3 * time.Second)
			goto doRequest
		}
		return
	}

	defer resp.Body.Close()

	regex := regexp.MustCompile(tagPattern)
	scanner := bufio.NewScanner(resp.Body)
	for scanner.Scan() {
		line := scanner.Text()
		ss := regex.FindAllString(line, -1)
		if len(ss) > 0 {
			releaseURL := "https://github.com" + ss[0]
			exe = getExeDownloadURL(releaseURL, exePattern)
			if exe != "" {
				return
			}
		}
	}
	return
}

func updateYouGet() {
	latestReleaseURL := `https://github.com/soimort/you-get/tags`
	tagPattern := `\/soimort\/you\-get\/releases\/tag\/v[0-9\.]+`
	exePattern := `\/soimort\/you\-get\/releases\/download\/v[0-9\.]+\/you\-get\-[0-9\.]+\-win32.exe`
	exeURL := searchExeDownloadURL(latestReleaseURL, tagPattern, exePattern)
	if exeURL != "" {
		filePath := `you-get.exe`
		if executable, err := osext.Executable(); err == nil {
			p := filepath.Dir(executable)
			downloadExe(exeURL, filepath.Join(p, filePath))
		}
	}
	wg.Done()
}

func updateYoutubeDL() {
	latestReleaseURL := `https://github.com/rg3/youtube-dl/releases/latest`
	pattern := `\/rg3\/youtube\-dl\/releases\/download\/[0-9\.]+\/youtube\-dl.exe`
	exeURL := getExeDownloadURL(latestReleaseURL, pattern)
	if exeURL != "" {
		filePath := `youtube-dl.exe`
		if executable, err := osext.Executable(); err == nil {
			p := filepath.Dir(executable)
			downloadExe(exeURL, filepath.Join(p, filePath))
		}
	}
	wg.Done()
}

func updateYKDL() {
	latestReleaseURL := `https://github.com/zhangn1985/ykdl/releases/latest`
	pattern := `\/zhangn1985\/ykdl\/releases\/download\/v[0-9\.]+\/ykdl_win32.exe`
	exeURL := getExeDownloadURL(latestReleaseURL, pattern)
	if exeURL != "" {
		filePath := `ykdl.exe`
		if executable, err := osext.Executable(); err == nil {
			p := filepath.Dir(executable)
			downloadExe(exeURL, filepath.Join(p, filePath))
		}
	}
	wg.Done()
}

func main() {
	flag.StringVar(&target, "target", "you-get,ykdl,youtube-dl", "update targets")
	flag.Parse()

	m := map[string]func(){
		"you-get":    updateYouGet,
		"youtube-dl": updateYoutubeDL,
		"ykdl":       updateYKDL,
	}
	targets := strings.Split(target, ",")
	for _, t := range targets {
		f, ok := m[strings.ToLower(t)]
		if ok {
			wg.Add(1)
			go f()
		}
	}
	wg.Wait()
}
