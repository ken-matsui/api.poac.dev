// +build !appengine

package misc_test

import (
	"github.com/poacpm/api.poac.pm/misc"
	"testing"
)

func TestCheckPackageName(t *testing.T) {
	err := misc.CheckPackageName("/-_1")
	if err == nil {
		t.Fatal("failed test")
	}

	err = misc.CheckPackageName("/test")
	if err == nil {
		t.Fatalf("failed test %#v", err)
	}
	err = misc.CheckPackageName("-test")
	if err == nil {
		t.Fatalf("failed test %#v", err)
	}
	err = misc.CheckPackageName("_test")
	if err == nil {
		t.Fatalf("failed test %#v", err)
	}

	err = misc.CheckPackageName("test/")
	if err == nil {
		t.Fatalf("failed test %#v", err)
	}
	err = misc.CheckPackageName("test-")
	if err == nil {
		t.Fatalf("failed test %#v", err)
	}
	err = misc.CheckPackageName("test_")
	if err == nil {
		t.Fatalf("failed test %#v", err)
	}

	err = misc.CheckPackageName("t//est")
	if err == nil {
		t.Fatalf("failed test %#v", err)
	}
	err = misc.CheckPackageName("t--est")
	if err == nil {
		t.Fatalf("failed test %#v", err)
	}
	err = misc.CheckPackageName("t__est")
	if err == nil {
		t.Fatalf("failed test %#v", err)
	}
	err = misc.CheckPackageName("t/e/st")
	if err == nil {
		t.Fatalf("failed test %#v", err)
	}
	err = misc.CheckPackageName("t-e-st")
	if err == nil {
		t.Fatalf("failed test %#v", err)
	}
	err = misc.CheckPackageName("t_e_st")
	if err == nil {
		t.Fatalf("failed test %#v", err)
	}
	err = misc.CheckPackageName("t/e-st")
	if err == nil {
		t.Fatalf("failed test %#v", err)
	}
	err = misc.CheckPackageName("t-e_st")
	if err == nil {
		t.Fatalf("failed test %#v", err)
	}
	err = misc.CheckPackageName("t_e/st")
	if err == nil {
		t.Fatalf("failed test %#v", err)
	}

	err = misc.CheckPackageName("test")
	if err != nil {
		t.Fatalf("failed test %#v", err)
	}

	err = misc.CheckPackageName("*test")
	if err == nil {
		t.Fatal("failed test")
	}
}
