cmake_minimum_required(VERSION 3.22)

project(small_sql
    VERSION 1.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options("-Wall")
#add_compile_options("-Werror")
add_compile_options("-Wextra")
add_compile_options("-DDEBUG")

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE REPO_SRC src/*.cpp)

#add_library(small_sql SHARED ${REPO_SRC})
add_library(small_sql STATIC ${REPO_SRC})
target_include_directories(small_sql PRIVATE REPO_SRC)

target_include_directories(small_sql
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

enable_testing()

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

file(GLOB TEST_SRC tests/*.cpp)

add_executable(small_sql_tests ${TEST_SRC})

target_link_libraries(small_sql_tests
    GTest::GTest
    GTest::Main
    small_sql
)

include(GoogleTest)
gtest_discover_tests(small_sql_tests)
