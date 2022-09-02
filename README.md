# API

API implementation with Supabase.

This repository is just for keeping declarations of DB information, such as
tables, DB functions, and RLS policies. That means this repository is not
completely synced with Supabase instances.

## Create a new migration

```bash
api/$ supabase migration new $table
```

## Create a new function

```bash
api/$ supabase functions new $name
```

## Deploy a function

```bash
api/$ supabase functions deploy search --project-ref <ref>
```

### Format

```bash
$ deno fmt
=======
# api.poac.pm

```shell
$ RUST_LOG=debug cargo run
```
