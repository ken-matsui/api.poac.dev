package tokens

import (
	"errors"
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/misc"
	"google.golang.org/api/iterator"
	"google.golang.org/appengine/log"
	"net/http"
)

type ValidateParam struct {
	Owners []string `json:"owners"`
	Token string `json:"token"`
}

func isExistsToken(r *http.Request, token string) (string, error) {
	ctx, app, err := misc.NewFirebaseApp(r)
	if err != nil {
		return "", err
	}

	client, err := app.Firestore(ctx)
	if err != nil {
		return "", err
	}
	defer client.Close()

	dsnap, err := client.Collection("tokens").Doc(token).Get(ctx)
	if err != nil {
		return "", err
	}

	if dsnap.Exists() {
		return dsnap.Data()["owner"].(string), nil
	} else {
		return "", errors.New("err")
	}
}

func getUserDocumentId(r *http.Request, id string) (string, error) {
	ctx, app, err := misc.NewFirebaseApp(r)
	if err != nil {
		return "", err
	}

	client, err := app.Firestore(ctx)
	if err != nil {
		return "", err
	}
	defer client.Close()

	iter := client.Collection("users").Where("id", "==", id).Documents(ctx)
	for {
		doc, err := iter.Next()
		if err == iterator.Done {
			break
		}
		if err != nil {
			log.Debugf(ctx, "Failed to iterate: %v", err)
		}
		return doc.Ref.ID, nil
	}
	return "", errors.New("")
}

func Validate() echo.HandlerFunc {
	return func(c echo.Context) error {
		param := new(ValidateParam)
		if err := c.Bind(param); err != nil {
			return err
		}

		tokenOwner, err := isExistsToken(c.Request(), param.Token)
		if err != nil {
			return err
		}

		for _, v := range param.Owners {
			userDocId, err := getUserDocumentId(c.Request(), v)
			if err != nil {
				return err
			}
			if userDocId == tokenOwner {
				return c.String(http.StatusOK, "ok")
			}
		}
		return c.String(http.StatusOK, "err") // TODO: statusのみでやりとりすべき
	}
}
