//
//  main.cpp
//  Created by Senik Matinyan
//
//  Revised by Jan Pieter Abrahams on 19.03.23.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <bit>
#include "Terse.hpp"
#include "Command_line.hpp"
#include "Medipix_tiff.hpp"

namespace fs = std::filesystem;


int main(int argc, char const* argv[]) {
    using namespace jpa;
    Command_line_tag folder_path_name("-path", "Path to the folder",  {"./"});
    Command_line_tag help("-help", {});
    Command_line_tag verbose("-verbose", {});
    Command_line input(argc, argv, help, verbose, folder_path_name);
    if (input.found("-help")) {
        std::cout << input.help() << std::endl;
        return 0;
    }
    
    fs::path folder_path(std::vector<std::string>(input.value<std::string>("-path"))[0]);

      // Print the folder path
    std::cout << "Folder path provided: " << folder_path << std::endl;

    const std::string input_extension = ".tiff";
    const std::string output_extension = ".trs";
    const fs::path input_dir_path = folder_path;
    const fs::path output_dir_path = folder_path;
    
    for (const auto& entry : fs::directory_iterator(input_dir_path)) {
        if (entry.is_regular_file() && entry.path().extension() == input_extension) {
            const fs::path input_file_path = entry.path();
            const fs::path output_file_path = output_dir_path / (input_file_path.stem().string() + output_extension);
            
            //while (fs::file_size(input_file_path) < (512*512*2 + 8)) {
            //std::this_thread::sleep_for(std::chrono::milliseconds(10)); // wait 10 milliseconds to allow finishing file write
            //}
            
            // Read input file.
            std::ifstream input_file(input_file_path, std::ios::binary);
            if (!input_file.is_open()) {
                std::cerr << "Failed to open input file " << input_file_path << std::endl;
                continue;
            }
            std::vector<std::uint16_t> input_data (512*512);
            jpa::read_tiff_Medipix(input_file_path.string(), input_data);
            
            
            // Write output file.
            std::ofstream output_file(output_file_path, std::ios::binary);
            if (!output_file.is_open()) {
                std::cerr << "Failed to open output file " << output_file_path << std::endl;
                continue;
            }
            Terse compressed(input_data);
            output_file << compressed;
            output_file << compressed;
            output_file.close();
            input_file.close();
            // Delete input file.
            fs::remove(input_file_path);
            std::cout << "Done processing file: " << input_file_path << std::endl;
        }
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
