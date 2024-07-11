//
//  main.cpp
//  Created by Senik Matinyan
//
//  Revised by Jan Pieter Abrahams on 19.03.23.
//

#include <iostream>
#include <chrono>
#include <cmath>
#include <filesystem>
#include "Command_line.hpp"
#include "Terse.hpp"
#include "Grey_tif.hpp"

namespace fs = std::filesystem;

int main(int argc, char const* argv[]) {
    using namespace jpa;
    Command_line_option help("-help", "print help");
    Command_line_option verbose("-verbose", "print expanded file names and compute times");
    Command_line input(argc, argv, {help, verbose});
    if (input.option("-help").found()) {
        std::cout << "prolix [-help] [-verbose] [file ...]\n";
        std::cout << "  expands trpx files to tiff files.\n";
        std::cout << "Examples:\n";
        std::cout << "   prolix *              // all TRPX files with .trpx extensions are expanded to tiff files with .tif extensions.\n";
        std::cout << "   prolix ˜/dir/my_img*  // decompresses all trpx files in the directory ~/dir that start with my_img\n";
        std::cout << "\nkeywords:\n";
        std::cout << input.help() << std::endl;
        return 0;
    }
    
    // Some timers and counters are required for the 'verbose' option.
    std::chrono::duration<double> user_time;
    std::chrono::duration<double> IO_time;
    std::size_t expanded_files = 0;
    
    // Loop over all input file names
    for (fs::path filename : input.params()) {
        
        // Only trpx files will be compressed
        if (fs::is_regular_file(filename) && filename.extension() ==".trpx") {
            
            // Start the IO timer and open the next file
            auto start_IO_time = std::chrono::high_resolution_clock::now();
            std::ifstream trpx_file(filename, std::ios::binary);
            if (!trpx_file.is_open())
                std::cerr << "Failed to open input file " << filename << std::endl;
            else {
                
                // A trpx file was opened. read its data. It may contain one ore more images in a stack.
                Terse trpx_data(trpx_file);
                trpx_file.close();
                
                // stop the IO timer, start the user timer
                IO_time += std::chrono::high_resolution_clock::now() - start_IO_time;
                auto start_user_time = std::chrono::high_resolution_clock::now();
                
                // Get the x&y dimensions of the images
                std::array<std::ptrdiff_t,2> dim;
                if (trpx_data.dim().size() == 0) // No dimensions given, so assume a square image
                    dim[0] = dim[1] = std::sqrt(trpx_data.size());
                else
                    std::copy_n(trpx_data.dim().begin(), 2, dim.begin());
                
                jpa::Grey_tif<std::byte> tif_data;
                // Expand the images in the Terse stack and push them on the tiff stack
                if (trpx_data.bits_per_val() <= 16 && trpx_data.is_signed()) {
                    for (int i = 0; i != trpx_data.number_of_frames(); ++i) {
                        tif_data.push_back<std::int16_t>(dim);
                        trpx_data.prolix(tif_data.image<std::int16_t>(i), i);
                    }
                }
                else if (trpx_data.bits_per_val() <= 16 && !trpx_data.is_signed()) {
                    for (int i = 0; i != trpx_data.number_of_frames(); ++i) {
                        tif_data.push_back<std::uint16_t>(dim);
                        trpx_data.prolix(tif_data.image<std::uint16_t>(i), i);
                    }
                }
                else if (trpx_data.bits_per_val() <= 32 && trpx_data.is_signed()) {
                    for (int i = 0; i != trpx_data.number_of_frames(); ++i) {
                        tif_data.push_back<std::int32_t>(dim);
                        trpx_data.prolix(tif_data.image<std::int16_t>(i), i);
                    }
                }
                else if (trpx_data.bits_per_val() <= 32 && !trpx_data.is_signed()) {
                    for (int i = 0; i != trpx_data.number_of_frames(); ++i) {
                        tif_data.push_back<std::uint32_t>(dim);
                        trpx_data.prolix(tif_data.image<std::uint16_t>(i), i);
                    }
                }
                else {
                    std::cerr << "Terse file " << filename << " encodes data that requires 64 bits per pixel." <<  std::endl;
                    std::cerr << "Prolix cannot process such trpx-stacks." << std::endl;
                    return 0;
                }
                
                // Stop the user timer, start the IO timer
                user_time += std::chrono::high_resolution_clock::now() - start_user_time;
                start_IO_time = std::chrono::high_resolution_clock::now();
                
                // Write the compressed data to the tif file
                std::ofstream tif_file(filename.replace_extension(".tif"), std::ios::binary);
                if (!tif_file.is_open())
                    std::cerr << "Failed to open trpx file " << filename.replace_extension(".trpx") << std::endl;
                else {
                    tif_data.write(tif_file);
                    tif_file.close();
                    fs::remove(filename.replace_extension(".trpx"));
                    ++expanded_files;
                }
            }
            // stop the IO timer
            IO_time += std::chrono::high_resolution_clock::now() - start_IO_time;
        }
    }
    
    // If required, provide verbose output
    if (input.option("-verbose").found()) {
        for (fs::path tif_filename : input.params())
            std::cout << "Expanded: " << tif_filename << std::endl;
        std::cout << "Prolix expanded : " << expanded_files << " files\n";
        std::cout << "User time       : " << user_time.count() << " seconds\n";
        std::cout << "IO time         : " << IO_time.count() << " seconds\n";
    }
    return 0;
}
