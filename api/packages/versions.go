package packages

import (
	"firebase.google.com/go"
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/misc"
	"golang.org/x/net/context"
	"google.golang.org/appengine/log"
	"net/http"
)

func getVersions(ctx context.Context, app *firebase.App, name string) ([]string, error) {
	client, err := app.Firestore(ctx)
	if err != nil {
		return nil, err
	}
	defer client.Close()

	var versions []string
	collection := client.Collection("packages")
	query := collection.Where("name", "==", name)
	iter := query.Documents(ctx)
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

func handleVersionsCache(c echo.Context, name string) error {
	// Create new firebase app
	ctx, app, err := misc.NewFirebaseApp(c.Request())
	if err != nil {
		return err
	}

	versions, err := getVersions(ctx, app, name)
	if err != nil {
		return err
	}

	if len(versions) == 0 {
		return c.String(http.StatusOK, "null")
	}
	return c.JSON(http.StatusOK, versions)
}

func Versions() echo.HandlerFunc {
	return func(c echo.Context) error {
		name := c.Param("owner") + "/" + c.Param("repo")
		return handleVersionsCache(c, name)
	}
}
