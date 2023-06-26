As of now, the software is only tested on ubuntu 20.04. Installation instructions are also based on Ubuntu. Different operating systems may also work, but might need custom installation steps.

Prerequisites: 
- gcc (tested with 9.4.0)
- CMake (tested with 3.16.3)

## Dependencies of third party libraries
The project uses two third party libraries for reading/writing pbf files and for starting a simple http server:
- https://github.com/osmcode/libosmium.git
- https://github.com/eidheim/Simple-Web-Server

#### Osmium
- Installation: git clone https://github.com/osmcode/libosmium.git 
- no additional build required
- at least version 2.19

This requires the following dependencies: 
- Zlib: zlib1g-dev
- lz4: liblz4-dev
- boost (at least 1.5.5), e.g. sudo apt-get install libboost-all-dev

###### Protozero
My installation: 
- git clone https://github.com/mapbox/protozero.git to somewhere
- cd protozero
- mkdir build
- cd build
- cmake ..
- make
- make install

The FindOsmium.cmake and FindProtozero.cmake should find the include directories of Osmium and Protozero, at least if they are in the parent directory

#### SimpleWebServer
- https://github.com/eidheim/Simple-Web-Server
- Only requires boost, which should already be installed due to Osmium

## Building and running
- build: 
  - cmake --build build
- Run: 
  - cd build
  - ./osm
- The program then starts to generate the graph and writes it to graph.fmi, if the file does not exist.
- Afterwards, a local web server is started, which listens to localhost:8080

## GUI
The entry point of the GUI is UI/index.html. 
The GUI needs a local file server to run; I used the VS Code liveserver extension, but any static fileserver should work (e.g. https://medium.com/swlh/need-a-local-static-server-here-are-several-options-bbbe77e59a11)
To run the GUI, start the liveserver and navigate to localhost:[PORT_OF_LIVESERVER]/UI/index.html

## Using the application
Upon clicking on the Set Start or the Set End button and then clicking on the map, the start and end location can be set.
If valid start and end positions are set, the backend service is called by clicking on Route and the "optimal route" is generated. 
The optimal route is displayed as an orange linestring.