#include <iostream>
// #include "osmpbfreader.h"
#include <osmium/io/pbf_input.hpp>



int main() {
    osmium::io::File input_file{"../files/antarctica-latest.osm.pbf"};
    osmium::io::Reader reader{input_file};
    osmium::io::Header header = reader.header();
    std::cout << "Hello World" << std::endl;
    return 0;
}
