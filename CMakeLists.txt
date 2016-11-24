# vim: ft=cmake:et:ts=4:ff=unix:fenc=utf-8:
cmake_minimum_required(VERSION 3.0.2)

project(racar C)

set(CMAKE_C_FLAGS "-std=c99 -Wall")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(racar src/main.c src/tlog.c src/base.c src/mempool.c src/model.c src/model_module.c src/model_loader.c)
target_link_libraries(racar dl)


