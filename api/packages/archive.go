package packages

import (
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/misc"
	"net/http"
)

func archiveUrl(name string, version string) (string, error) {
	url := "https://storage.googleapis.com/poac-pm.appspot.com/"

	err := misc.CheckPackageName(name)
	if err != nil {
		return "", err
	}
	url += name

	err = misc.CheckPackageVersion(version)
	if err != nil {
		return "", err
	}
	url += "-" + version

	url += ".tar.gz"
	return url, nil
}

func Archive() echo.HandlerFunc {
	return func(c echo.Context) error {
		url, err := archiveUrl(c.Param("name"), c.Param("version"))
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
		}
		return c.Redirect(http.StatusPermanentRedirect, url)
	}
}

func ArchiveDeps() echo.HandlerFunc {
	return func(c echo.Context) error {
		url, err := archiveUrl(c.Param("org") + "-" + c.Param("name"), c.Param("version"))
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
		}
		return c.Redirect(http.StatusPermanentRedirect, url)
	}
}
