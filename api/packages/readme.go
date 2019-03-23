package packages

import (
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/misc"
	"net/http"
)

func readmeImpl(c echo.Context, name string, version string) error {
	err := validateParam(name, version)
	if err != nil {
		return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
	}

	url, err := misc.ArchiveUrl(c.Request(), name + "-" + version + "/README.md")
	if err != nil {
		return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
	}

	return c.Redirect(http.StatusPermanentRedirect, url)
}

func Readme() echo.HandlerFunc {
	return func(c echo.Context) error {
		name := c.Param("name")
		version := c.Param("version")
		return readmeImpl(c, name, version)
	}
}

func ReadmeOrg() echo.HandlerFunc {
	return func(c echo.Context) error {
		name := c.Param("org") + "-" + c.Param("name")
		version := c.Param("version")
		return readmeImpl(c, name, version)
	}
}
