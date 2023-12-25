> [!NOTE]
> This API server has been shut down since there are currently no reliable free hosting services.  The same functionalities are provided in another way.

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

## Run the server

```shell
cargo install cargo-shuttle
cargo shuttle run
```

## Useful Links

* https://docs.diesel.rs/diesel/dsl/index.html#functions
* https://docs.diesel.rs/diesel/query_dsl/trait.QueryDsl.html
