package packages

import (
	"github.com/labstack/echo"
	"github.com/poacpm/api.poac.pm/misc"
	"google.golang.org/appengine/log"
	"net/http"
)

func getVersions(r *http.Request, name string) ([]string, error) {
	ctx, client, err := misc.NewFirestoreClient(r)
	if err != nil {
		return nil, err
	}

	versions := []string{}
	iter := client.Collection("packages").Where("name", "==", name).Documents(ctx)
	for {
		doc, err := iter.Next()
		if err != nil {
			log.Debugf(ctx, err.Error())
			break
		}
		log.Debugf(ctx, "version %v", doc.Data()["version"].(string))
		versions = append(versions, doc.Data()["version"].(string))
	}
	return versions, nil
}

func Versions() echo.HandlerFunc {
	return func(c echo.Context) error {
		name := c.Param("name")

		versions, err := getVersions(c.Request(), name)
		if err != nil {
			return err
		}
		if len(versions) == 0 {
			return c.String(http.StatusOK, "null")
		}
		return c.JSON(http.StatusOK, versions)
	}
}

func VersionsOrg() echo.HandlerFunc {
	return func(c echo.Context) error {
		org := c.Param("org")
		name := c.Param("name")

		versions, err := getVersions(c.Request(), org + "/" + name)
		if err != nil {
			return err
		}
		if len(versions) == 0 {
			return c.String(http.StatusOK, "null")
		}
		return c.JSON(http.StatusOK, versions)
	}
}
