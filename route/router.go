package route

import (
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/api"
	"github.com/poacpm/api.poac.pm/api/packages"
	"github.com/poacpm/api.poac.pm/api/tokens"
	_ "github.com/poacpm/api.poac.pm/docs"
	"github.com/poacpm/api.poac.pm/middleware"
	"github.com/swaggo/echo-swagger/v2"
)

func Init() *echo.Echo {
	e := echo.New()
	middleware.Set(e)

	// Routes
	packagesRoute := e.Group("/packages")
	{
		depsRoute := packagesRoute.Group("/deps")
		{
			depsRoute.GET("/:name/:version", packages.Deps())
			depsRoute.GET("/:org/:name/:version", packages.DepsOrg())
		}
		versionsRoute := packagesRoute.Group("/versions")
		{
			versionsRoute.GET("/:name", packages.Versions())
			versionsRoute.GET("/:org/:name", packages.VersionsOrg())
		}
		existsRoute := packagesRoute.Group("/exists")
		{
			existsRoute.GET("/:name/:version", packages.Exists())
			existsRoute.GET("/:org/:name/:version", packages.ExistsOrg())
		}
		packagesRoute.POST("/upload", packages.Upload())
	}
	e.POST("/tokens/validate", tokens.Validate())
	e.GET("/flush-memcache", api.FlushMemcache())
	e.GET("/swagger/*", echoSwagger.WrapHandler)
	return e
}
