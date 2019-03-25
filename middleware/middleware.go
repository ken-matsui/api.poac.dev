package middleware

import (
	"github.com/didip/tollbooth"
	"github.com/didip/tollbooth/limiter"
	"github.com/labstack/echo/v4"
	"github.com/labstack/echo/v4/middleware"
	"time"
)

func CacheControlHeader(next echo.HandlerFunc) echo.HandlerFunc {
	return func(c echo.Context) error {
		c.Response().Header().Set("Cache-Control", "public, max-age=60")
		return next(c)
	}
}

func LimitMiddleware(lmt *limiter.Limiter) echo.MiddlewareFunc {
	return func(next echo.HandlerFunc) echo.HandlerFunc {
		return echo.HandlerFunc(func(c echo.Context) error {
			httpError := tollbooth.LimitByRequest(lmt, c.Response(), c.Request())
			if httpError != nil {
				return c.String(httpError.StatusCode, httpError.Message)
			}
			return next(c)
		})
	}
}

func LimitHandler() echo.MiddlewareFunc {
	// create a 1 request/second limiter and
	// every token bucket in it will expire 1 hour after it was initially set.
	expirableOpt := limiter.ExpirableOptions{DefaultExpirationTTL: time.Hour}
	lmt := tollbooth.NewLimiter(1, &expirableOpt)
	lmt.SetMessage("You have reached maximum request limit.")
	return LimitMiddleware(lmt)
}

func Set(e *echo.Echo) {
	// Set Bundle MiddleWare
	e.Use(middleware.Logger())
	e.Use(middleware.Gzip())
	e.Use(CacheControlHeader)
	e.Use(LimitHandler())
	e.Use(middleware.CORSWithConfig(middleware.CORSConfig{
		AllowOrigins: []string{"http://localhost:5000", "https://poac.pm", "https://poac.io"},
	}))
}
