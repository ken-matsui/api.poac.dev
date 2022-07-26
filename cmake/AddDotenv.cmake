include_guard(GLOBAL)

message(CHECK_START "Adding dotenv")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME dotenv
    VERSION 0.1.0
    GITHUB_REPOSITORY laserpants/dotenv-cpp
    GIT_TAG e6615a18edc3b72d50b1dae8b867450d3fda11d6
    OPTIONS
        "BUILD_DOCS OFF"
)

list(APPEND POAC_API_DEPENDENCIES dotenv)
message(CHECK_PASS "added")

list(POP_BACK CMAKE_MESSAGE_INDENT)
