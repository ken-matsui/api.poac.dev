include_guard(GLOBAL)

message(CHECK_START "Adding ${PROJECT_NAME} dependencies")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
unset(missingDependencies)

include(cmake/AddDrogon.cmake)
include(cmake/AddDotenv.cmake)

list(POP_BACK CMAKE_MESSAGE_INDENT)
if (missingDependencies)
    message(CHECK_FAIL "missing dependencies: ${missingDependencies}")
    message(FATAL_ERROR "missing dependencies found")
else ()
    message(CHECK_PASS "all dependencies are added")
endif ()

message(STATUS "dependencies are ... ${POAC_API_DEPENDENCIES}")
