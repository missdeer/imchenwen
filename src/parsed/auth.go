package main

import (
	"errors"
	"net/http"

	"github.com/gin-gonic/gin"
)

var (
	// generated by command line `head /dev/urandom | tr -dc A-Za-z0-9 | head -c 13 ; echo ''` on Linux bash
	testAPIKey  = "yb2Q1ozScRfJJ"
	adminAPIKey = "z77BV0FABp2PU"
)

func checkInput(c *gin.Context) (u string, err error) {
	apiKey := c.PostForm("apikey")
	if apiKey == "" {
		c.JSON(http.StatusOK, gin.H{
			"Status": "error",
			"Result": "incorrect api key",
		})
		err = errors.New("incorrect api key")
		return
	}

	u = c.PostForm("url")
	if u == "" {
		c.JSON(http.StatusOK, gin.H{
			"Status": "error",
			"Result": "missing url",
		})
		err = errors.New("missing url")
		return
	}
	return
}
