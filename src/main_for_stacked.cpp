//
//  main.cpp
//  Grey_tif
//
//  Created by Jan Pieter Abrahams on 23.05.23.
//

#include <iostream>
#include <vector>
#include "stack_tiff.hpp"

int main(int argc, const char * argv[]) {
    using namespace jpa;
    Grey_tif tif; // Construct a tif object.
   
    std::filesystem::path input = "";
    std::filesystem::path output = "";
    std::ifstream istream(input, std::ios::binary);
    istream >> tif; // read in data from dis into the tif object
    
    std::vector<std::uint16_t> img0({42,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});
    tif.push_back(img0, {4,4}); // apend a (very small) image, creating a tiff stack.
    
    std::vector<std::uint32_t> img1({4242,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});
    tif.push_back(img1, {4,4}); // you can also append 4-byte image data.
    
    // The images of the stacks can be accessed using an STL vector, that is a member of the Grey_tif object.
    std::cout << "The tif stack now contains "<< tif.stack.size() << " images."<< std::endl;
    
    // All the STL functions and C++ features that work on STL containers also are valid for images in a tif object:
    for (auto val : tif.stack[1])
        std::cout << val << " ";
    std::cout << std::endl;
    for (int i=0; i != tif.stack[2].size(); ++i)
        std::cout << tif.stack[2][i] << " ";
    std::cout << std::endl;

    // Besides the STL iterators like begin() and end(), images in a tif object have two additional member functions determine its dimensions:
    std::cout << "The dimensions of the 2nd image are: " << tif.stack[1].dim()[0] << " by "<< tif.stack[1].dim()[0] << " pixels." << std::endl;
    std::cout << "The pixels of the 3rd image have " << tif.stack[2].sizeof_pixel() << " bytes." << std::endl;

    std::vector<std::uint16_t> img3(512*512);
    std::vector<std::uint16_t> img4(512*512);

    // The first image (or only) of a stack can be extracted without having to specify its stack number.
    std::copy(tif.begin(), tif.end(), img3.begin());
    std::copy(tif.stack[0].begin(), tif.stack[0].end(), img4.begin());
    assert (img3 == img4);
    
    // A tif object can be written to disk using the overloaded <<() operator.
    std::ofstream ostream(output, std::ios::binary);
    ostream << tif;
    return 0;
}
