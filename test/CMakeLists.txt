cmake_minimum_required(VERSION 3.21)
project(poac_api_test CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} test_main.cc)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# ##############################################################################
# If you include the drogon source code locally in your project, use this method
# to add drogon 
# target_link_libraries(${PROJECT_NAME}_test PRIVATE drogon)
#
# and comment out the following lines
target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)

ParseAndAddDrogonTests(${PROJECT_NAME})
