package main

import (
	"net/http"
	"strings"

	"github.com/DeanThompson/ginpprof"
	"github.com/gin-gonic/gin"
)

type Stream struct {
	Format       string `json:",omitempty"`
	ITag         string `json:",omitempty"`
	Container    string
	Quality      string `json:",omitempty"`
	VideoProfile string `json:",omitempty"`
	Size         string `json:",omitempty"`
	DownloadWith string `json:",omitempty"`
	RealURLs     []string
}

type CmdResponse struct {
	Site    string
	Title   string
	Artist  string `json:",omitempty"`
	Streams []*Stream
}

func nativeJSONRequest(c *gin.Context) bool {
	nativeJSON := c.PostForm("nativejson")
	return nativeJSON == "true"
}

func handleParseRequest(c *gin.Context) {
	parser := c.DefaultPostForm("parser", "all")
	switch strings.ToLower(parser) {
	case "ykdl":
		handleYKDLParseRequest(c)
	case "you-get":
		handleYouGetParseRequest(c)
	case "youtube-dl":
		handleYoutubeDLParseRequest(c)
	case "all":
		handleParseAllRequest(c)
	}
}

func handleParseAllRequest(c *gin.Context) {
	u, err := checkInput(c)
	if err != nil {
		return
	}

	if nativeJSONRequest(c) {
		var resultFromYKDL, resultFromYouGet, resultFromYoutubeDL interface{}
		fromYKDL := make(chan interface{})
		go parseByYKDLJSON(u, fromYKDL)
		fromYouGet := make(chan interface{})
		go parseByYouGetJSON(u, fromYouGet)
		fromYoutubeDL := make(chan interface{})
		go parseByYoutubeDLJSON(u, fromYoutubeDL)
		for i := 0; i < 3; {
			select {
			case resultFromYKDL = <-fromYKDL:
				i++
			case resultFromYouGet = <-fromYouGet:
				i++
			case resultFromYoutubeDL = <-fromYoutubeDL:
				i++
			}
		}

		c.JSON(http.StatusOK, gin.H{
			"Result":    "OK",
			"YKDL":      resultFromYKDL,
			"YouGet":    resultFromYouGet,
			"YoutubeDL": resultFromYoutubeDL,
		})
	} else {
		var resultFromYKDL, resultFromYouGet, resultFromYoutubeDL *CmdResponse
		fromYKDL := make(chan *CmdResponse)
		go parseByYKDL(u, fromYKDL)
		fromYouGet := make(chan *CmdResponse)
		go parseByYouGet(u, fromYouGet)
		fromYoutubeDL := make(chan *CmdResponse)
		go parseByYoutubeDL(u, fromYoutubeDL)
		for i := 0; i < 3; {
			select {
			case resultFromYKDL = <-fromYKDL:
				i++
			case resultFromYouGet = <-fromYouGet:
				i++
			case resultFromYoutubeDL = <-fromYoutubeDL:
				i++
			}
		}

		c.JSON(http.StatusOK, gin.H{
			"Result":    "OK",
			"YKDL":      resultFromYKDL,
			"YouGet":    resultFromYouGet,
			"YoutubeDL": resultFromYoutubeDL,
		})
	}
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
		v1.POST("/parse/all", handleParseAllRequest)
		v1.POST("/parse/ykdl", handleYKDLParseRequest)
		v1.POST("/parse/youget", handleYouGetParseRequest)
		v1.POST("/parse/youtubedl", handleYoutubeDLParseRequest)
	}

	admin := r.Group("/admin")
	{
		admin.POST("/auth/adduser", handleAddUser)
		admin.POST("/auth/deluser", handleDeleteUser)
	}

	ginpprof.Wrapper(r)

	redisInit()
	// Listen and Server in 127.0.0.1:8765
	r.Run("127.0.0.1:8765")
}
