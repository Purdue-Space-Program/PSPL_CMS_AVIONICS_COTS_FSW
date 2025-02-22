cmake_minimum_required(VERSION 3.22.1)
project(cms-fsw)

set(CMAKE_EXPORT_COMPILE_COMMANDS True)

add_executable(fsw 
    src/main.cc
    src/bang_bang_controller.cpp
    src/command_handler.cpp
    src/telemetry_writer.cpp
    src/daq.cpp
    src/queue.cpp
)

target_link_libraries(fsw gpiod daqhats)
include_directories(fsw include)

add_compile_options(fsw
    -O3
)

install(TARGETS fsw DESTINATION bin)
