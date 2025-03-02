cmake_minimum_required(VERSION 3.22.1)
project(cms-fsw)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

add_executable(fsw 
    src/ads1263.c
    src/bang_bang_controller.cpp
    src/command_handler.cpp
    src/daq.cpp
    src/hwif.cpp
    src/main.cpp
    src/queue.cpp
    src/telemetry_writer.cpp
)

target_include_directories(fsw PRIVATE include)
target_link_libraries(fsw gpiod)

add_compile_options(fsw
    -g
    -fsanitize=address,undefined,thread,integer,leak,alignment
)

install(TARGETS fsw DESTINATION bin)
