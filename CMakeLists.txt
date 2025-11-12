cmake_minimum_required(VERSION 3.16)
project(kurai-webserver C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set common warning flags
set(COMMON_WARN_FLAGS
    -Wall -Wextra -Wpedantic
    -Wformat=2 -Wno-unused-parameter -Wshadow
    -Wwrite-strings -Wstrict-prototypes -Wold-style-definition
    -Wredundant-decls -Wnested-externs -Wmissing-include-dirs
)

# Additional GCC-specific warnings
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    list(APPEND COMMON_WARN_FLAGS -Wjump-misses-init -Wlogical-op)
endif()

add_compile_options(${COMMON_WARN_FLAGS})

add_subdirectory(core)
# add_subdirectory(routes)
