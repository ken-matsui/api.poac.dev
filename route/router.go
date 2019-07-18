package route

import (
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/api"
	"github.com/poacpm/api.poac.pm/api/packages"
	_ "github.com/poacpm/api.poac.pm/docs"
	"github.com/poacpm/api.poac.pm/middleware"
	"github.com/swaggo/echo-swagger"
)

func Init() *echo.Echo {
	e := echo.New()
	middleware.Set(e)
	packagesRoute := e.Group("/packages")
	{
		packagesRoute.GET("/deps/:owner/:name/:version", packages.Deps())
		packagesRoute.GET("/exists/:owner/:name/:version", packages.Exists())
		packagesRoute.GET("/versions/:owner/:name", packages.Versions())
		packagesRoute.POST("/register", packages.Register())
	}
	e.GET("/flush-memcache", api.FlushMemcache())
	e.GET("/firestore-export", api.FirestoreExport())
	e.GET("/swagger/*", echoSwagger.WrapHandler)
	return e
}
