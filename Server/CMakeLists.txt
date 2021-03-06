cmake_minimum_required(VERSION 3.10)

# Find boost
find_package(Boost 1.53.0 COMPONENTS system)

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

# Set the project name
project(Report_Server)

# Set include directory
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${Boost_INCLUDE_DIRS}")

# Set source files
set(SOURCES
    Main.cpp
    src/ConnectionHandler.cpp
    src/Server.cpp
)

add_executable(main ${SOURCES})
target_link_libraries(main ${Boost_LIBRARIES})