package route

import (
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/api"
	"github.com/poacpm/api.poac.pm/api/packages"
	"github.com/poacpm/api.poac.pm/api/tokens"
	_ "github.com/poacpm/api.poac.pm/docs"
	"github.com/poacpm/api.poac.pm/middleware"
	"github.com/swaggo/echo-swagger"
)

func Init() *echo.Echo {
	e := echo.New()
	middleware.Set(e)

	// Routes
	packagesRoute := e.Group("/packages")
	{
		archiveRoute := packagesRoute.Group("/archive")
		{
			archiveRoute.GET("/:name/:version", packages.Archive())
			archiveRoute.GET("/:org/:name/:version", packages.ArchiveOrg())
		}
		depsRoute := packagesRoute.Group("/deps")
		{
			depsRoute.GET("/:name/:version", packages.Deps())
			depsRoute.GET("/:org/:name/:version", packages.DepsOrg())
		}
		existsRoute := packagesRoute.Group("/exists")
		{
			existsRoute.GET("/:name/:version", packages.Exists())
			existsRoute.GET("/:org/:name/:version", packages.ExistsOrg())
		}
		readmeRoute := packagesRoute.Group("/readme")
		{
			readmeRoute.GET("/:name/:version", packages.Readme())
			readmeRoute.GET("/:org/:name/:version", packages.ReadmeOrg())
		}
		versionsRoute := packagesRoute.Group("/versions")
		{
			versionsRoute.GET("/:name", packages.Versions())
			versionsRoute.GET("/:org/:name", packages.VersionsOrg())
		}
		packagesRoute.POST("/upload", packages.Upload())
	}
	e.POST("/tokens/validate", tokens.Validate())
	e.GET("/flush-memcache", api.FlushMemcache())
	e.GET("/swagger/*", echoSwagger.WrapHandler)
	return e
}
