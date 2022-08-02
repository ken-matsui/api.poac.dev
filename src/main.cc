// std
#include <cstdlib>
#include <string>

// external
#include <dotenv.h> // NOLINT(build/include_order)
#include <drogon/drogon.h> // NOLINT(build/include_order)

struct DbInfo {
  std::string dbType = "postgresql";
  std::string userName;
  std::string password;
  std::string host;
  std::uint16_t port = 5432;
  std::string databaseName;
};

inline std::string
getEnv(const std::string& name) {
  if (const char* env_p = std::getenv(name.c_str())) {
    LOG_DEBUG << name << ": " << env_p;
    return {env_p};
  }
  LOG_ERROR << name << " should be exported";
  std::exit(EXIT_FAILURE);
}

DbInfo
getDbInfo() {
  DbInfo dbInfo;

  const std::string url = getEnv("DATABASE_URL");
  std::size_t prev_pos = url.find("://");
  prev_pos += 3; // `://`

  std::size_t cur_pos = url.find(':', prev_pos);
  dbInfo.userName = url.substr(prev_pos, cur_pos - prev_pos);
  LOG_DEBUG << "userName: " << dbInfo.userName;
  prev_pos = cur_pos + 1; // `:`

  cur_pos = url.find('@', prev_pos);
  dbInfo.password = url.substr(prev_pos, cur_pos - prev_pos);
  LOG_DEBUG << "password: " << dbInfo.password;
  prev_pos = cur_pos + 1; // `@`

  cur_pos = url.find(':', prev_pos);
  dbInfo.host = url.substr(prev_pos, cur_pos - prev_pos);
  LOG_DEBUG << "host: " << dbInfo.host;
  prev_pos = cur_pos + 1; // `:`

  cur_pos = url.find('/', prev_pos);
  dbInfo.port = std::stoi(url.substr(prev_pos, cur_pos - prev_pos));
  LOG_DEBUG << "port: " << dbInfo.port;
  prev_pos = cur_pos + 1; // `/`

  dbInfo.databaseName = url.substr(prev_pos);
  LOG_DEBUG << "databaseName: " << dbInfo.databaseName;
  return dbInfo;
}

int
main() {
  dotenv::init();

  // Set HTTP listener address and port
  const std::uint16_t port = std::stoi(dotenv::getenv("PORT", "8000"));
  drogon::app()
      .addListener("0.0.0.0", port)
      .setLogLevel(trantor::Logger::LogLevel::kTrace);

  // Connect to database
  const DbInfo dbInfo = getDbInfo();
  drogon::app().createDbClient(
      dbInfo.dbType,
      dbInfo.host,
      dbInfo.port,
      dbInfo.databaseName,
      dbInfo.userName,
      dbInfo.password
  );

  // Load config file
  // drogon::app().loadConfigFile("../config.json");

  // Run HTTP framework,the method will block in the internal event loop
  drogon::app().run();
  return 0;
}
