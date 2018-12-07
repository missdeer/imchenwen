package main

import (
	"net/http"
	"os"
	"strings"

	"github.com/DeanThompson/ginpprof"
	"github.com/gin-gonic/gin"
)

func handleMapURL(c *gin.Context) {
	from := c.PostForm("from")
	to := c.PostForm("to")
}

func handleResolveURL(c *gin.Context) {
	r := c.DefaultQuery("r", "ykdl,you-get,youtube-dl,annie")
	rs := strings.Split(r, ",")
	if len(rs) == 0 {
		c.JSON(http.StatusOK, gin.H{
			"msg": "resolver missing",
		})
		return
	}
	url := c.Query("u")
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
		v1.POST("/map", handleMapURL)
		v1.GET("/resolve", handleResolveURL)
	}

	ginpprof.Wrapper(r)

	redisInit()

	bindAddr := os.Getenv("BIND_ADDR")
	if bindAddr == "" {
		// Listen and Server in 127.0.0.1:8765 by default
		bindAddr = "127.0.0.1:8765"
	}
	r.Run(bindAddr)
}
