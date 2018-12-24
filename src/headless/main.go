package main

import (
	"io"
	"log"
	"net/http"
	"os"
	"path/filepath"
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
	// 1. 已经下载完成的，本地有对应的完整文件的
	// 2. 正在下载的，下载到一半的，本地有对应的不完整文件的
	if p, err := cache.Get(id + ":fs"); err == nil {
		if b, err := fileExists(p.(string)); err == nil && b {
			c.File(p.(string))
			return
		}
	}
	// 3. 没有下载过的，只有一个目标URL（以及user-agent，cookie，referer等信息）的
	to, err := cache.Get(id + ":to")
	if err != nil {
		c.JSON(http.StatusOK, gin.H{
			"result": "error",
			"reason": "not found matched target URL",
		})
		return
	}
	userAgent, err := cache.Get(id + ":ua")
	cookie, err := cache.Get(id + ":cookie")
	referer, err := cache.Get(id + ":referer")

	client := &http.Client{}
	toURL := to.(string)
	request, err := http.NewRequest("GET", toURL, nil)
	if err != nil {
		log.Println("Could not parse getting video request:", err)
		return
	}
	if userAgent == nil {
		request.Header.Set("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:45.0) Gecko/20100101 Firefox/45.0")
	} else {
		request.Header.Set("User-Agent", userAgent.(string))
	}
	if cookie != nil {
		request.Header.Set("Cookie", cookie.(string))
	}
	if referer != nil {
		request.Header.Set("Referer", referer.(string))
	}
	request.Header.Set("Accept", "application/json, text/javascript, */*")

	resp, err := client.Do(request)
	if err != nil {
		log.Println("Could not send getting video request:", err)
		return
	}

	defer resp.Body.Close()
	if strings.HasSuffix(toURL, ".m3u8") {
		// 3.a. m3u8类型的

		// 3.a.1）加密的；
		// 3.a.2）不加密的。

		// 3.a.1）完整URL的；
		// 3.a.2）根路径的；
		// 3.a.3）相对路径的。
	} else {
		// 3.b. mp4/flv/ts 等类型的
		staticFilePath := filepath.Join(staticPath, id)
		lp := &LocalPipe{}
		lp.File(staticFilePath)
		go func() {
			io.Copy(lp, resp.Body)
			lp.End()
			cache.Put(id+":fs", staticFilePath)
		}()

		c.DataFromReader(http.StatusOK, resp.ContentLength, "application/octet-stream", lp, nil)
	}
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
	cache.Put(from+":to", to)
	res := gin.H{
		"result": "ok",
		"from":   from,
		"to":     to,
	}
	if userAgent := c.PostForm("userAgent"); userAgent != "" {
		cache.Put(from+":ua", userAgent)
		res["userAgent"] = userAgent
	}
	if cookie := c.PostForm("cookie"); cookie != "" {
		cache.Put(from+":cookie", cookie)
		res["cookie"] = cookie
	}
	if referer := c.PostForm("referer"); referer != "" {
		cache.Put(from+":referer", referer)
		res["referer"] = referer
	}
	c.JSON(http.StatusOK, res)
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
