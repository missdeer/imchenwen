package main

import (
	"net/http"

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
	DownloadWith string
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

func handlePreferredParseRequest(c *gin.Context) {
	u, err := checkInput(c)
	if err != nil {
		return
	}

	if nativeJSONRequest(c) {
		fromYKDL := make(chan interface{})
		go parseByYKDLJSON(u, fromYKDL)
		resultFromYKDL := <-fromYKDL

		c.JSON(http.StatusOK, gin.H{
			"Result":    "OK",
			"Preferred": resultFromYKDL,
		})
	} else {
		fromYKDL := make(chan *CmdResponse)
		go parseByYKDL(u, fromYKDL)
		resultFromYKDL := <-fromYKDL

		c.JSON(http.StatusOK, gin.H{
			"Result":    "OK",
			"Preferred": resultFromYKDL,
		})
	}
}

func handleBackupParseRequest(c *gin.Context) {
	u, err := checkInput(c)
	if err != nil {
		return
	}

	if nativeJSONRequest(c) {
		fromYouGet := make(chan interface{})
		go parseByYouGetJSON(u, fromYouGet)
		resultFromYouGet := <-fromYouGet

		c.JSON(http.StatusOK, gin.H{
			"Result": "OK",
			"Backup": resultFromYouGet,
		})
	} else {
		fromYouGet := make(chan *CmdResponse)
		go parseByYouGet(u, fromYouGet)
		resultFromYouGet := <-fromYouGet

		c.JSON(http.StatusOK, gin.H{
			"Result": "OK",
			"Backup": resultFromYouGet,
		})
	}
}

func handleParseRequest(c *gin.Context) {
	u, err := checkInput(c)
	if err != nil {
		return
	}

	if nativeJSONRequest(c) {
		var resultFromYKDL, resultFromYouGet interface{}
		fromYKDL := make(chan interface{})
		go parseByYKDLJSON(u, fromYKDL)
		fromYouGet := make(chan interface{})
		go parseByYouGetJSON(u, fromYouGet)
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
			"Preferred": resultFromYKDL,
			"Backup":    resultFromYouGet,
		})
	} else {
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
			"Preferred": resultFromYKDL,
			"Backup":    resultFromYouGet,
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
		v1.POST("/parse/all", handleParseRequest)
		v1.POST("/parse/preferred", handlePreferredParseRequest)
		v1.POST("/parse/backup", handleBackupParseRequest)
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
