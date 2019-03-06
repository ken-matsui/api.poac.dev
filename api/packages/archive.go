package packages

import (
	"github.com/blang/semver"
	"github.com/labstack/echo/v4"
	"net/http"
)

func Archive() echo.HandlerFunc {
	return func(c echo.Context) error {
		url := "https://storage.googleapis.com/poac-pm.appspot.com/"
		url += c.Param("name") // TODO: upload.goで使用している検証をこちらでも行う

		version := c.Param("version")
		_, err := semver.Make(version) // semver error
		if err != nil {
			errStr := "Invalid version.\nPlease adapt to semver.\nSee https://semver.org for details."
			return echo.NewHTTPError(http.StatusInternalServerError, errStr)
		}

		url += "-" + version
		url += ".tar.gz"
		return c.Redirect(http.StatusPermanentRedirect, url)
	}
}

func ArchiveDeps() echo.HandlerFunc {
	return func(c echo.Context) error {
		url := "https://storage.googleapis.com/poac-pm.appspot.com/"
		url += c.Param("org") + "-" + c.Param("name")

		version := c.Param("version")
		_, err := semver.Make(version) // semver error
		if err != nil {
			errStr := "Invalid version.\nPlease adapt to semver.\nSee https://semver.org for details."
			return echo.NewHTTPError(http.StatusInternalServerError, errStr)
		}

		url += "-" + version
		url += ".tar.gz"
		return c.Redirect(http.StatusPermanentRedirect, url)
	}
}
