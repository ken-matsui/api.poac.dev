package main

import (
	"github.com/poacpm/api.poac.pm/route"
	"google.golang.org/appengine"
	"net/http"
)

func main() {
	e := route.Init()
	http.Handle("/", e)
	appengine.Main()
}
