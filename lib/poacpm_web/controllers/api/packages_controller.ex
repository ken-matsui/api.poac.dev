defmodule PoacpmWeb.Api.PackagesController do
  use PoacpmWeb, :controller
  alias Poacpm.Util.Firestore
  alias Poacpm.Util.GCS
  import Phoenix.Controller, only: [
    put_new_layout: 2,
    put_new_view: 2,
    text: 2
  ]


  defp get_token_name(x) do
    # "projects/{}/databases/{}/documents/tokens/ABCDEFG"
    x.name
    |> String.split("/")
    |> List.last() # ABCDEFG
  end

  # boost/config -> boost-config
  def orgToPkg(name) do
    name
    |> String.replace("/", "-")
  end

  @api_url "https://www.googleapis.com/storage/v1/b/poac-pm.appspot.com/o/"
  def getMd5Hash(filename) do
    HTTPoison.start()
    res = HTTPoison.get!(@api_url <> filename)
    case res do
      %{status_code: 200, body: body} ->
        body
        |> Poison.decode!()
        |> Map.get("md5Hash")
    end
  end

  def calc_file_size(path) do
    case File.stat(path) do
      {:ok, %{size: size}} -> size
      {:error, reason} -> reason # handle error
    end
  end

  @doc """
  IF >100MB, invalid
  Note: There is a possibility of difference depending on OS and file system.
  Ret: ture -> valid, false -> invalid
  """
  def check_file_size(size) do
    size < 100000000
  end


  @doc """
  Note: This file is temporary, and Plug will remove it
         from the directory as the request completes.
        If we need to do anything with this file,
         we need to do it before then.
  Ref: https://phoenixframework.org/blog/file-uploads
  """
  def upload(conn, %{"user" => user_params}) do
    file = user_params["data"]
    file_size = file.path
                |> calc_file_size()
    check_ret = file_size
                |> check_file_size()

    if check_ret do
      setting = user_params["setting"]
                |> Poison.decode!()
                |> Map.get("setting")
                |> YamlElixir.read_from_string!()

      token = user_params["token"]
              |> Poison.decode!()
              |> Map.get("token")


      owners = setting["owners"]
      name = setting["name"]
      version = setting["version"]


      exists_api = "https://poac.pm/api/packages/" <> name <> "/" <> version <> "/exists"
      exists = case HTTPoison.get!(exists_api) do
        %{status_code: 200, body: body} ->
          body
          |> Poison.decode!()
        end


      validate_api = "https://poac.pm/api/tokens/validate"
      body = %{"token" => token, "owners" => owners} |> Poison.encode!()
      headers = [{"Content-type", "application/json"}]
      validate = case HTTPoison.post!(validate_api, body, headers, []) do
        %{status_code: 200, body: body} ->
          if body == "ok", do: true, else: false
      end


      # TODO: もし，存在していても，ownerが一致していれば，上書きができる(next version)
      if validate and !exists do
        # Upload to google cloud storage
        GCS.upload_file(file.filename, file.path) # TODO: このfile.filenameを使用してしてしまうと，hackされる恐れが高まる

        # Wait pending... (10s)
        Process.sleep(10000)

        setting
        |> Map.put("object_name", file.filename)
        |> Firestore.create_doc()

        # Wait pending... (5s)
        Process.sleep(5000)

        text(conn, "ok")
      else
        text(conn, "err")
      end

    else
      text(conn, "err")
    end
  end
end
