package packages

import (
	"archive/tar"
	"bytes"
	"compress/gzip"
	"errors"
	"github.com/ghodss/yaml"
	"github.com/labstack/echo/v4"
	"github.com/poacpm/api.poac.pm/api/tokens"
	"github.com/poacpm/api.poac.pm/misc"
	"io"
	"io/ioutil"
	"mime/multipart"
	"net/http"
	"sort"
	"strings"
)

type ValidateBody struct {
	Token  string   `json:"token"`
	Owners []string `json:"owners"`
}

type Package struct {
	Name string
	Config map[string]interface{}
	PackageBuf *bytes.Buffer
	ReadmeBuf map[string][]byte
}

func createPackage(r *http.Request, packageData *Package) error {
	err := misc.CreateDoc(r, packageData.Config)
	if err != nil {
		return err
	}

	// TODO: この中で生成されているcontextはできれば使い回すべき
	err = misc.CreateObject(r, packageData.PackageBuf, packageData.Name+".tar.gz")
	if err != nil {
		return err
	}

	// TODO: If firebase cannot recursive delete in folder
	//  problem(https://stackoverflow.com/a/38215897) is solved,
	//  upload other than README.md.
	if readmeBytes, ok := packageData.ReadmeBuf["README.md"]; ok {
		readmeBuf := bytes.NewBuffer(readmeBytes)
		err = misc.CreateObject(r, readmeBuf, packageData.Name+"/README.md")
		if err != nil {
			return err
		}
	}
	return nil
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

func getOwners(yamlOwners interface{}) []string {
	ownersInterface := yamlOwners.([]interface{})
	owners := make([]string, len(ownersInterface))
	for i, v := range ownersInterface {
		owners[i] = v.(string)
	}
	return owners
}

func checkConfigFile(c echo.Context, packageFileName string, yamlByte []byte) (string, map[string]interface{}, error) {
	config := make(map[string]interface{})
	err := yaml.Unmarshal(yamlByte, &config)
	if err != nil {
		return "", nil, err
	}

	configName := config["name"].(string)
	err = misc.CheckPackageName(configName)
	if err != nil {
		return "", nil, err
	}

	configDesc := config["description"].(string)
	if configDesc == "**TODO: Add description**" {
		errStr := "Invalid description.\nIt is prohibited to use the same as the output of `poac new`."
		return "", nil, errors.New(errStr)
	}

	configCppVersion := config["cpp_version"].(float64)
	files := []float64{98, 3, 11, 14, 17, 20}
	sort.Float64s(files)
	i := sort.Search(len(files),
		func(i int) bool { return files[i] >= configCppVersion })
	if !(i < len(files) && files[i] == configCppVersion) {
		errStr := "Invalid cpp_version.\nPlease select one of 98, 03, 11, 14, 17, 20." // TODO: 詳細ドキュメントを用意する
		return "", nil, errors.New(errStr)
	}

	configVersion := config["version"].(string)
	err = misc.CheckPackageVersion(configVersion)
	if err != nil {
		return "", nil, err
	}

	// package name error
	packageName := strings.Replace(configName, "/", "-", -1) + "-" + configVersion
	if packageName+".tar.gz" != packageFileName {
		errStr := packageFileName + " is an invalid name.\nIt must be replace(name, \"/\", \"-\") + \"-\" + version."
		return "", nil, errors.New(errStr)
	}
	// TODO: name-version.tar.gz -extract> [name-version/poac.yml, ...] if name-version is other it, error

	err = checkExists(c, configName, configVersion)
	if err != nil {
		return "", nil, err
	}

	err = checkToken(c.Request(), c.FormValue("token"), getOwners(config["owners"]))
	if err != nil {
		return "", nil, err
	}

	return packageName, config, nil
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

func extractConfig(packageFile multipart.File) (*bytes.Buffer, map[string][]byte, error) {
	fileBuf := bytes.NewBuffer(nil)
	teeFile := io.TeeReader(packageFile, fileBuf)
	fileBytes, err := unTarGzip(teeFile)
	if err != nil {
		return nil, nil, err
	}
	return fileBuf, fileBytes, nil
}

func loadFile(c echo.Context) (string, multipart.File, error) {
	packageFile, err := c.FormFile("file")
	if err != nil {
		return "", nil, err
	}

	if packageFile.Size > 100000000 {
		return "", nil, errors.New("Allowed under 100MB package only")
	}

	src, err := packageFile.Open()
	return packageFile.Filename, src, err
}

func loadFileAndConfig(c echo.Context) (string, *bytes.Buffer, map[string][]byte, error) {
	packageFileName, packageFile, err := loadFile(c)
	if err != nil {
		return "", nil, nil, err
	}
	defer packageFile.Close()

	fileBuf, fileBytes, err := extractConfig(packageFile)
	if err != nil {
		return "", nil, nil, err
	}
	return packageFileName, fileBuf, fileBytes, nil
}

func Upload() echo.HandlerFunc {
	return func(c echo.Context) error {
		packageFileName, fileBuf, fileBytes, err := loadFileAndConfig(c)
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
		}
		if _, ok := fileBytes["poac.yml"]; !ok {
			return echo.NewHTTPError(http.StatusInternalServerError, "poac.yml does not exist")
		}

		packageName, config, err := checkConfigFile(c, packageFileName, fileBytes["poac.yml"])
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
		}


		//filer.FromZIP()
		//fl, err := filer.FromDirectory("/path/to/project")
		//// https://github.com/src-d/go-license-detector#algorithm
		//licenses, err := licensedb.Detect(fl) // TODO: errorなら，LICENSEが存在しない -> no license file was found -> licenseぽかったら読む
		//for k, v := range licenses {
		//	fmt.Printf("%v, %v\n", k, v)
		//} // GPL-3.0という文字列を含んでいれば，それにする
		// TODO: configの，licenseキーを上書きする必要がある


		err = createPackage(c.Request(), &Package{
			Name: packageName,
			Config: config,
			PackageBuf: fileBuf,
			ReadmeBuf: fileBytes,
		})
		if err != nil {
			return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
		}

		return c.String(http.StatusOK, "ok") // TODO: ok文字列は必要ない
	}
}
