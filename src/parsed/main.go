package main

import (
	"flag"
	"net/http"
	"strings"

	"github.com/DeanThompson/ginpprof"
	"github.com/gin-gonic/gin"
)

var (
	localMode bool
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

type NativeJSONResult struct {
	Service string
	Result  interface{}
}

type CmdResult struct {
	Service string
	Result  *CmdResponse
}

func nativeJSONRequest(c *gin.Context) bool {
	nativeJSON := c.PostForm("nativejson")
	return nativeJSON == "true"
}

func handleParseRequest(c *gin.Context) {
	u, err := checkInput(c)
	if err != nil {
		return
	}

	parser := c.DefaultPostForm("parser", "all")
	if strings.ToLower(parser) == "all" {
		parser = "ykdl,you-get,youtube-dl"
	}
	parsers := strings.Split(parser, ",")
	if nativeJSONRequest(c) {
		res := make(chan *NativeJSONResult, len(parsers))
		count := 0
		m := map[string]func(string, chan *NativeJSONResult){
			"ykdl":       getYKDLParseNativeJSONResult,
			"you-get":    getYouGetParseNativeJSONResult,
			"youtube-dl": getYoutubeDLParseNativeJSONResult,
		}
		for _, parserName := range parsers {
			if p, ok := m[strings.ToLower(parserName)]; ok {
				go p(u, res)
				count++
			}
		}
		h := gin.H{"Result": "OK"}
		for i := 0; i < count; i++ {
			r := <-res
			h[r.Service] = r.Result
		}
		c.JSON(http.StatusOK, h)
	} else {
		res := make(chan *CmdResult, len(parsers))
		count := 0
		m := map[string]func(string, chan *CmdResult){
			"ykdl":       getYKDLParseCmdResult,
			"you-get":    getYouGetParseCmdResult,
			"youtube-dl": getYoutubeDLParseCmdResult,
			"mt2t":       getMT2TParseCmdResult,
			"aikantv":    getAikanTVParseCmdResult,
			"sfsft":      getSFSFTParseCmdResult,
			"vip":        getVIPJieXiParseCmdResult,
			"vipjiexi":   getVIPJieXiParseCmdResult,
		}
		for _, parserName := range parsers {
			if p, ok := m[strings.ToLower(parserName)]; ok {
				go p(u, res)
				count++
			}
		}
		h := gin.H{"Result": "OK"}
		for i := 0; i < count; i++ {
			r := <-res
			h[r.Service] = r.Result
		}
		c.JSON(http.StatusOK, h)
	}
}

func main() {
	flag.BoolVar(&localMode, "l", false, "local mode")
	flag.Parse()
	gin.SetMode(gin.ReleaseMode)
	r := gin.Default()

	// Ping test
	r.GET("/ping", func(c *gin.Context) {
		c.String(200, "pong")
	})

	v1 := r.Group("/v1")
	{
		v1.POST("/parse", handleParseRequest)
	}

	admin := r.Group("/admin")
	{
		admin.POST("/auth/adduser", handleAddUser)
		admin.POST("/auth/deluser", handleDeleteUser)
	}

	ginpprof.Wrapper(r)

	if !localMode {
		redisInit()
	}
	// Listen and Server in 127.0.0.1:8765
	r.Run("127.0.0.1:8765")
}
