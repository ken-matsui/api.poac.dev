package middleware

import (
	"github.com/labstack/echo"
	"github.com/labstack/echo/middleware"
)

func Set(e *echo.Echo) {
	// Set Bundle MiddleWare
	e.Use(middleware.Logger())
	e.Use(middleware.Gzip())
}
