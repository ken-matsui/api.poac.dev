# api.poac.pm

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
DOCKER_BUILDKIT=1 docker build --secret id=root.crt,src=$HOME/.postgresql/root.crt .
```
