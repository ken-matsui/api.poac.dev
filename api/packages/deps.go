package packages

import (
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/misc"
	"net/http"
)

type DepsBody struct {
	Name  string   `json:"name"`
	Version string `json:"version"`
}

func getDeps(r *http.Request, name string, version string) ([]DepsBody, error) {
	ctx, app, err := misc.NewFirebaseApp(r)
	if err != nil {
		return nil, err
	}

	client, err := app.Firestore(ctx)
	if err != nil {
		return nil, err
	}
	defer client.Close()

	depsBody := []DepsBody{}
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

func Deps() echo.HandlerFunc {
	return func(c echo.Context) error {
		name := c.Param("name")
		version := c.Param("version")

		deps, err := getDeps(c.Request(), name, version)
		if err != nil {
			return err
		}
		if len(deps) == 0 {
			return c.String(http.StatusOK, "null")
		}
		return c.JSON(http.StatusOK, deps)
	}
}

func DepsOrg() echo.HandlerFunc {
	return func(c echo.Context) error {
		org := c.Param("org")
		name := c.Param("name")
		version := c.Param("version")

		deps, err := getDeps(c.Request(), org + "/" + name, version)
		if err != nil {
			return err
		}
		if len(deps) == 0 {
			return c.String(http.StatusOK, "null")
		}
		return c.JSON(http.StatusOK, deps)
	}
}
