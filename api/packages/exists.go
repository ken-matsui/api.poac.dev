package packages

import (
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/misc"
	"net/http"
	"strconv"
)

func getIsExists(r *http.Request, name string, version string) (bool, error) {
	ctx, app, err := misc.NewFirebaseApp(r)
	if err != nil {
		return false, err
	}

	client, err := app.Firestore(ctx)
	if err != nil {
		return false, err
	}
	defer client.Close()

	iter := client.Collection("packages").Where("name", "==", name).Where("version", "==", version).Documents(ctx)
	_, err = iter.Next()
	if err != nil {
		return false, nil
	}
	return true, nil
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
