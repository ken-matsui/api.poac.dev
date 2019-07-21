package packages

import (
	"firebase.google.com/go"
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/misc"
	"golang.org/x/net/context"
	"net/http"
	"strconv"
)

func getExists(ctx context.Context, app *firebase.App, owner, repo, version string) (bool, error) {
	client, err := app.Firestore(ctx)
	if err != nil {
		return false, err
	}
	defer client.Close()

	collection := client.Collection("packages")
	query := collection.Where("owner", "==", owner).
		                Where("repo", "==", repo).
		                Where("version", "==", version)
	iter := query.Documents(ctx)
	_, err = iter.Next()
	if err != nil {
		return false, nil
	}
	return true, nil
}

func handleExists(c echo.Context, owner, repo, version string) (bool, error) {
	// Create new firebase app
	ctx, app, err := misc.NewFirebaseApp(c.Request())
	if err != nil {
		return false, err
	}

	isExists, err := getExists(ctx, app, owner, repo, version)
	if err != nil {
		return false, err
	}
	return isExists, nil
}

func Exists() echo.HandlerFunc {
	return func(c echo.Context) error {
		isExists, err := handleExists(c, c.Param("owner"), c.Param("repo"), c.Param("version"))
		if err != nil {
			return err
		}
		return c.String(http.StatusOK, strconv.FormatBool(isExists))
	}
}
