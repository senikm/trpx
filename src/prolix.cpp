//
//  main.cpp
//  Created by Senik Matinyan
//
//  Revised by Jan Pieter Abrahams on 19.03.23.
//

#include <iostream>
#include <chrono>
#include "Terse.hpp"
#include "Command_line.hpp"
#include "tiff_library.hpp"

namespace fs = std::filesystem;

int main(int argc, char const* argv[]) {
    Command_line_tag help("-help", "print help");
    Command_line_tag verbose("-verbose", "print compute times");
    Command_line_tag list_files("-list", "list compressed files");
    Command_line input(argc, argv, help, verbose, list_files);
    if (input.found("-help")) {
        std::cout << "prolix [-help] [-verbose] [file ...]\n";
        std::cout << "  expands terse files to tiff files.\n";
        std::cout << "Examples:\n";
        std::cout << "   prolix *                   // all terse files with .trs extensions are expanded to tiff files with .tif extensions.\n";
        std::cout << "   prolix ˜/dir/my_img*  // compresses all tiff files in the directory ~/dir that start with my_img\n";
        std::cout << input.help() << std::endl;
        return 0;
    }
    
    std::chrono::duration<double> user_time;
    std::chrono::duration<double> IO_time;
    std::vector<std::uint16_t> img(512*512);
    std::size_t expanded_files = 0;

    for (const auto& filename : input.data()) {
        fs::path entry = filename;
        if (fs::is_regular_file(entry) && entry.extension() == ".trs") {
            const fs::path input_file_path = entry;
            const fs::path output_file_path = entry.replace_extension(".tif");

            auto start_IO_time = std::chrono::high_resolution_clock::now();
            std::ifstream input_file(input_file_path, std::ios::binary);
            std::ofstream output_file(output_file_path, std::ios::binary);
            if (!input_file.is_open())
                std::cerr << "Failed to open input file " << input_file_path << std::endl;
            else if (!output_file.is_open())
                std::cerr << "Failed to open output file " << output_file_path << std::endl;
            else {
                jpa::Grey_tif tif;
                auto compressed = Terse(input_file);
                IO_time += std::chrono::high_resolution_clock::now() - start_IO_time;
                auto start_user_time = std::chrono::high_resolution_clock::now();
                compressed.prolix(img.begin());
                user_time += std::chrono::high_resolution_clock::now() - start_user_time;
                auto start_IO_time = std::chrono::high_resolution_clock::now();
                tif.push_back(img, {512,512});
                output_file<<tif;
                ++expanded_files;
                IO_time += std::chrono::high_resolution_clock::now() - start_IO_time;
            }
 
            // Close files and delete input file.
            start_IO_time = std::chrono::high_resolution_clock::now();
            output_file.close();
            input_file.close();
            fs::remove(input_file_path);
            IO_time += std::chrono::high_resolution_clock::now() - start_IO_time;

            if (input.found("-list"))
                std::cout << "Expanded: " << input_file_path << std::endl;
        }
    }
    if (input.found("-verbose")) {
        std::cout << "prolix expanded: " << expanded_files << " files\n";
        std::cout << "User time      : " << user_time.count() << " seconds\n";
        std::cout << "IO time        : " << IO_time.count() << " seconds\n";
    }
    return 0;
}
