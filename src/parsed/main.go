package main

import (
	"net/http"

	"github.com/DeanThompson/ginpprof"
	"github.com/gin-gonic/gin"
)

type Stream struct {
	Format       string
	Container    string
	VideoProfile string
	DownloadWith string
	Size         string `json:",omitempty"`
	RealURLs     []string
}

type CmdResponse struct {
	Site    string
	Title   string
	Artist  string `json:",omitempty"`
	Streams []*Stream
}

func handlePreferredParseRequest(c *gin.Context) {
	u, err := checkInput(c)
	if err != nil {
		return
	}

	fromYKDL := make(chan *CmdResponse)
	go parseByYKDL(u, fromYKDL)
	resultFromYKDL := <-fromYKDL

	c.JSON(http.StatusOK, gin.H{
		"Result":    "OK",
		"Perferred": resultFromYKDL,
	})
}

func handleBackupParseRequest(c *gin.Context) {
	u, err := checkInput(c)
	if err != nil {
		return
	}

	fromYouGet := make(chan *CmdResponse)
	go parseByYouGet(u, fromYouGet)
	resultFromYouGet := <-fromYouGet

	c.JSON(http.StatusOK, gin.H{
		"Result": "OK",
		"Backup": resultFromYouGet,
	})
}

func handleParseRequest(c *gin.Context) {
	u, err := checkInput(c)
	if err != nil {
		return
	}

	var resultFromYKDL, resultFromYouGet *CmdResponse
	fromYKDL := make(chan *CmdResponse)
	go parseByYKDL(u, fromYKDL)
	fromYouGet := make(chan *CmdResponse)
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
	gin.SetMode(gin.ReleaseMode)
	r := gin.Default()

	// Ping test
	r.GET("/ping", func(c *gin.Context) {
		c.String(200, "pong")
	})

	v1 := r.Group("/v1")
	{
		v1.POST("/parse", handleParseRequest)
		v1.POST("/parse/all", handleParseRequest)
		v1.POST("/parse/preferred", handlePreferredParseRequest)
		v1.POST("/parse/backup", handleBackupParseRequest)
	}

	ginpprof.Wrapper(r)

	redisInit()
	// Listen and Server in 127.0.0.1:8765
	r.Run("127.0.0.1:8765")
}
