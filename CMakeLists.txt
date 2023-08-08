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

set(GRAPH src/graph/Graph.h src/graph/Graph.cpp src/graph/GraphGenerator.h src/graph/GraphGenerator.cpp src/graph/InPolyTest.h src/graph/InPolyTest.cpp)
set(UTILS src/utils/Utils.h src/utils/Utils.cpp src/utils/GraphUtils.h src/utils/GraphUtils.cpp)
set(IO  src/io/GeoWriter.h src/io/GeoWriter.cpp src/io/PBFProcessing.h src/io/PBFProcessing.cpp)
set(TRANSIT_NODES src/transit_nodes/TransitNodesRouting.h src/transit_nodes/TransitNodesRouting.cpp)

set(SOURCES src/main.cpp libs/server_http.hpp)
add_executable(osm ${SOURCES} ${GRAPH} ${UTILS} ${IO} ${TRANSIT_NODES})
target_include_directories(osm PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(osm -lz -pthread)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)