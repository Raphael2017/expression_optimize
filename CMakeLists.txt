cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -pedantic ${CMAKE_CXX_FLAGS}")
project(sql92)

set(CMAKE_CXX_STANDARD 11)
include_directories(libsql92/include)

add_executable(sql92 main.cpp
        libsql92/src/lex.cpp
        libsql92/src/search_condition.cpp
        libsql92/src/predicate.cpp
        libsql92/src/row_value_constructor.cpp
        libsql92/src/parse.cpp
        libsql92/src/hash_map_double_key.cpp
        libsql92/src/dynamic_programming_contained.cpp)


#add_subdirectory(libsql92)
