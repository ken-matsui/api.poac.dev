package packages

import (
	"github.com/labstack/echo"
	"github.com/poacpm/api.poac.pm/misc"
	"net/http"
	"strconv"
)

func getIsExists(r *http.Request, name string, version string) (bool, error) {
	ctx, client, err := misc.NewFirestoreClient(r)
	if err != nil {
		return false, err
	}

	iter := client.Collection("packages").Where("name", "==", name).Where("version", "==", version).Documents(ctx)
	for {
		_, err := iter.Next()
		if err != nil {
			return false, nil
		}
		return true, nil
	}
}

func Exists() echo.HandlerFunc {
	return func(c echo.Context) error {
		name := c.Param("name")
		version := c.Param("version")

		isExists, err := getIsExists(c.Request(), name, version)
		if err != nil {
			return err
		}
		return c.String(http.StatusOK, strconv.FormatBool(isExists))
	}
}

func ExistsOrg() echo.HandlerFunc {
	return func(c echo.Context) error {
		org := c.Param("org")
		name := c.Param("name")
		version := c.Param("version")

		isExists, err := getIsExists(c.Request(), org + "/" + name, version)
		if err != nil {
			return err
		}
		return c.String(http.StatusOK, strconv.FormatBool(isExists))
	}
}
