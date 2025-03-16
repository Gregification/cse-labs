cmake_minimum_required(VERSION 3.20) #3.22.1 tested

project(Assignemnt3 
    LANGUAGES CXX
    VERSION 0.0.0
    DESCRIPTION "cse3320, assignment 3. fall2024. George Boone 1002055713"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRCS
    src/*.cpp
)

add_executable(${PROJECT_NAME} ${SRCS})
