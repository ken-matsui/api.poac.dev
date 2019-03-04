package misc

import (
	"cloud.google.com/go/firestore"
	"firebase.google.com/go"
	"golang.org/x/net/context"
	"google.golang.org/appengine"
	"net/http"
)

func NewFirestoreClient(r *http.Request) (context.Context, *firestore.Client, error) {
	ctx := appengine.NewContext(r)
	projectID := "poac-pm"
	conf := &firebase.Config{ProjectID: projectID}
	app, err := firebase.NewApp(ctx, conf)
	if err != nil {
		return nil, nil, err
	}

	client, err := app.Firestore(ctx)
	if err != nil {
		return nil, nil, err
	}
	defer client.Close()

	return ctx, client, nil
}
