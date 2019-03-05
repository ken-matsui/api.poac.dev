package packages

import (
	"archive/tar"
	"bytes"
	"cloud.google.com/go/storage"
	"compress/gzip"
	"errors"
	"github.com/blang/semver"
	"github.com/ghodss/yaml"
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/api/tokens"
	"github.com/poacpm/api.poac.pm/misc"
	"golang.org/x/net/context"
	"google.golang.org/appengine"
	"google.golang.org/appengine/log"
	"io"
	"io/ioutil"
	"net/http"
	"regexp"
	"sort"
	"strings"
)

type ValidateBody struct {
	Token  string   `json:"token"`
	Owners []string `json:"owners"`
}

func createDoc(r *http.Request, config map[string]interface{}) error {
	ctx, app, err := misc.NewFirebaseApp(r)
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
func checkToken(r *http.Request, token string, owners []string) error {
	res, err := tokens.ValidateImpl(r, token, owners)
	if err != nil {
		return err
	}

	if res == "ok" {
		return nil
	} else {
		return errors.New("Token verification failed.\n" +
			"Please check the following check lists.\n" +
			"1. Does token really belong to you?\n" +
			"2. Is the user ID described `owners` in poac.yml\n" +
			"    the same as that of GitHub account?")
	}
}

func checkExists(c echo.Context, name string, version string) error {
	isExists, err := handleExists(c, name, version)
	if err == nil {
		return err
	}

	if isExists {
		return errors.New("Already " + name + ": " + version + "is exists")
	} else {
		return nil
	}
}

func getOwners(yamlOwners interface{}) []string {
	ownersInterface := yamlOwners.([]interface{})
	owners := make([]string, len(ownersInterface))
	for i, v := range ownersInterface {
		owners[i] = v.(string)
	}
	return owners
}

func checkConfigName(name string) error {
	if regexp.MustCompile("^(\\/|\\-|_|\\d)+$").Match([]byte(name)) {
		errStr := "Invalid name.\nIt is prohibited to use / and -, _, number only string of the project name."
		return echo.NewHTTPError(http.StatusInternalServerError, errStr)
	} else if regexp.MustCompile("^(\\/|\\-|_)$").Match([]byte(string(name[0]))) {
		errStr := "Invalid name.\nIt is prohibited to use / and -, _ \n at the begenning of the project name."
		return echo.NewHTTPError(http.StatusInternalServerError, errStr)
	} else if regexp.MustCompile("^(\\/|\\-|_)$").Match([]byte(string(name[len(name)-1]))) {
		errStr := "Invalid name.\nIt is prohibited to use / and -, _ \n at the last of the project name."
		return echo.NewHTTPError(http.StatusInternalServerError, errStr)
	} else if regexp.MustCompile("^.*(\\/|\\-|_){2,}.*$").Match([]byte(name)) {
		errStr := "Invalid name.\nIt is prohibited to use / and -, _ \n twice of the project name."
		return echo.NewHTTPError(http.StatusInternalServerError, errStr)
	} else if strings.Count(name, "/") > 1 {
		errStr := "Invalid name.\nIt is prohibited to use two\n /(slashes) in the project name."
		return echo.NewHTTPError(http.StatusInternalServerError, errStr)
	} else if !regexp.MustCompile("^([a-z|\\d|\\-|_|\\/]*)$").Match([]byte(name)) {
		errStr := "Invalid name.\nIt is prohibited to use a character string that does not match ^([a-z|\\d|\\-|_|\\/]*)$ in the project name."
		return echo.NewHTTPError(http.StatusInternalServerError, errStr)
	} else {
		return nil
	}
}

func checkConfigFile(yamlByte []byte) (map[string]interface{}, string, string, error) {
	config := make(map[string]interface{})
	err := yaml.Unmarshal(yamlByte, &config)
	if err != nil {
		return nil, "", "", echo.NewHTTPError(http.StatusInternalServerError, err.Error())
	}

	configName := config["name"].(string)
	err = checkConfigName(configName)
	if err != nil {
		return nil, "", "", err
	}

	configDesc := config["description"].(string)
	if configDesc == "**TODO: Add description**" {
		errStr := "Invalid description.\nIt is prohibited to use the same as the output of `poac new`."
		return nil, "", "", echo.NewHTTPError(http.StatusInternalServerError, errStr)
	}

	configCppVersion := config["cpp_version"].(float64)
	files := []float64{98, 3, 11, 14, 17, 20}
	sort.Float64s(files)
	i := sort.Search(len(files),
		func(i int) bool { return files[i] >= configCppVersion })
	if !(i < len(files) && files[i] == configCppVersion) {
		errStr := "Invalid cpp_version.\nPlease select one of 98, 03, 11, 14, 17, 20." // TODO: 詳細ドキュメントを用意する
		return nil, "", "", echo.NewHTTPError(http.StatusInternalServerError, errStr)
	}

	// semver error
	configVersion := config["version"].(string)
	_, err = semver.Make(configVersion)
	if err != nil {
		errStr := "Invalid version.\nPlease adapt to semver.\nSee https://semver.org for details."
		return nil, "", "", echo.NewHTTPError(http.StatusInternalServerError, errStr)
	}

	return config, configName, configVersion, nil
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

func Upload() echo.HandlerFunc {
	return func(c echo.Context) error {
		file, err := c.FormFile("file")
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
		}

		if file.Size > 100000000 {
			return echo.NewHTTPError(http.StatusInternalServerError, "Allowed under 100MB package only")
		}

		src, err := file.Open()
		if err != nil {
			return err
		}
		defer src.Close()

		// Create file buffer and reader
		fileBuf := bytes.NewBuffer(nil)
		teeFile := io.TeeReader(src, fileBuf)

		fileBytes, err := unTarGzip(teeFile)
		if _, ok := fileBytes["poac.yml"]; !ok {
			return echo.NewHTTPError(http.StatusInternalServerError, "poac.yml does not exist")
		}

		config, configName, configVersion, err := checkConfigFile(fileBytes["poac.yml"])
		if err != nil {
			return err
		}

		// package name error
		packageName := strings.Replace(configName, "/", "-", -1) + "-" + configVersion
		if packageName+".tar.gz" != file.Filename {
			errStr := file.Filename + " is an invalid name.\nIt must be replace(name, \"/\", \"-\") + \"-\" + version."
			return echo.NewHTTPError(http.StatusInternalServerError, errStr)
		}
		// TODO: name-version.tar.gz -extract> [name-version/poac.yml, ...] if name-version is other it, error


		err = checkExists(c, configName, configVersion)
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
		}

		token := c.FormValue("token")
		owners := getOwners(config["owners"])

		request := c.Request()
		err = checkToken(request, token, owners)
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
		}

		err = createDoc(request, config)
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
		}

		ctx := appengine.NewContext(request)
		err = createObject(ctx, fileBuf, file.Filename)
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
		}

		// TODO: If firebase cannot recursive delete in folder
		//  problem(https://stackoverflow.com/a/38215897) is solved,
		//  upload other than README.md.
		if readmeBytes, ok := fileBytes["README.md"]; ok {
			readmeBuf := bytes.NewBuffer(readmeBytes)
			err = createObject(ctx, readmeBuf, packageName+"/README.md")
			if err != nil {
				return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
			}
		}

		return c.String(http.StatusOK, "ok") // TODO: ok文字列は必要ない
	}
}
