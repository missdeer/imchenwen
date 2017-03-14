package main

import (
	"bufio"
	"io/ioutil"
	"log"
	"net/http"
	"os/exec"
	"strings"

	"github.com/gin-gonic/gin"
)

var (
	// generated by command line `head /dev/urandom | tr -dc A-Za-z0-9 | head -c 13 ; echo ''` on Linux bash
	testAPIKey  = "yb2Q1ozScRfJJ"
	adminAPIKey = "z77BV0FABp2PU"
)

type Stream struct {
	Format       string
	Container    string
	VideoProfile string
	DownloadWith string
	RealURLs     []string
}

type CmdResponse struct {
	Site    string
	Title   string
	Artist  string
	Streams []*Stream
}

func parseByYouGet(u string, r chan string) {
	cmd := exec.Command("you-get", "-i", u)
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Println("you-get stdout pipe failed", err)
		r <- ""
		return
	}
	err = cmd.Start()
	if err != nil {
		log.Println("starting you-get failed", err)
		r <- ""
		return
	}
	o, err := ioutil.ReadAll(stdout)
	if err != nil {
		log.Println("reading you-get stdout failed", err)
		r <- ""
		return
	}
	err = cmd.Wait()
	if err != nil {
		log.Println("waiting for you-get exiting failed", err)
		r <- ""
		return
	}
	r <- string(o)
}

func parseByYKDL(u string, r chan string) {
	cmd := exec.Command("python3", "/Users/billy/Shareware/ykdl/bin/ykdl.py", "-i", u)
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Println("ykdl stdout pipe failed", err)
		r <- ""
		return
	}
	err = cmd.Start()
	if err != nil {
		log.Println("starting ykdl failed", err)
		r <- ""
		return
	}
	o, err := ioutil.ReadAll(stdout)
	if err != nil {
		log.Println("reading ykdl stdout failed", err)
		r <- ""
		return
	}
	scanner := bufio.NewScanner(stdout)
	scanner.Split(bufio.ScanLines)
	status := 0
	for scanner.Scan() {
		line := scanner.Text()
		switch status {
		case 0:
			if strings.HasPrefix(line, "site:") {
				status = 1
			}
		case 1:
			if strings.HasPrefix(line, "title:") {
				status = 2
			}
		case 2:
			if strings.HasPrefix(line, "artist:") {
				status = 3
			}
		case 3:
			if strings.HasPrefix(line, "streams:") {
				status = 4
			}
		case 4:
			if strings.Contains(line, "- format:") {

			}
		}
	}
	err = cmd.Wait()
	if err != nil {
		log.Println("waiting for ykdl exiting failed", err)
		r <- ""
		return
	}

	r <- string(o)
}

func handleParseRequest(c *gin.Context) {
	apiKey := c.PostForm("apikey")
	if apiKey == "" {
		c.JSON(http.StatusOK, gin.H{
			"Status": "error",
			"Result": "incorrect api key",
		})
		return
	}

	u := c.PostForm("url")
	if u == "" {
		c.JSON(http.StatusOK, gin.H{
			"Status": "error",
			"Result": "missing url",
		})
		return
	}

	var resultFromYKDL, resultFromYouGet string
	fromYKDL := make(chan string)
	go parseByYKDL(u, fromYKDL)
	fromYouGet := make(chan string)
	go parseByYouGet(u, fromYouGet)
	for i := 0; i < 2; {
		select {
		case resultFromYKDL = <-fromYKDL:
			i++
		case resultFromYouGet = <-fromYouGet:
			i++
		}
	}

	c.JSON(http.StatusOK, gin.H{
		"Result":    "OK",
		"Perferred": resultFromYKDL,
		"Backup":    resultFromYouGet,
	})
}

func main() {
	r := gin.Default()

	// Ping test
	r.GET("/ping", func(c *gin.Context) {
		c.String(200, "pong")
	})

	v1 := r.Group("/v1")
	{
		v1.POST("/parse", handleParseRequest)
	}

	// Listen and Server in 127.0.0.1:8765
	r.Run("127.0.0.1:8765")
}
