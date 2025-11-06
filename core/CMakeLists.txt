file(GLOB K_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/k_*.c")

if(NOT K_SRCS)
    message(FATAL_ERROR "No sources matching k_*.c found in ${CMAKE_CURRENT_SOURCE_DIR}.\nMake sure your k_*.c files are present or update the glob pattern.")
endif()

message(STATUS "k_core sources: ${K_SRCS}")

add_library(k_core STATIC ${K_SRCS})

target_include_directories(k_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(k_core PUBLIC pthread dl) # dl for dlopen

add_executable(kurai-webserver main.c)
target_link_libraries(kurai-webserver PRIVATE k_core)
