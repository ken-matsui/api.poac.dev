// std
#include <cstdlib>
#include <string>

// external
#include <dotenv.h> // NOLINT(build/include_order)
#include <drogon/drogon.h> // NOLINT(build/include_order)

std::string
getDbHost() {
  if (const char* env_p = std::getenv("POAC_API_DB_HOST")) {
    LOG_DEBUG << "POAC_API_DB_HOST: " << env_p;
    return {env_p};
  }
  LOG_ERROR << "POAC_API_DB_HOST should be exported";
  std::exit(EXIT_FAILURE);
}

std::uint16_t
getDbPort() {
  if (const char* env_p = std::getenv("POAC_API_DB_PORT")) {
    LOG_DEBUG << "POAC_API_DB_PORT: " << env_p;
    return std::stoi(env_p);
  }
  LOG_ERROR << "POAC_API_DB_PORT should be exported";
  std::exit(EXIT_FAILURE);
}

std::string
getDbDatabaseName() {
  if (const char* env_p = std::getenv("POAC_API_DB_DATABASE_NAME")) {
    LOG_DEBUG << "POAC_API_DB_DATABASE_NAME: " << env_p;
    return {env_p};
  }
  LOG_ERROR << "POAC_API_DB_DATABASE_NAME should be exported";
  std::exit(EXIT_FAILURE);
}

std::string
getDbUserName() {
  if (const char* env_p = std::getenv("POAC_API_DB_USER_NAME")) {
    LOG_DEBUG << "POAC_API_DB_USER_NAME: " << env_p;
    return {env_p};
  }
  LOG_ERROR << "POAC_API_DB_USER_NAME should be exported";
  std::exit(EXIT_FAILURE);
}

std::string
getDbPassword() {
  return dotenv::getenv("POAC_API_DB_PASSWORD", "");
}

int
main() {
  dotenv::init();

  // Set HTTP listener address and port
  const std::uint16_t port = std::stoi(dotenv::getenv("PORT", "8000"));
  drogon::app().addListener("0.0.0.0", port);

  // Connect to database
  drogon::app().createDbClient(
      "postgresql", getDbHost(), getDbPort(), getDbDatabaseName(),
      getDbUserName(), getDbPassword()
  );

  // Load config file
  // drogon::app().loadConfigFile("../config.json");

  // Run HTTP framework,the method will block in the internal event loop
  drogon::app().run();
  return 0;
}
