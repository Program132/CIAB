cmake_minimum_required(VERSION 3.16)

project(CIAB C)

set(CMAKE_C_STANDARD 23)

add_executable(creator 
    src/creator.c 

    src/ReadFile/ReadFile.c
    src/ReadFile/ReadFile.h
    src/SQlite/SQLite.c
    src/SQlite/SQLite.h   
    )

add_executable(generator 
    src/generator.c 

    src/ReadFile/ReadFile.c
    src/ReadFile/ReadFile.h
    src/SQlite/SQLite.c
    src/SQlite/SQLite.h  
    )