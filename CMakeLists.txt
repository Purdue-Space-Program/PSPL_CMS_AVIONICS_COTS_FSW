cmake_minimum_required(VERSION 3.22.1)
project(cms-fsw)

set(CMAKE_EXPORT_COMPILE_COMMANDS True)
set(CMAKE_INSTALL_PREFIX /fsw/install)

# Add executable
add_executable(fsw ./src/main.cc)
include_directories(fsw ./include)

include_directories(/usr/local/include)

target_link_libraries(fsw /usr/local/lib/libdaqhats.so)

add_compile_options(fsw
    -O3
)

install(TARGETS fsw DESTINATION bin)
