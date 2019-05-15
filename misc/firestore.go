package misc

import (
	"firebase.google.com/go"
	"golang.org/x/net/context"
	"google.golang.org/appengine"
	"google.golang.org/appengine/log"
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

func CreateDoc(r *http.Request, config map[string]interface{}) error {
	ctx, app, err := NewFirebaseApp(r)
	if err != nil {
		return err
	}

	client, err := app.Firestore(ctx)
	if err != nil {
		return err
	}
	defer client.Close()

	_, _, err = client.Collection("packages").Add(ctx, config)
	if err != nil {
		log.Debugf(ctx, "Failed adding collection: %v", err)
		return err
	}
	return nil
}
