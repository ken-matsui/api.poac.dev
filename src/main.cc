// std
#include <cstdlib>
#include <string>

// external
#include <dotenv.h> // NOLINT(build/include_order)
#include <drogon/drogon.h> // NOLINT(build/include_order)

inline std::string
getEnv(const std::string& name) {
  if (const char* env_p = std::getenv(name.c_str())) {
    LOG_DEBUG << name << ": " << env_p;
    return {env_p};
  }
  LOG_ERROR << name << " should be exported";
  std::exit(EXIT_FAILURE);
}

inline std::string
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
      "postgresql", getEnv("POAC_API_DB_HOST"),
      std::stoi(getEnv("POAC_API_DB_PORT")), getEnv("POAC_API_DB_DATABASE"),
      getEnv("POAC_API_DB_USER"), getDbPassword()
  );

  // Load config file
  // drogon::app().loadConfigFile("../config.json");

  // Run HTTP framework,the method will block in the internal event loop
  drogon::app().run();
  return 0;
}
