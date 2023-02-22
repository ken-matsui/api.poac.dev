# api.poac.dev

```shell
$ RUST_LOG=debug cargo run
```

## Create a new migration

```shell
diesel migration generate create_posts
```

## Apply migrations

```shell
diesel migration run
```

## Build Dockerfile

```shell
docker build --build-arg ROOT_CRT='' .
```

## Useful Links

* https://docs.diesel.rs/diesel/dsl/index.html#functions
* https://docs.diesel.rs/diesel/query_dsl/trait.QueryDsl.html
