package main

import (
	"archive/tar"
	"bytes"
	"cloud.google.com/go/storage"
	"compress/gzip"
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
	"net/http"
	"regexp"
	"strconv"
	"strings"
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

func createObject(ctx context.Context, fileBuf *bytes.Buffer, objName string) error {
	client, err := storage.NewClient(ctx)
	if err != nil {
		log.Debugf(ctx, "failed to create gcs client : %v", err)
		return err
	}

	// GCS writer
	bucketName := "poac-pm.appspot.com"
	writer := client.Bucket(bucketName).Object(objName).NewWriter(ctx)

	// upload : write object body
	if _, err := writer.Write(fileBuf.Bytes()); err != nil {
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
		return errors.New("Token verification failed.\n" +
			"Please check the following check lists.\n" +
			"1. Does token really belong to you?\n" +
			"2. Is the user ID described `owners` in poac.yml\n" +
			"    the same as that of GitHub account?")
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

func unTarGzip(fileBuf io.Reader) (map[string][]byte, error) {
	gzipReader, err := gzip.NewReader(fileBuf)
	if err != nil {
		return map[string][]byte{}, err
	}
	defer gzipReader.Close()

	tarReader := tar.NewReader(gzipReader)

	buffers := make(map[string][]byte)
	for {
		tarHeader, err := tarReader.Next()
		if err == io.EOF {
			break
		}
		// name-version/poac.yml
		if strings.Count(tarHeader.Name, "/") == 1 && strings.Contains(tarHeader.Name, "poac.yml") {
			buf, err := ioutil.ReadAll(tarReader)
			if err != nil {
				return map[string][]byte{}, err
			}
			buffers["poac.yml"] = buf
		} else if strings.Count(tarHeader.Name, "/") == 1 && strings.Contains(tarHeader.Name, "README.md") {
			buf, err := ioutil.ReadAll(tarReader)
			if err != nil {
				return map[string][]byte{}, err
			}
			buffers["README.md"] = buf
		}
	}
	return buffers, nil
}

func handle(w http.ResponseWriter, r *http.Request) {
	ctx := appengine.NewContext(r)

	log.Infof(ctx, "Function started")

	if r.Method != "POST" {
		http.Error(w, "Allowed POST method only", http.StatusMethodNotAllowed)
		return
	}

	log.Infof(ctx, "Checked method")

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

	// Create file buffer and reader
	fileBuf := bytes.NewBuffer(nil)
	teeFile := io.TeeReader(file, fileBuf)

	fileBytes, err := unTarGzip(teeFile)
	if _, ok := fileBytes["poac.yml"]; !ok {
		http.Error(w, "poac.yml does not exist", http.StatusInternalServerError)
		return
	}

	config := make(map[string]interface{})
	err = yaml.Unmarshal(fileBytes["poac.yml"], &config)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	configName := config["name"].(string)
	if regexp.MustCompile("^(\\/|\\-|_|\\d)+$").Match([]byte(configName)) {
		errStr := "Invalid name.\nIt is prohibited to use / and -, _, number only string of the project name."
		http.Error(w, errStr, http.StatusInternalServerError)
		return
	} else if regexp.MustCompile("^(\\/|\\-|_)$").Match([]byte(string(configName[0]))) {
		errStr := "Invalid name.\nIt is prohibited to use / and -, _ \n at the begenning of the project name."
		http.Error(w, errStr, http.StatusInternalServerError)
		return
	} else if regexp.MustCompile("^(\\/|\\-|_)$").Match([]byte(string(configName[len(configName)]))) {
		errStr := "Invalid name.\nIt is prohibited to use / and -, _ \n at the last of the project name."
		http.Error(w, errStr, http.StatusInternalServerError)
		return
	} else if regexp.MustCompile("^.*(\\/|\\-|_){2,}.*$").Match([]byte(configName)) {
		errStr := "Invalid name.\nIt is prohibited to use / and -, _ \n twice of the project name."
		http.Error(w, errStr, http.StatusInternalServerError)
		return
	} else if strings.Count(configName, "/") > 1 {
		errStr := "Invalid name.\nIt is prohibited to use two\n /(slashes) in the project name."
		http.Error(w, errStr, http.StatusInternalServerError)
		return
	} else if !regexp.MustCompile("^([a-z|\\d|\\-|_|\\/]*)$").Match([]byte(configName)) {
		errStr := "Invalid name.\nIt is prohibited to use a character string that does not match ^([a-z|\\d|\\-|_|\\/]*)$ in the project name."
		http.Error(w, errStr, http.StatusInternalServerError)
		return
	}

	configVersion := config["version"].(string)
	packageName := strings.Replace(configName, "/", "-", -1) + "-" + configVersion
	if packageName+".tar.gz" != fileHeader.Filename {
		errStr := fileHeader.Filename + " is an invalid name.\nIt must be replace(name, \"/\", \"-\") + \"-\" + version."
		http.Error(w, errStr, http.StatusInternalServerError)
		return
	}
	// TODO: name-version.tar.gz -extract> [name-version/poac.yml, ...] if name-version is other it, error

	err = checkExists(ctx, configName, configVersion)
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

	err = createDoc(ctx, config)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	err = createObject(ctx, fileBuf, fileHeader.Filename)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	// TODO: If firebase cannot recursive delete in folder problem(https://stackoverflow.com/a/38215897) is solved,
	//  upload other than README.md.
	if readmeBytes, ok := fileBytes["README.md"]; ok {
		readmeBuf := bytes.NewBuffer(readmeBytes)
		err = createObject(ctx, readmeBuf, packageName+"/README.md")
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
	}

	fmt.Fprintln(w, "ok")
}

func main() {
	http.HandleFunc("/packages/upload", handle)
	appengine.Main()
}
