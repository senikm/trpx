//
// Created by Senik Matinyan on 23.03.23.
//
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <numeric>
#include <cstdint>
#include "Terse.hpp"
#include "Command_line.hpp"
#include "Medipix_tiff.hpp"

namespace fs = std::filesystem;



int main(int argc, char const* argv[]) {
    using namespace jpa;
    Command_line_tag help("-help", "print help");
    Command_line_tag verbose("-verbose", "provide extra output");
    Command_line input(argc, argv, help, verbose);
    if (input.found("-help")) {
        std::cout << "terse [-help] [-verbose] [file ...]\n";
        std::cout << "  converts all files with .tiff extensions with to terse files with .trs extensions.\n";
        std::cout << "Example:\n";
        std::cout << "   prolix *                   // decompresses all files in the currrent directory\n";
        std::cout << "   prolix ˜/dir/*             // decompresses all files in the the directory ~/dir\n";
        std::cout << "   prolix ˜/dir/my_img*       // decompresses all files in the the directory ~/dir that start with my_img\n";
        std::cout << input.help() << std::endl;
        for (auto f : input.data())
            std::cout << "   input file: " << f << "\n";
    }

    const std::string input_extension = ".trs";
    const std::string output_extension = ".tiff";
    const std::vector<std::string> tiffs = input.data();
    
    for (const auto& filename : tiffs) {
        fs::path entry = filename;
        if (fs::is_regular_file(entry) && entry.extension() == input_extension) {
            const fs::path input_file_path = entry;
            const fs::path output_file_path = entry.replace_extension(output_extension);
            
            // Read input file.
            std::ifstream input_file(input_file_path, std::ios::binary);
            if (!input_file.is_open()) {
                std::cerr << "Failed to open input file " << input_file_path << std::endl;
                continue;
            }

            Terse compressed(input_file);
            std::cout << "Compressed size: " << compressed.size() << std::endl;


            // Decompress
            std::vector<uint16_t> decompressed_data(512*512);
            compressed.prolix(decompressed_data.begin());


            // Write output file.
            std::ofstream output_file(output_file_path, std::ios::binary);
            if (!output_file.is_open()) {
                std::cerr << "Failed to open output file " << output_file_path << std::endl;
                continue;
            }
            // Write output file.
            jpa::write_tiff_Medipix<uint16_t>(output_file_path.string(), decompressed_data);

            input_file.close();
            output_file.close();
            std::cout << "Done processing file: " << input_file_path << std::endl;
            }

        }

    return 0;
}


