defmodule PoacpmWeb.Router do
  use PoacpmWeb, :router

  pipeline :api do
    plug(:accepts, ["json"])
  end

  scope "/api", PoacpmWeb.Api do
    pipe_through(:api)
    post("/packages/upload", PackagesController, :upload)
  end
end
