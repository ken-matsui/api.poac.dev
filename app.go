package main

import (
	"github.com/newrelic/go-agent"
	"github.com/poacpm/api.poac.pm/route"
	"google.golang.org/appengine"
	"net/http"
	"os"
)

func main() {
	e := route.Init()
	config := newrelic.NewConfig("api.poac.pm", os.Getenv("NEWRELIC_LICENSE"))
	app, _ := newrelic.NewApplication(config)
	http.Handle(newrelic.WrapHandle(app, "/", e))
	txn := app.StartTransaction("myTxn", nil, nil)
	defer txn.End()
	appengine.Main()
}
