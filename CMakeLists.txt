cmake_minimum_required(VERSION 3.10)
project(kaleiscope)
set(CMAKE_CXX_STANDARD 14)

if (APPLE)
  set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)
endif()

set(CMAKE_C_FLAGS "-Werror -Wall -g")


# LLVM, equivalent to 'clang++ `llvm-config --cxxflags`'
# https://stackoverflow.com/questions/52931852/how-to-convert-llvm-clang-command-line-to-cmake-config
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM_FOUND ${LLVM_FOUND}")
message(STATUS "LLVM_DIR ${LLVM_DIR}")
message(STATUS "LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM_DEFINITIONS: ${LLVM_DEFINITIONS}")
message(STATUS "llvm_libs: ${llvm_libs}")

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

llvm_map_components_to_libnames(
        llvm_libs
        Analysis
        Core
        ExecutionEngine
        InstCombine
        Object
        OrcJIT
        RuntimeDyld
        ScalarOpts
        Support
        TransformUtils
        native
        irreader
        )

if (NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

# include_directories("src/" "mpc/" "/usr/local/include")

add_executable(main src/main.cpp)
target_link_libraries(main ${llvm_libs})
