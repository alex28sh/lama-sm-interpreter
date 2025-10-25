cmake_minimum_required(VERSION 3.30)
project(lama_sm_interpreter)

set(CMAKE_CXX_STANDARD 20)

#add_definitions(-DLAMA_ENV)
include(FetchContent)
FetchContent_Declare(
        magic_enum
        GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
        GIT_TAG        v0.9.7        # Use latest released version
)

FetchContent_MakeAvailable(magic_enum)

include(FetchContent)
FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 10.1.1
)
FetchContent_MakeAvailable(fmt)

add_executable(lama_sm_interpreter src/main.cpp runtime/custom_data.c bytefile/bytefile.cpp src/printers.hpp)

target_link_libraries(lama_sm_interpreter ${CMAKE_SOURCE_DIR}/runtime/runtime.a magic_enum::magic_enum fmt::fmt)
target_include_directories(lama_sm_interpreter PRIVATE
        ${CMAKE_SOURCE_DIR}/src/decoders
        ${CMAKE_SOURCE_DIR}/src/single_interpreters
        ${CMAKE_SOURCE_DIR}/src/state
        ${CMAKE_SOURCE_DIR}/bytefile
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")

target_compile_options(lama_sm_interpreter PRIVATE
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
        -fstack-protector-all
        -Ofast
        -fno-inline -fno-inline-functions -fno-inline-small-functions
        #        -Wno-pointer-to-int-cast
#        -Wno-writable-strings
)
