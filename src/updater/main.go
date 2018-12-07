package main

import (
	"flag"
	"strings"
)

func main() {
	flag.StringVar(&target, "target", "you-get,ykdl,youtube-dl,annie", "update targets")
	flag.Parse()

	m := map[string]func(){
		"you-get":    updateYouGet,
		"youtube-dl": updateYoutubeDL,
		"ykdl":       updateYKDL,
		"annie":      updateAnnie,
	}
	targets := strings.Split(target, ",")
	for _, t := range targets {
		f, ok := m[strings.ToLower(t)]
		if ok {
			wg.Add(1)
			go f()
		}
	}
	wg.Wait()
}
