package main

import (
	"bytes"
	"cloud.google.com/go/storage"
	"encoding/json"
	"errors"
	"firebase.google.com/go"
	"fmt"
	"github.com/ghodss/yaml"
	"golang.org/x/net/context"
	"google.golang.org/appengine"
	"google.golang.org/appengine/log"
	"google.golang.org/appengine/urlfetch"
	"io"
	"io/ioutil"
	"mime/multipart"
	"net/http"
	"strconv"
)

type ValidateBody struct {
	Token  string   `json:"token"`
	Owners []string `json:"owners"`
}

func createDoc(ctx context.Context, config map[string]interface{}) error {
	projectID := "poac-pm"
	conf := &firebase.Config{ProjectID: projectID}
	app, err := firebase.NewApp(ctx, conf)
	if err != nil {
		return err
	}

	client, err := app.Firestore(ctx)
	if err != nil {
		return err
	}
	defer client.Close()

	_, _, err = client.Collection("packages").Add(ctx, config)
	if err != nil {
		log.Debugf(ctx, "Failed adding collection: %v", err)
		return err
	}
	return nil
}

func createObject(ctx context.Context, file multipart.File, fileHeader *multipart.FileHeader) error {
	bucketName := "poac-pm.appspot.com"
	objName := fileHeader.Filename

	client, err := storage.NewClient(ctx)
	if err != nil {
		log.Debugf(ctx, "failed to create gcs client : %v", err)
		return err
	}

	// GCS writer
	writer := client.Bucket(bucketName).Object(objName).NewWriter(ctx)

	// Create file buffer
	buf := bytes.NewBuffer(nil)
	if _, err := io.Copy(buf, file); err != nil {
		return err
	}

	// upload : write object body
	if _, err := writer.Write(buf.Bytes()); err != nil {
		log.Debugf(ctx, "failed to write object body : %v", err)
		return err
	}

	if err := writer.Close(); err != nil {
		log.Debugf(ctx, "failed to close gcs writer : %v", err)
		return err
	}
	return nil
}

// TODO: If exist it and match owner, can update it. (next version)
func checkToken(ctx context.Context, token string, owners []string) error {
	validateUrl := "https://poac.pm/api/tokens/validate"
	contentType := "application/json"
	body := ValidateBody{
		Token:  token,
		Owners: owners,
	}
	jsonBody, err := json.Marshal(body)
	if err != nil {
		return err
	}

	log.Debugf(ctx, "jsonBody %v", string(jsonBody))

	client := urlfetch.Client(ctx)
	resp, err := client.Post(validateUrl, contentType, bytes.NewBuffer(jsonBody))
	if err == nil {
		return err
	}
	defer resp.Body.Close()

	byteArray, _ := ioutil.ReadAll(resp.Body)
	if string(byteArray) == "ok" {
		return nil
	} else {
		return errors.New("token validation error")
	}
}

func checkExists(ctx context.Context, name string, version string) error {
	existsUrl := "https://poac.pm/api/packages/" + name + "/" + version + "/exists"
	client := urlfetch.Client(ctx)
	resp, err := client.Get(existsUrl)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	byteArray, _ := ioutil.ReadAll(resp.Body)
	isExists, err := strconv.ParseBool(string(byteArray))
	if err == nil {
		return err
	}

	if isExists {
		return errors.New("Already " + name + ": " + version + "is exists")
	} else {
		return nil
	}
}

func handle(w http.ResponseWriter, r *http.Request) {
	if r.Method != "POST" {
		http.Error(w, "Allowed POST method only", http.StatusMethodNotAllowed)
		return
	}

	// max memory
	err := r.ParseMultipartForm(32 << 20)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	file, fileHeader, err := r.FormFile("file")
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	defer file.Close()

	if fileHeader.Size > 100000000 {
		http.Error(w, "Allowed under 100MB package only", http.StatusInternalServerError)
		return
	}

	configString := r.FormValue("config")
	config := make(map[string]interface{})
	err = yaml.Unmarshal([]byte(configString), &config)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	ctx := appengine.NewContext(r)

	err = checkExists(ctx, config["name"].(string), config["version"].(string))
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	token := r.FormValue("token")
	ownersInterface := config["owners"].([]interface{})
	owners := make([]string, len(ownersInterface))
	for i, v := range ownersInterface {
		owners[i] = v.(string)
	}

	err = checkToken(ctx, token, owners)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	err = createObject(ctx, file, fileHeader)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	err = createDoc(ctx, config)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	fmt.Fprintln(w, "ok")
}

func main() {
	http.HandleFunc("/packages/upload", handle)
	appengine.Main()
}
