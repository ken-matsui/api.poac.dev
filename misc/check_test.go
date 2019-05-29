// +build !appengine

package misc_test

import (
	"github.com/poacpm/api.poac.pm/misc"
	"testing"
)

func TestCheckPackageName1(t *testing.T) {
	err := misc.CheckPackageName("hoge")
	if err != nil {
		t.Fatalf("failed test %#v", err)
	}
}

func TestCheckPackageName2(t *testing.T) {
	err := misc.CheckPackageName("*hoge")
	if err == nil {
		t.Fatal("failed test")
	}
}
