package packages

import (
	"github.com/labstack/echo"
	"github.com/poacpm/api.poac.pm/misc"
	"net/http"
)

type DepsBody struct {
	Name  string   `json:"name"`
	Version string `json:"version"`
}

func getDeps(r *http.Request, name string, version string) ([]DepsBody, error) {
	ctx, client, err := misc.NewFirestoreClient(r)
	if err != nil {
		return nil, err
	}

	deps := []DepsBody{}
	iter := client.Collection("packages").Where("name", "==", name).Where("version", "==", version).Documents(ctx)
	for {
		doc, err := iter.Next()
		if err != nil {
			break
		}
		dep := doc.Data()["deps"].(map[string]interface{}) // TODO: これって複数じゃないの？？？
		deps = append(deps, DepsBody{dep["name"].(string), dep["version"].(string)})
	}
	return deps, nil
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
