package api

import (
	"github.com/labstack/echo/v4"
	"google.golang.org/appengine"
	"google.golang.org/appengine/log"
	"google.golang.org/appengine/memcache"
	"net/http"
)

func FlushMemcache() echo.HandlerFunc {
	return func(c echo.Context) error {
		ctx := appengine.NewContext(c.Request())
		err := memcache.Flush(ctx)
		if err != nil {
			log.Debugf(ctx, err.Error())
			return c.NoContent(http.StatusOK)
		} else {
			return c.NoContent(http.StatusOK)
		}
	}
}
