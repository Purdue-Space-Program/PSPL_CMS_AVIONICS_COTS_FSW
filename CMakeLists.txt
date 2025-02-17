cmake_minimum_required(VERSION 3.22.1)
project(cms-fsw)

set(CMAKE_EXPORT_COMPILE_COMMANDS True)
set(CMAKE_INSTALL_PREFIX /fsw/install)

add_executable(fsw 
    src/main.cc
    src/bang_bang_controller.cpp
    src/command_handler.cpp
    src/daq.cpp
    src/queue.cpp
)

# for some reason pkg_check_modules cannot find libdaqhats, no idea why not
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)

target_include_directories(fsw PRIVATE ${GPIOD_INCLUDE_DIRS} /usr/local/include)
target_link_libraries(fsw ${GPIOD_LIBRARIES} /usr/local/lib/libdaqhats.so)
include_directories(fsw include)

add_compile_options(fsw
    -O3
)
install(TARGETS fsw DESTINATION bin)
