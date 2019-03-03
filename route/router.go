package route

import (
	"github.com/labstack/echo"
	"github.com/poacpm/api.poac.pm/api/packages"
	"github.com/poacpm/api.poac.pm/middleware"
)

func Init() *echo.Echo {
	e := echo.New()
	middleware.Set(e)

	// Routes
	route := e.Group("")
	{
		route.POST("/packages/upload", upload.Do())
		//route.GET("/members", api.GetMembers())
		//route.GET("/members/:id", api.GetMember())
	}
	return e
}
