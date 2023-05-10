//
//  main.cpp
//  Created by Senik Matinyan
//
//  Revised by Jan Pieter Abrahams on 19.03.23.
//

#include <iostream>
#include <chrono>
#include "Terse_new.hpp"
#include "Command_line.hpp"
#include "Medipix_tiff.hpp"

namespace fs = std::filesystem;

int main(int argc, char const* argv[]) {
    Command_line_tag help("-help", "print help");
    Command_line_tag verbose("-verbose", "print compute times and compression rate");
    Command_line_tag list_files("-list", "list compressed files");
    Command_line input(argc, argv, help, verbose, list_files);
    if (input.found("-help")) {
        std::cout << "terse [-help] [-verbose] [-list] [file ...]\n";
        std::cout << "  compresses all files with .tiff or .tif extensions to terse files with .trs extensions.\n";
        std::cout << "Examples:\n";
        std::cout << "   terse *                   // all tiff files in this directory are compressed to terse files.\n";
        std::cout << "   terse ˜/dir/my_img*       // compresses all tiff files in the directory ~/dir that start with my_img\n";
        std::cout << input.help() << std::endl;
        return 0;
    }
    
    std::chrono::duration<double> user_time;
    std::chrono::duration<double> IO_time;
    std::vector<std::uint16_t> img(512*512);
    double compression_rate = 0;
    std::size_t compressed_files = 0;
    
    for (const auto& filename : input.data()) {
        fs::path entry = filename;
        const bool is_tif = entry.extension() == ".tiff" || entry.extension() == ".tif";
        if (fs::is_regular_file(entry) && is_tif) {
            const fs::path input_file_path = entry;
            const fs::path output_file_path = entry.replace_extension(".trs");
            auto start_IO_time = std::chrono::high_resolution_clock::now();
            std::ifstream input_file(input_file_path, std::ios::binary);
            std::ofstream output_file(output_file_path, std::ios::binary);
            if (!input_file.is_open())
                std::cerr << "Failed to open input file " << input_file_path << std::endl;
            else if (!output_file.is_open())
                std::cerr << "Failed to open output file " << output_file_path << std::endl;
            else {
                jpa::read_tiff_Medipix(input_file, img);
                IO_time += std::chrono::high_resolution_clock::now() - start_IO_time;
                auto start_user_time = std::chrono::high_resolution_clock::now();
                auto compressed = Terse<std::uint8_t>(img);
                compression_rate += compressed.terse_size() / (2 * 512.0 * 512.0);
                ++compressed_files;
                user_time += std::chrono::high_resolution_clock::now() - start_user_time;
                auto start_IO_time = std::chrono::high_resolution_clock::now();
                output_file << compressed;
                IO_time += std::chrono::high_resolution_clock::now() - start_IO_time;
            }

            // Close files and delete input file.
            start_IO_time = std::chrono::high_resolution_clock::now();
            output_file.close();
            input_file.close();
            fs::remove(input_file_path);
            IO_time += std::chrono::high_resolution_clock::now() - start_IO_time;
            if (input.found("-list"))
                std::cout << "Compressed: " << input_file_path << std::endl;
        }
    }
    if (input.found("-verbose")) {
        std::cout << "terse compressed: " << compressed_files << " files\n";
        std::cout << "User time       : " << user_time.count() << " seconds\n";
        std::cout << "IO time         : " << IO_time.count() << " seconds\n";
        std::cout << "compression rate: " << std::round(1000 * (1 - compression_rate / compressed_files)) / 10 << "%\n";
    }
    return 0;
}
