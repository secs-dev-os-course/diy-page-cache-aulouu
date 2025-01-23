cmake_minimum_required(VERSION 3.12)

project(PageCache
        VERSION 0.0.0
        DESCRIPTION "OS lab №2 | Windows LFU"
        LANGUAGES C CXX
)

#set(CMAKE_C_STANDARD 23)
#set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(app)
add_subdirectory(${CMAKE_SOURCE_DIR}/Cache)
