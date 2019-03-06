package packages

import (
	"errors"
	"github.com/blang/semver"
	"github.com/labstack/echo/v4"
	"net/http"
)

func archiveUrl(name string, version string) (string, error) {
	url := "https://storage.googleapis.com/poac-pm.appspot.com/"
	url += name // TODO: upload.goで使用している検証をこちらでも行う

	_, err := semver.Make(version) // semver error
	if err != nil {
		errStr := "Invalid version.\nPlease adapt to semver.\nSee https://semver.org for details."
		return "", errors.New(errStr)
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
