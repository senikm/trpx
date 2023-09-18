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
#include "Terse.hpp"
#include "Command_line.hpp"
#include "Grey_tif.hpp"

namespace fs = std::filesystem;

template <typename T> void Terse_pushback(jpa::Terse&, jpa::Grey_tif_image<T> const&);

int main(int argc, char const* argv[]) {
    using namespace jpa;
    Command_line_option help("-help", "print help");
    Command_line_option verbose("-verbose", "print compressed filenames, compute times and compression rate");
    Command_line input(argc, argv, {help, verbose});
    if (input.option("-help").found()) {
        std::cout << "terse [-help] [-verbose] [file ...]\n";
        std::cout << "  compresses all files with .tiff or .tif extensions to terse files with .trpx extensions.\n";
        std::cout << "Examples:\n";
        std::cout << "   terse *                   // all tiff files in this directory are compressed to trpx files.\n";
        std::cout << "   terse ˜/dir/my_img*       // compresses all tiff files in the directory ~/dir that start with my_img\n";
        std::cout << "\nkeywords:\n";
        std::cout << input.help() << std::endl;
        return 0;
    }
    
    // Some timers and counters are required for the 'verbose' option.
    std::chrono::duration<double> user_time(0);
    std::chrono::duration<double> IO_time(0);
    double total_trpx_size = 0;
    double total_tiff_size = 0;
    std::size_t compressed_files = 0;
    
    // Loop over all input file names
    for (fs::path tif_filename : input.params()) {
        
        // Only tif files will be compressed
        if (fs::is_regular_file(tif_filename) && (tif_filename.extension() == ".tiff" ||
                                                  tif_filename.extension() == ".tif" ||
                                                  tif_filename.extension() == ".TIFF" ||
                                                  tif_filename.extension() == ".TIF")) {

            // Start the IO timer and open the next file
            auto start_IO_time = std::chrono::high_resolution_clock::now();
            std::ifstream tif_file(tif_filename, std::ios::binary);
            if (!tif_file.is_open())
                std::cerr << "Failed to open input file " << tif_filename << std::endl;
            else {
                
                // A tiff file was opened. Read its data. It may contain one or more images in a stack.
                jpa::Grey_tif<std::byte> tif_data(tif_file);
                tif_file.close();
                    
                total_tiff_size += tif_data.raw_data_size();
                
                // stop the IO timer, start the user timer
                IO_time += std::chrono::high_resolution_clock::now() - start_IO_time;
                auto start_user_time = std::chrono::high_resolution_clock::now();
                
                Terse compressed;
                
                for (int i = 0; i != tif_data.image_stack_size(); ++i)
                    if (tif_data.dim() == tif_data.image(i).dim())
                        Terse_pushback(compressed, tif_data.image(i));
                    else {
                        std::cerr << "Tiff file " << tif_filename << " contains a stack of images with varying sizes." <<  std::endl;
                        std::cerr << "Terse cannot process such tiff-stacks. First unstack this tiff file and compress the images separately." << std::endl;
                        return 0;
                    }
                 total_trpx_size += compressed.terse_size();
                
                // Stop the user timer, start the IO timer
                user_time += std::chrono::high_resolution_clock::now() - start_user_time;
                start_IO_time = std::chrono::high_resolution_clock::now();
                
                // Write the compressed data to the trpx file
                auto trpx_filename = tif_filename;
                std::ofstream trpx_file(trpx_filename.replace_extension(".trpx"), std::ios::binary);
                if (!trpx_file.is_open())
                    std::cerr << "Failed to open trpx file " << trpx_filename << std::endl;
                else {
                    compressed.write(trpx_file);
                    trpx_file.close();
                    fs::remove(tif_filename);
                    ++compressed_files;
                }
            }
            
            // stop the IO timer
            IO_time += std::chrono::high_resolution_clock::now() - start_IO_time;
        }
    }
    
    // If required, provide verbose output
    if (input.option("-verbose").found()) {
        for (fs::path tif_filename : input.params()) 
            std::cout << "Compressed: " << tif_filename << std::endl;
        std::cout << "Terse compressed: " << compressed_files << " files\n";
        std::cout << "User time       : " << user_time.count() << " seconds\n";
        std::cout << "IO time         : " << IO_time.count() << " seconds\n";
        if (total_tiff_size > 0)
            std::cout << "Compression rate: " << std::round(1000 * (1 - total_trpx_size / total_tiff_size)) / 10 << "%\n";
    }
    return 0;
}

template <typename T>
void Terse_pushback(jpa::Terse& compressed, jpa::Grey_tif_image<T> const& img) {
    using namespace jpa;
    if constexpr (!std::is_same_v<T, std::byte>)
        compressed.push_back(img);
    else {
        POD_type_traits const& img_type = img.type();
        if      (img_type.is<std::int8_t>())   Terse_pushback<std::int8_t>  (compressed, img);
        else if (img_type.is<std::uint8_t>())  Terse_pushback<std::uint8_t> (compressed, img);
        else if (img_type.is<std::int16_t>())  Terse_pushback<std::int16_t> (compressed, img);
        else if (img_type.is<std::uint16_t>()) Terse_pushback<std::uint16_t>(compressed, img);
        else if (img_type.is<std::int32_t>())  Terse_pushback<std::int32_t> (compressed, img);
        else if (img_type.is<std::uint32_t>()) Terse_pushback<std::uint32_t>(compressed, img);
        else {
            std::vector<int64_t> tmp(img.dim()[0] * img.dim()[1]);
            if      (img_type.is<float>())  std::copy_n(Grey_tif_image<float const>(img).begin(),  tmp.size(), tmp.begin());
            else if (img_type.is<double>()) std::copy_n(Grey_tif_image<double const>(img).begin(), tmp.size(), tmp.begin());
            compressed.push_back(tmp);
        }
    }
}
