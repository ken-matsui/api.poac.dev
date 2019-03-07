package packages

import (
	"firebase.google.com/go"
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/misc"
	"golang.org/x/net/context"
	"net/http"
	"strconv"
)

func getExists(ctx context.Context, app *firebase.App, name string, version string) (bool, error) {
	client, err := app.Firestore(ctx)
	if err != nil {
		return false, err
	}
	defer client.Close()

	collection := client.Collection("packages")
	query := collection.Where("name", "==", name).Where("version", "==", version)
	iter := query.Documents(ctx)
	_, err = iter.Next()
	if err != nil {
		return false, nil
	}
	return true, nil
}

func handleExists(c echo.Context, name string, version string) (bool, error) {
	// Create new firebase app
	ctx, app, err := misc.NewFirebaseApp(c.Request())
	if err != nil {
		return false, err
	}

	isExists, err := getExists(ctx, app, name, version)
	if err != nil {
		return false, err
	}
	return isExists, nil
}

func Exists() echo.HandlerFunc {
	return func(c echo.Context) error {
		name := c.Param("name")
		version := c.Param("version")
		isExists, err := handleExists(c, name, version)
		if err != nil {
			return err
		}
		return c.String(http.StatusOK, strconv.FormatBool(isExists))
	}
}

func ExistsOrg() echo.HandlerFunc {
	return func(c echo.Context) error {
		name := c.Param("org") + "/" + c.Param("name")
		version := c.Param("version")
		isExists, err := handleExists(c, name, version)
		if err != nil {
			return err
		}
		return c.String(http.StatusOK, strconv.FormatBool(isExists))
	}
}
