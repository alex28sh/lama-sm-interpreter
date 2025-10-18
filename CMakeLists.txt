cmake_minimum_required(VERSION 4.0)
project(lama_sm_interpreter)

set(CMAKE_CXX_STANDARD 20)

add_executable(lama_sm_interpreter src/main.cpp runtime/custom_data.c bytefile/bytefile.c)

target_link_libraries(lama_sm_interpreter ${CMAKE_SOURCE_DIR}/runtime/runtime.a)
target_include_directories(lama_sm_interpreter PRIVATE
        ${CMAKE_SOURCE_DIR}/src/decoders
        ${CMAKE_SOURCE_DIR}/src/single_interpreters
        ${CMAKE_SOURCE_DIR}/src/state
        ${CMAKE_SOURCE_DIR}/bytefile
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")

#target_compile_options(lama_sm_interpreter PRIVATE
##        -Wno-pointer-to-int-cast
#        -Wno-writable-strings
#)
