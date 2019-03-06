package misc

import (
	"firebase.google.com/go"
	"golang.org/x/net/context"
	"google.golang.org/appengine"
	"net/http"
)

func NewFirebaseApp(r *http.Request) (context.Context, *firebase.App, error) {
	ctx := appengine.NewContext(r)
	projectID := appengine.AppID(ctx)
	conf := &firebase.Config{ProjectID: projectID}
	app, err := firebase.NewApp(ctx, conf)
	if err != nil {
		return nil, nil, err
	}

	return ctx, app, nil
}
