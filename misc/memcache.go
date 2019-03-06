package misc

import (
	"golang.org/x/net/context"
	"google.golang.org/appengine/memcache"
	"strconv"
)

func MemcacheAdd(ctx context.Context, key string, value []byte) {
	// Create an Item
	item := &memcache.Item{
		Key:   key,
		Value: value,
	}
	// Add the item to the memcache, if the key does not already exist
	_ = memcache.Add(ctx, item)
}

func MemcacheAddIndexSize(ctx context.Context, key string, size int) {
	MemcacheAdd(ctx, key, []byte(strconv.Itoa(size)))
}
