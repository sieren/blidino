cmake_minimum_required(VERSION 2.8)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)

# Enable ExternalProject CMake module
include(ExternalProject)

# Download and install GoogleTest
ExternalProject_Add(
    gtest
    GIT_REPOSITORY https://github.com/google/googletest.git
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/googletest
    # Disable install step
    INSTALL_COMMAND ""
)

# Create a libgtest target to be used as a dependency by test programs
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)

# Set gtest properties
ExternalProject_Get_Property(gtest source_dir binary_dir)
set_target_properties(libgtest PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/libgtest.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
#    "INTERFACE_INCLUDE_DIRECTORIES" "${source_dir}/include"
)
# I couldn't make it work with INTERFACE_INCLUDE_DIRECTORIES
#include_directories("${source_dir}/include")
include_directories(SYSTEM
  ${CMAKE_CURRENT_BINARY_DIR}/googletest/src/gtest/googletest/include)
include_directories(SYSTEM
  ${CMAKE_CURRENT_BINARY_DIR}/googletest/src/gtest/googlemock/include)
# Set gmock properties
# I couldn't make it work with INTERFACE_INCLUDE_DIRECTORIES
include_directories("${source_dir}/include")
add_library(libgmock IMPORTED STATIC GLOBAL)

file(GLOB SRCS *.cpp *.h)
add_executable(bletest ${SRCS} unit_tests/tst_BLEParser.cpp)
set_property(TARGET bletest PROPERTY CXX_STANDARD 11)

set_target_properties(libgmock PROPERTIES IMPORTED_LOCATION
  ${CMAKE_CURRENT_BINARY_DIR}/googletest/src/gtest-build/googlemock/libgmock.a)
set_target_properties(libgtest PROPERTIES IMPORTED_LOCATION
  ${CMAKE_CURRENT_BINARY_DIR}/googletest/src/gtest-build/googlemock/gtest/libgtest.a)

target_link_libraries(bletest
    libgtest
    libgmock
)
IF(CMAKE_SYSTEM_NAME STREQUAL Linux)
  set_target_properties(bletest PROPERTIES COMPILE_FLAGS -pthread LINK_FLAGS -pthread)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL Linux)
install(TARGETS bletest DESTINATION bin)
