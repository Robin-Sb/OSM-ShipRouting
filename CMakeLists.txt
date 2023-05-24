cmake_minimum_required(VERSION 3.0.0)
project(osm VERSION 0.1.0)

include(CTest)
enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/libs")

find_package(Protozero 1.6.3 REQUIRED)
include_directories(SYSTEM ${PROTOZERO_INCLUDE_DIR})

find_package(Osmium 2.19.0 REQUIRED COMPONENTS pbf)
find_package(Osmium REQUIRED)

if(NOT OSMIUM_FOUND)
    message(WARNING "Libosmium not found!\n")
endif()

include_directories(${OSMIUM_INCLUDE_DIRS})

set(SOURCES src/main.cpp src/PBFReader.h src/PBFReader.cpp src/Graph.h src/Graph.cpp src/Utils.h src/Utils.cpp src/InPolyTest.h src/InPolyTest.cpp src/GeoWriter.h src/GeoWriter.cpp src/GraphUtils.h src/GraphUtils.cpp)
add_executable(osm ${SOURCES})
target_include_directories(osm PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(osm -lz -pthread)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)