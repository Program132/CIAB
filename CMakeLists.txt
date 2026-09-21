cmake_minimum_required(VERSION 3.16)

project(CIAB C)

set(CMAKE_C_STANDARD 23)

include_directories(src)

enable_testing()

add_executable(test_strlist
    tests/test_strlist.c
    src/strlist/strlist.c
    src/strlist/strlist.h
)

add_test(NAME test_strlist COMMAND test_strlist)

add_executable(test_hashmapstr
    tests/test_hashmapstr.c
    src/hashmapstr/hashmapstr.c
    src/hashmapstr/hashmapstr.h
)

add_test(NAME test_hashmapstr COMMAND test_hashmapstr)

add_executable(test_trigram
    tests/test_trigram.c
    src/trigram/trigram.c
    src/trigram/trigram.h
)

add_test(NAME test_trigram COMMAND test_trigram)


