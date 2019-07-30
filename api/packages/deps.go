package packages

import (
	"firebase.google.com/go"
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/misc"
	"golang.org/x/net/context"
	"net/http"
)

type DepsBody struct {
	Name  string   `json:"name"`
	Version string `json:"version"`
}

func getDeps(ctx context.Context, app *firebase.App, name string, version string) ([]DepsBody, error) {
	client, err := app.Firestore(ctx)
	if err != nil {
		return nil, err
	}
	defer client.Close()

	var depsBody []DepsBody
	collection := client.Collection("packages")
	iter := collection.Where("name", "==", name).Where("version", "==", version).Documents(ctx)
	for {
		doc, err := iter.Next()
		if err != nil {
			break
		}

		switch deps := doc.Data()["deps"].(type){
		case map[string]interface{}:
			for name, node := range deps {
				version := node // TODO: tagとか，versionとかの奥にあるかもしれない
				depsBody = append(depsBody, DepsBody{name, version.(string)})
			}
		}
	}
	return depsBody, nil
}

func handleDepsCache(c echo.Context, name string, version string) error {
	// Create new firebase app
	ctx, app, err := misc.NewFirebaseApp(c.Request())
	if err != nil {
		return err
	}

	deps, err := getDeps(ctx, app, name, version)
	if err != nil {
		return err
	}

	if len(deps) == 0 {
		return c.String(http.StatusOK, "null")
	}
	return c.JSON(http.StatusOK, deps)
}

func Deps() echo.HandlerFunc {
	return func(c echo.Context) error {
		name := c.Param("owner") + "/" + c.Param("repo")
		version := c.Param("version")
		return handleDepsCache(c, name, version)
	}
}
