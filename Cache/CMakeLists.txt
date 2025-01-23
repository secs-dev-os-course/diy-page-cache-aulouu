cmake_minimum_required(VERSION 3.12)
project(app
        VERSION 0.0.0
        DESCRIPTION "Dynamic-link library module"
        LANGUAGES CXX
)

cmake_policy(SET CMP0076 NEW) # avoid warning of relative paths translation

add_library(Cache SHARED ""
        CacheAPI.cpp
        CacheAPI.h
        CacheManager.cpp
        CacheManager.h
)

target_include_directories(Cache PRIVATE ${CMAKE_SOURCE_DIR})

target_sources(Cache PRIVATE
        CacheAPI.cpp
        CacheAPI.h
        CacheManager.cpp
        CacheManager.h
)

target_compile_definitions(Cache PRIVATE
        CACHE_EXPORTS)

set(COPY_TO_APP OFF)
if (COPY_TO_APP)
    add_custom_command(TARGET Cache POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Cache>"
            "${CMAKE_BINARY_DIR}/app")
endif ()

#if (COPY_TO_APP)
#    add_custom_command(TARGET CacheManager POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            "${PROJECT_BINARY_DIR}/libCache.dll"
#            "${CMAKE_BINARY_DIR}/app")
#endif ()
