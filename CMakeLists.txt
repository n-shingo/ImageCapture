cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(ImageCapture)

# set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# set the build type
set(CMAKE_BUILD_TYPE Release)
ADD_DEFINITIONS("-std=c++0x") #for c++11

# confirmation messages
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_C_FLAGS_RELEASE: ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")

find_package(PkgConfig)
pkg_check_modules(OPENCV REQUIRED opencv)

add_executable(ImageCapture ./src/main.cpp)
target_link_libraries(ImageCapture ${OPENCV_LIBRARIES})

