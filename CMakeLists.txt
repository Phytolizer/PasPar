cmake_minimum_required(VERSION 3.12...3.23)

project(
  PasPar
  LANGUAGES C
  VERSION 0.1.0
  DESCRIPTION "Parser for Pascal"
  HOMEPAGE_URL "https://github.com/Phytolizer/PasPar.git"
)

add_library(vec vec/src/vec.c)
target_include_directories(vec PUBLIC vec/inc)

add_library(uthash INTERFACE)
target_include_directories(uthash INTERFACE uthash/inc)

add_library(pas pas/src/lex.c pas/src/string.c)
target_include_directories(pas PUBLIC pas/inc)
target_link_libraries(pas PUBLIC vec uthash)

add_executable(paspar paspar/src/main.c)
target_link_libraries(paspar PUBLIC pas)
