include_guard(GLOBAL)

message(CHECK_START "Adding dotenv")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME dotenv
    VERSION 0.1.0
    GITHUB_REPOSITORY laserpants/dotenv-cpp
    GIT_TAG 4a64a2a8f8a8df2435e42b971a5c1de0aa084924
)

list(APPEND POAC_API_DEPENDENCIES dotenv)
message(CHECK_PASS "added")

list(POP_BACK CMAKE_MESSAGE_INDENT)
