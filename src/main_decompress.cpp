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
    Command_line_tag folder_path_name("-path", "Path to the folder",  {"./"});
    Command_line_tag help("-help", {});
    Command_line input(argc, argv, help, folder_path_name);
    if (input.found("-help")) {
        std::cout << input.help() << std::endl;
        return 0;
    }

    fs::path folder_path(std::vector<std::string>(input.value<std::string>("-path"))[0]);

    // Print the folder path
    std::cout << "Folder path provided: " << folder_path << std::endl;

    const std::string input_extension = ".trs";
    const std::string output_extension = ".tiff";
    const fs::path input_dir_path = folder_path;
    const fs::path output_dir_path = folder_path;

    for (const auto& entry : fs::directory_iterator(input_dir_path)) {
        if (entry.is_regular_file() && entry.path().extension() == input_extension) {
            const fs::path input_file_path = entry.path();
            const fs::path output_file_path = output_dir_path / (input_file_path.stem().string() + output_extension);


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


