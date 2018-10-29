# poac.pm-api-upload
## Execute on local

#### Install dependencies
```bash
$ mix deps.get
```

#### Export required variables
```bash
$ export GOOGLE_APPLICATION_CREDENTIALS=/path/to/credentials.json
```

#### Start Phoenix endpoint
```bash
$ mix phx.server
```

Now you can visit [`localhost:4000`](http://localhost:4000) from your browser.


## Deploy
```bash
$ gcloud app deploy
```

### Reference
https://cloud.google.com/community/tutorials/elixir-phoenix-on-google-app-engine?hl=ja
