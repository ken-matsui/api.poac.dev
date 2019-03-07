package packages

import (
	"encoding/json"
	"firebase.google.com/go"
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/misc"
	"golang.org/x/net/context"
	"google.golang.org/appengine/memcache"
	"net/http"
	"strconv"
	"unsafe"
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

	// Get the item from the memcache
	memcacheKey := "deps/" + name + "/" + version
	if item, err := memcache.Get(ctx, memcacheKey); err != nil {
		deps, err := getDeps(ctx, app, name, version)
		if err != nil {
			return err
		}

		misc.MemcacheAddIndexSize(ctx, memcacheKey, len(deps))
		for i, dep := range deps {
			memcacheKeyIndex := memcacheKey + "/" + strconv.Itoa(i)
			depValue := []byte(`{"name":"` + dep.Name + `","version":"` + dep.Version + `"}`)
			misc.MemcacheAdd(ctx, memcacheKeyIndex, depValue)
		}

		if len(deps) == 0 {
			return c.String(http.StatusOK, "null")
		}
		return c.JSON(http.StatusOK, deps)
	} else {
		depsSizeStr := *(*string)(unsafe.Pointer(&item.Value))
		depsSize, _ := strconv.Atoi(depsSizeStr)

		var deps []DepsBody
		for i := 0; i < depsSize; i++ {
			if item2, err := memcache.Get(ctx, memcacheKey + "/" + strconv.Itoa(i)); err != nil {
				// TODO: detect error
			} else {
				var dep DepsBody
				if err := json.Unmarshal(item2.Value, &dep); err != nil {
					panic(err) // TODO:
				}
				deps = append(deps, dep)
			}
		}

		if len(deps) == 0 {
			return c.String(http.StatusOK, "null")
		}
		return c.JSON(http.StatusOK, deps)
	}
}

func Deps() echo.HandlerFunc {
	return func(c echo.Context) error {
		name := c.Param("name")
		version := c.Param("version")
		return handleDepsCache(c, name, version)
	}
}

func DepsOrg() echo.HandlerFunc {
	return func(c echo.Context) error {
		name := c.Param("org") + "/" + c.Param("name")
		version := c.Param("version")
		return handleDepsCache(c, name, version)
	}
}
