cmake_minimum_required(VERSION 3.25)

project(cms_fsw CXX C)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

add_executable(
    fsw
)

target_sources(fsw PRIVATE
    src/bang_bang_controller.cpp
    src/command_handler.cpp
    src/data_writer.cpp
    src/daq.cpp
    src/gpio.cpp
    src/hwif.cpp
    src/main.cpp
    src/queue.cpp
    src/state_writer.cpp
    src/stat.cpp
    src/telem_server.cpp
    
    src/ads1263.c
)

target_include_directories(fsw PRIVATE
    include
)

target_link_libraries(fsw
    gpiod
)

target_compile_options(fsw PRIVATE
    "-Wall"
    "-Wno-unused-function"
    "-Wno-unknown-pragmas"
    "-Wno-unused-const-variable"
    "-O2"
    "-flto"
)

set(CMAKE_CXX_FLAGS
    ${CMAKE_CXX_FLAGS}
    -std=c++20
)
set(CMAKE_C_FLAGS
    ${CMAKE_C_FLAGS}
    -std=c11
)
set(LDFLAGS
    ${LDFLAGS}
    -flto
)
