package api

import (
	"bytes"
	"encoding/json"
	"github.com/labstack/echo/v4"
	"google.golang.org/appengine"
	"google.golang.org/appengine/urlfetch"
	"math/rand"
	"net/http"
	"os"
	"time"
)

type PostParam struct {
	Timestamp int `json:"timestamp"`
	Value int `json:"value"`
}

func Statuspage() echo.HandlerFunc {
	return func(c echo.Context) error {
		api_key := os.Getenv("STATUSPAGE_API_KEY")
		page_id := "mq03q0kkvqq4"
		metric_id := "ys6rfldn7z00"
		api_base := "api.statuspage.io"

		// need 1 data point every 5 minutes
		// submit random data for the whole day
		totalPoints := (60 / 5 * 24)
		for i := 0; i < totalPoints; i++ {
			t := time.Now()
			rand.Seed(t.UnixNano())
			param := PostParam{
				Timestamp: int(t.Unix()) - (i * 5 * 60),
				Value:     rand.Intn(100),
			}
			paramBytes, err := json.Marshal(param)
			if err != nil {
				return c.NoContent(http.StatusInternalServerError)
			}

			ctx := appengine.NewContext(c.Request())
			client := urlfetch.Client(ctx)
			url := "https://" + api_base + "/v1/pages/" + page_id + "/metrics/" + metric_id + "/data.json"
			req, err := http.NewRequest("POST", url, bytes.NewReader(paramBytes))
			if err != nil {
				return c.NoContent(http.StatusInternalServerError)
			}

			req.Header.Set("Content-Type", "application/x-www-form-urlencoded")
			req.Header.Set("Authorization", "OAuth " + api_key)
			_, err = client.Do(req)
			if err != nil {
				return c.NoContent(http.StatusInternalServerError)
			}

			time.Sleep(1 * time.Second) // sleep 1 second
		}

		return c.NoContent(http.StatusOK)
	}
}
