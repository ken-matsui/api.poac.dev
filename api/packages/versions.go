package packages

import (
	"firebase.google.com/go"
	"fmt"
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/misc"
	"golang.org/x/net/context"
	"google.golang.org/appengine/log"
	"google.golang.org/appengine/memcache"
	"net/http"
	"strconv"
	"unsafe"
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

	// Get the item from the memcache
	memcacheKey := "versions/" + name
	if item, err := memcache.Get(ctx, memcacheKey); err != nil {
		versions, err := getVersions(ctx, app, name)
		if err != nil {
			return err
		}

		misc.MemcacheAddIndexSize(ctx, memcacheKey, len(versions))
		for i, ver := range versions {
			memcacheKeyIndex := memcacheKey + "/" + strconv.Itoa(i)
			misc.MemcacheAdd(ctx, memcacheKeyIndex, []byte(ver))
		}

		if len(versions) == 0 {
			return c.String(http.StatusOK, "null")
		}
		return c.JSON(http.StatusOK, versions)
	} else {
		versionsSizeStr := *(*string)(unsafe.Pointer(&item.Value))
		versionsSize, _ := strconv.Atoi(versionsSizeStr)

		var versions []string
		for i := 0; i < versionsSize; i++ {
			if item2, err := memcache.Get(ctx, memcacheKey + "/" + strconv.Itoa(i)); err != nil {
				// TODO: detect error
			} else {
				ver := fmt.Sprintf("%s", item2.Value)
				versions = append(versions, ver)
			}
		}

		if len(versions) == 0 {
			return c.String(http.StatusOK, "null")
		}
		return c.JSON(http.StatusOK, versions)
	}
}

func Versions() echo.HandlerFunc {
	return func(c echo.Context) error {
		name := c.Param("owner") + "/" + c.Param("repo")
		return handleVersionsCache(c, name)
	}
}
