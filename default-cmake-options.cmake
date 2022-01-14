set(cmsis_DIR "${PROJECT_SOURCE_DIR}/../cmsis-posix/public")
message(STATUS "cmsis_DIR=${cmsis_DIR}")
find_package(cmsis REQUIRED)
add_compile_definitions(OS_POSIX)

set(CMAKE_C_FLAGS "-std=gnu99")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wunknown-pragmas")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wtrigraphs")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wimplicit-int")

set(BUILD_TYPE "debug")
if (debug)
    set(BUILD_TYPE "debug")
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g")
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -O0")
else()
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -O2")
endif()

message(STATUS "BUILD_TYPE " ${BUILD_TYPE})

set(LWIP_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(LWIP_CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lwip")

