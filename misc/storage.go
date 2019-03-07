package misc

import (
	"bytes"
	"cloud.google.com/go/storage"
	"google.golang.org/appengine"
	"google.golang.org/appengine/file"
	"google.golang.org/appengine/log"
	"net/http"
)

func CreateObject(r *http.Request, fileBuf *bytes.Buffer, objName string) error {
	ctx := appengine.NewContext(r)
	client, err := storage.NewClient(ctx)
	if err != nil {
		log.Debugf(ctx, "failed to create gcs client : %v", err)
		return err
	}

	// GCS writer
	bucketName, err := file.DefaultBucketName(ctx)
	if err != nil {
		return err
	}
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
