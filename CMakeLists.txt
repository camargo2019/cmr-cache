cmake_minimum_required(VERSION 3.10)

project(CmrCache)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR}/vendor)

find_package(Boost REQUIRED COMPONENTS system thread)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()

set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/)

file(GLOB SRCS ${SOURCE_DIR}/*.cpp)

add_executable(cmr_cache ${SRCS})

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(cmr_cache ${Boost_LIBRARIES})

if (UNIX AND NOT WIN32)
    target_link_libraries(cmr_cache pthread)
endif()
