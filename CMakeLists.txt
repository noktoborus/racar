# vim: ft=cmake:et:ts=4:ff=unix:fenc=utf-8:
cmake_minimum_required(VERSION 3.0.2)

project(_init_ C)

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -pedantic")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

# *** library
project(model C)

add_library(model SHARED
    src/tlog.c
    src/mempool.c
    src/model.c
    src/model_module.c
    src/model_loader.c
    src/model_session.c
    src/evsock.c
    )

# *** executable
project(racar C)

add_executable(racar
    src/main.c
    )

target_link_libraries(racar model dl ev)

# *** modules
option(OPT_MODULE_RACAR "Build racar module" ON)
if (OPT_MODULE_RACAR)
    project(module-racar C)

    add_library(module-racar MODULE
        src/modules/racar.c
        src/base.c
        )
endif ()

option(OPT_MODULE_SIMPLE "Build simple module" ON)
if (OPT_MODULE_SIMPLE)
    project(module-simple C)

    add_library(module-simple MODULE
        src/modules/simple.c
        )
endif ()

