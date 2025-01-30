cmake_minimum_required(VERSION 3.5)

project(PP LANGUAGES C)

add_executable(PP main.c)

install(TARGETS PP
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
