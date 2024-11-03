cmake_minimum_required(VERSION 3.5.0)
project(libart VERSION 0.1.0 LANGUAGES C)

set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

file(GLOB SOURCES
     "${SRC_DIR}/*.h"
     "${SRC_DIR}/*.c"
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${SRC_DIR})

# include(CTest)
# enable_testing()
