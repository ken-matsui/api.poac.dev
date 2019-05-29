package api

import (
	"bytes"
	"encoding/json"
	"fmt"
	"github.com/labstack/echo/v4"
	"google.golang.org/appengine"
	"google.golang.org/appengine/log"
	"google.golang.org/appengine/urlfetch"
	"io"
	"net/http"
	"time"
)

// https://firebase.google.com/docs/firestore/solutions/schedule-export
// IAM -> ${projectId}@appspot.gserviceaccount.com -> Cloud Datastore Import Export Admin

func StreamToString(stream io.Reader) string {
	buf := new(bytes.Buffer)
	buf.ReadFrom(stream)
	return buf.String()
}

func FirestoreExport() echo.HandlerFunc {
	return func(c echo.Context) error {
		ctx := appengine.NewContext(c.Request())

		values := struct{
			CollectionIds []string `json:"collectionIds"`
			OutputURIPrefix string `json:"outputUriPrefix"`
		}{}

		values.CollectionIds = []string{"packages", "tokens", "users"}

		bucketName := "poac-pm-backups-firestore"
		jst, err := time.LoadLocation("Asia/Tokyo")
		if err != nil {
			return err
		}
		values.OutputURIPrefix = fmt.Sprintf("gs://%s/%s", bucketName, time.Now().In(jst).Format(time.RFC3339))

		jsonStr, err := json.Marshal(values)
		if err != nil {
			return err
		}

		accessToken, _, err := appengine.AccessToken(ctx, "https://www.googleapis.com/auth/datastore")
		if err != nil {
			return err
		}

		projectId := appengine.AppID(ctx)
		req, err := http.NewRequest(
			"POST",
			fmt.Sprintf("https://firestore.googleapis.com/v1beta2/projects/%s/databases/(default):exportDocuments", projectId),
			bytes.NewBuffer(jsonStr),
		)
		if err != nil {
			return err
		}
		req.Header.Set("Content-Type", "application/json")
		req.Header.Set("Authorization", "Bearer " + accessToken)

		client := urlfetch.Client(ctx)
		resp, err := client.Do(req)
		if err != nil {
			return err
		}
		if resp.StatusCode != 200 {
			log.Errorf(ctx, fmt.Sprintf("%s: %s", resp.Status, StreamToString(resp.Body)))
			return c.NoContent(resp.StatusCode)
		}

		return c.NoContent(http.StatusOK)
	}
}
