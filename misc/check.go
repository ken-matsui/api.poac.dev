package misc

import (
	"errors"
	"github.com/blang/semver"
	"regexp"
)

func CheckPackageName(name string) error {
	if regexp.MustCompile("^(\\/|\\-|_|\\d)+$").Match([]byte(name)) {
		errStr := "Invalid name.\nIt is prohibited to use only / or -, _, number for the project name."
		return errors.New(errStr)
	} else if regexp.MustCompile("^(\\/|\\-|_)$").Match([]byte(string(name[0]))) {
		errStr := "Invalid name.\nIt is prohibited to use / or -, _ \n at the begenning for the project name."
		return errors.New(errStr)
	} else if regexp.MustCompile("^(\\/|\\-|_)$").Match([]byte(string(name[len(name)-1]))) {
		errStr := "Invalid name.\nIt is prohibited to use / or -, _ \n at the last for the project name."
		return errors.New(errStr)
	} else if regexp.MustCompile("^.*(\\/|\\-|_).*(\\/|\\-|_).*$").Match([]byte(name)) {
		errStr := "Invalid name.\nIt is prohibited to use / or -, _ \n twice for the project name."
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
