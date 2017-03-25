package main

import (
	"log"
	"testing"
)

func TestFindExecutable(t *testing.T) {
	log.Println(findExecutable("ykdl"))
	log.Println(findExecutable("you-get"))
	log.Println(findExecutable("youtube-dl"))
}
