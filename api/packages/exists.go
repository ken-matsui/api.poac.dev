package packages

import (
	"firebase.google.com/go"
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/misc"
	"golang.org/x/net/context"
	"google.golang.org/appengine/memcache"
	"net/http"
	"strconv"
	"unsafe"
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

func handleExistsCache(c echo.Context, name string, version string) error {
	// Create new firebase app
	ctx, app, err := misc.NewFirebaseApp(c.Request())
	if err != nil {
		return err
	}

	// Get the item from the memcache
	memcacheKey := "exists/" + name + "/" + version
	if item, err := memcache.Get(ctx, memcacheKey); err != nil {
		isExists, err := getExists(ctx, app, name, version)
		if err != nil {
			return err
		}

		// Create an Item
		isExistsStr := strconv.FormatBool(isExists)
		item := &memcache.Item{
			Key:   memcacheKey,
			Value: *(*[]byte)(unsafe.Pointer(&isExistsStr)),
		}
		// Add the item to the memcache, if the key does not already exist
		_ = memcache.Add(ctx, item)

		return c.String(http.StatusOK, strconv.FormatBool(isExists))
	} else {
		return c.String(http.StatusOK, *(*string)(unsafe.Pointer(&item.Value)))
	}
}

func Exists() echo.HandlerFunc {
	return func(c echo.Context) error {
		name := c.Param("name")
		version := c.Param("version")
		return handleExistsCache(c, name, version)
	}
}

func ExistsOrg() echo.HandlerFunc {
	return func(c echo.Context) error {
		org := c.Param("org")
		name := c.Param("name")
		version := c.Param("version")
		return handleExistsCache(c, org + "/" + name, version)
	}
}
