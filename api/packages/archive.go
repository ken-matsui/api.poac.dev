package packages

import (
	"cloud.google.com/go/storage"
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/misc"
	"google.golang.org/appengine"
	"google.golang.org/appengine/file"
	"net/http"
	"time"
)

func archiveUrl(r *http.Request, name string, version string) (string, error) {
	ctx := appengine.NewContext(r)

	bucketName, err := file.DefaultBucketName(ctx)
	if err != nil {
		return "", err
	}
	googleAccessStorageId, err := appengine.ServiceAccount(ctx)
	if err != nil {
		return "", err
	}

	// The expiration time is 10 minute.
	expires := time.Now().Add(10 * time.Minute)
	signedUrlOptions := storage.SignedURLOptions{
		GoogleAccessID: googleAccessStorageId,
		SignBytes: func(b []byte) ([]byte, error) {
			_, signedBytes, err := appengine.SignBytes(ctx, b)
			return signedBytes, err
		},
		Method:  "GET",
		Expires: expires,
	}

	objectName := name + "-" + version + ".tar.gz"
	signedUrl, err := storage.SignedURL(bucketName, objectName, &signedUrlOptions)
	return signedUrl, nil
}

func validateParam(name string, version string) error {
	err := misc.CheckPackageName(name)
	if err != nil {
		return err
	}
	err = misc.CheckPackageVersion(version)
	if err != nil {
		return err
	}
	return nil
}

func Archive() echo.HandlerFunc {
	return func(c echo.Context) error {
		name := c.Param("name")
		version := c.Param("version")

		err := validateParam(name, version)
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
		}

		url, err := archiveUrl(c.Request(), name, version)
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
		}

		return c.Redirect(http.StatusPermanentRedirect, url)
	}
}

func ArchiveDeps() echo.HandlerFunc {
	return func(c echo.Context) error {
		name := c.Param("org") + "-" + c.Param("name")
		version := c.Param("version")

		err := validateParam(name, version)
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
		}

		url, err := archiveUrl(c.Request(), name, version)
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
		}

		return c.Redirect(http.StatusPermanentRedirect, url)
	}
}
