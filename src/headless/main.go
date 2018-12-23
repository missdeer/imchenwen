package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
	"strings"

	"github.com/DeanThompson/ginpprof"
	"github.com/gin-gonic/gin"
)

var (
	staticPath string
	cache      *RedisCache
)

func handleDynamicReuqest(c *gin.Context) {
	id := c.Param("id")
	toURL, err := cache.Get(id)
	if err != nil {
		c.JSON(http.StatusOK, gin.H{
			"result": "error",
			"reason": fmt.Sprintln(err),
		})
		return
	}
	// forward toURL
}

func handleMapURL(c *gin.Context) {
	from := c.PostForm("from")
	to := c.PostForm("to")

	if from == "" || to == "" {
		c.JSON(http.StatusOK, gin.H{
			"result": "error",
			"reason": "need from or to",
		})
		return
	}
	// save to redis
	cache.Put(from, to)
	c.JSON(http.StatusOK, gin.H{
		"result": "ok",
		"from":   from,
		"to":     to,
	})
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
	log.Println(url)
}

func main() {
	staticPath = os.Getenv("STATIC_PATH")
	if staticPath == "" {
		log.Fatal("need STATIC_PATH environment variable set")
	}
	os.MkdirAll(staticPath, 0755)

	redisAddr := os.Getenv("REDIS_ADDR")
	if redisAddr != "" {
		defaultRedisServer = redisAddr
	}
	cache = redisInit()

	gin.SetMode(gin.ReleaseMode)
	r := gin.Default()

	v1 := r.Group("/v1")
	{
		v1.POST("/map", handleMapURL)
		v1.GET("/resolve", handleResolveURL)
		v1.GET("/d/:id", handleDynamicReuqest)
		v1.Static("/s", staticPath)
	}

	ginpprof.Wrapper(r)

	bindAddr := os.Getenv("BIND_ADDR")
	if bindAddr == "" {
		// Listen and Server in 127.0.0.1:8765 by default
		bindAddr = "127.0.0.1:8765"
	}
	r.Run(bindAddr)
}
