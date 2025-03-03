cmake_minimum_required(VERSION 3.22.1)
project(cms-fsw)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

# set compiler to gcc and g++
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

option(BUILD_SITL "Build the Software-In-The-Loop version" OFF)

if (BUILD_SITL)
    add_executable(fsw_sitl
        src/bang_bang_controller.cpp
        src/command_handler.cpp
        src/daq.cpp
        src/main.cpp
        src/queue.cpp
        src/data_writer.cpp
        sitl_harness/harness.cpp
    )

    target_include_directories(fsw_sitl PRIVATE include)

    add_compile_options(fsw_sitl
        -g
        -fsanitize=address,undefined,thread,integer,leak,alignment
    )
else()
    add_executable(fsw 
        src/ads1263.c
        src/bang_bang_controller.cpp
        src/gpio.cpp
        src/command_handler.cpp
        src/daq.cpp
        src/hwif.cpp
        src/main.cpp
        src/queue.cpp
        src/data_writer.cpp
    )

    target_include_directories(fsw PRIVATE include)
    target_link_libraries(fsw gpiod)

    add_compile_options(fsw
        -g
        -fsanitize=address,undefined,thread,integer,leak,alignment
    )

    install(TARGETS fsw DESTINATION bin)
    install(FILES tools/fsw_telemetry_server 
            DESTINATION bin
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_EXECUTE
                        WORLD_READ WORLD_EXECUTE)
endif()