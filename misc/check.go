package misc

import (
	"errors"
	"github.com/blang/semver"
	"regexp"
	"strings"
)

func CheckPackageName(name string) error {
	if regexp.MustCompile("^(\\/|\\-|_|\\d)+$").Match([]byte(name)) {
		errStr := "Invalid name.\nIt is prohibited to use / and -, _, number only string of the project name."
		return errors.New(errStr)
	} else if regexp.MustCompile("^(\\/|\\-|_)$").Match([]byte(string(name[0]))) {
		errStr := "Invalid name.\nIt is prohibited to use / and -, _ \n at the begenning of the project name."
		return errors.New(errStr)
	} else if regexp.MustCompile("^(\\/|\\-|_)$").Match([]byte(string(name[len(name)-1]))) {
		errStr := "Invalid name.\nIt is prohibited to use / and -, _ \n at the last of the project name."
		return errors.New(errStr)
	} else if regexp.MustCompile("^.*(\\/|\\-|_){2,}.*$").Match([]byte(name)) {
		errStr := "Invalid name.\nIt is prohibited to use / and -, _ \n twice of the project name."
		return errors.New(errStr)
	} else if strings.Count(name, "/") > 1 {
		errStr := "Invalid name.\nIt is prohibited to use two\n /(slashes) in the project name."
		return errors.New(errStr)
	} else if !regexp.MustCompile("^([a-z|\\d|\\-|_|\\/]*)$").Match([]byte(name)) {
		errStr := "Invalid name.\nIt is prohibited to use a character string that does not match ^([a-z|\\d|\\-|_|\\/]*)$ in the project name."
		return errors.New(errStr)
	} else {
		return nil
	}
}

func CheckPackageVersion(version string) error {
	// semver error
	_, err := semver.Make(version)
	if err != nil {
		errStr := "Invalid version.\nPlease adapt to semver.\nSee https://semver.org for details."
		return errors.New(errStr)
	} else {
		return nil
	}
}
