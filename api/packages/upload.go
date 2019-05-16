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

type Links struct {
	Github string `json:"github"`
	Homepage string `json:"homepage"`
}

type Config struct {
	// Loaded config
	Name string `json:"name"`
	Version string `json:"version"`
	CppVersion float64 `json:"cpp_version"`
	Description string `json:"description"`
	Owners []string `json:"owners"` // TODO: gitconfigの情報を元に構築できるかも
	License string `json:"license"` // TODO: 将来的に，Auto generated configへ移動
	Links Links `json:"links"`
	Deps map[string]interface{} `json:"deps"`
	Build map[string]interface{} `json:"build"`
	Test map[string]interface{} `json:"test"`
	// Auto generated config
	PackageType string `json:"package_type"`
}

type Package struct {
	Name string
	Config Config
	PackageBuf *bytes.Buffer
	ReadmeBuf map[string][]byte
}

func EmptyToNull(m map[string]interface{}) {
	if m["license"] == "" {
		m["license"] = nil
	}
	if m["links"].(map[string]interface{})["github"] == "" {
		m["links"].(map[string]interface{})["github"] = nil
	}
	if m["links"].(map[string]interface{})["homepage"] == "" {
		m["links"].(map[string]interface{})["homepage"] = nil
	}
}

func createPackage(r *http.Request, packageData *Package) error {
	configMap, err := misc.StructToJsonTagMap(packageData.Config)
	if err != nil {
		return err
	}

	// structの状態ではnullにできなかったので""(空文字)にしていた．
	// それを，mapに変換後の状態を利用して，nullにする．
	EmptyToNull(configMap)

	err = misc.CreateDoc(r, configMap)
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
	_, err := tokens.ValidateImpl(r, token, owners)
	if err != nil {
		return err
	}
	return nil
}

func getOwners(yamlOwners interface{}) []string {
	ownersInterface := yamlOwners.([]interface{})
	owners := make([]string, len(ownersInterface))
	for i, v := range ownersInterface {
		owners[i] = v.(string)
	}
	return owners
}

func getLinks(yamlLinks interface{}) Links {
	if yamlLinks == nil {
		return Links{}
	}

	links := Links{}

	LinksInterface := yamlLinks.(map[string]interface{})
	if homepage, ok := LinksInterface["homepage"]; ok {
		links.Homepage = homepage.(string)
	} else {
		links.Homepage = ""
	}
	if github, ok := LinksInterface["github"]; ok {
		links.Github = github.(string)
	} else {
		links.Github = ""
	}

	return links
}

func checkConfigFile(c echo.Context, packageFileName string, yamlByte []byte) (string, Config, error) {
	loadedConfig := make(map[string]interface{})
	err := yaml.Unmarshal(yamlByte, &loadedConfig)
	if err != nil {
		return "", Config{}, err
	}

	config := Config{}

	config.Name = loadedConfig["name"].(string)
	err = misc.CheckPackageName(config.Name)
	if err != nil {
		return "", Config{}, err
	}

	config.Description = loadedConfig["description"].(string)
	if config.Description == "**TODO: Add description**" {
		errStr := "Invalid description.\nIt is prohibited to use the same as the output of `poac new`."
		return "", Config{}, errors.New(errStr)
	}

	config.CppVersion = loadedConfig["cpp_version"].(float64)
	files := []float64{98, 3, 11, 14, 17, 20}
	sort.Float64s(files)
	i := sort.Search(len(files),
		func(i int) bool { return files[i] >= config.CppVersion })
	if !(i < len(files) && files[i] == config.CppVersion) {
		errStr := "Invalid cpp_version.\nPlease select one of 98, 03, 11, 14, 17, 20." // TODO: 詳細ドキュメントを用意する
		return "", Config{}, errors.New(errStr)
	}

	config.Version = loadedConfig["version"].(string)
	err = misc.CheckPackageVersion(config.Version)
	if err != nil {
		return "", Config{}, err
	}

	// package name error
	packageName := strings.Replace(config.Name, "/", "-", -1) + "-" + config.Version
	if packageName+".tar.gz" != packageFileName {
		errStr := packageFileName + " is an invalid name.\nIt must be replace(name, \"/\", \"-\") + \"-\" + version."
		return "", Config{}, errors.New(errStr)
	}
	// TODO: name-version.tar.gz -extract> [name-version/poac.yml, ...] if name-version is other it, error

	err = checkExists(c, config.Name, config.Version)
	if err != nil {
		return "", Config{}, err
	}

	config.Owners = getOwners(loadedConfig["owners"])
	err = checkToken(c.Request(), c.FormValue("token"), config.Owners)
	if err != nil {
		return "", Config{}, err
	}

	// ここまできたなら，config.Name, Description, Version, CppVersionがfillされている．
	// 残りの情報をfillする．具体的には以下に挙げるもの．
	// License, Links, Deps, Build, Test,
	// BuildFlag
	if license, ok := loadedConfig["license"]; ok {
		config.License = license.(string)
	} else {
		config.License = ""
	}
	config.Links = getLinks(loadedConfig["links"])
	if deps, ok := loadedConfig["deps"]; ok {
		config.Deps = deps.(map[string]interface{})
	} else {
		config.Deps = nil
	}
	if build, ok := loadedConfig["build"]; ok {
		config.Build = build.(map[string]interface{})
		// binがtrueなら，PackageTypeは，applicationになる．
		if bin, ok := config.Build["bin"]; ok {
			if bin.(bool) {
				config.PackageType = "application"
			} else {
				config.PackageType = "build-required library"
			}
		} else { // binが存在しない -> false扱い
			config.PackageType = "build-required library"
		}
	} else {
		config.Build = nil
		config.PackageType = "header-only library"
	}
	if test, ok := loadedConfig["test"]; ok {
		config.Test = test.(map[string]interface{})
	} else {
		config.Test = nil
	}

	return packageName, config, nil
}

func unTarGz(fileBuf io.Reader) (map[string][]byte, error) {
	gzipReader, err := gzip.NewReader(fileBuf)
	if err != nil {
		return nil, err
	}
	defer gzipReader.Close()

	tarReader := tar.NewReader(gzipReader)

	buffers := make(map[string][]byte)
	for {
		tarHeader, err := tarReader.Next()
		if err == io.EOF {
			break
		}
		if err != nil {
			return nil, err
		}

		// name-version/poac.yml
		if strings.Count(tarHeader.Name, "/") == 1 && strings.Contains(tarHeader.Name, "poac.yml") {
			buf, err := ioutil.ReadAll(tarReader)
			if err != nil {
				return nil, err
			}
			buffers["poac.yml"] = buf
		} else if strings.Count(tarHeader.Name, "/") == 1 && strings.Contains(tarHeader.Name, "README.md") {
			buf, err := ioutil.ReadAll(tarReader)
			if err != nil {
				return nil, err
			}
			buffers["README.md"] = buf
		}
	}
	return buffers, nil
}

func extractConfig(packageFile multipart.File) (*bytes.Buffer, map[string][]byte, error) {
	fileBuf := bytes.NewBuffer(nil)
	teeFile := io.TeeReader(packageFile, fileBuf)
	fileBytes, err := unTarGz(teeFile)
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
	if err != nil {
		return "", nil, err
	}

	return packageFile.Filename, src, nil
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
			return err
		}
		if _, ok := fileBytes["poac.yml"]; !ok {
			return echo.NewHTTPError(http.StatusInternalServerError, "poac.yml does not exist")
		}

		packageName, config, err := checkConfigFile(c, packageFileName, fileBytes["poac.yml"])
		if err != nil {
			return err
		}

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
