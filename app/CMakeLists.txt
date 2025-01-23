cmake_minimum_required(VERSION 3.12)
project(app
        VERSION 0.0.0
        DESCRIPTION "Application module"
        LANGUAGES C CXX
)

cmake_policy(SET CMP0076 NEW) # avoid warning of relative paths translation

add_executable(app)

target_include_directories(app PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(app PRIVATE Cache)

#target_include_directories(app PRIVATE ${CMAKE_SOURCE_DIR})
#target_link_libraries(app "${CMAKE_BINARY_DIR}/Cache/libCache.dll")

target_sources(app
        PRIVATE
        app.cpp
)

SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.dll.a;.dll")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")
