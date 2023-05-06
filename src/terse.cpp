//
//  main.cpp
//  Created by Senik Matinyan
//
//  Revised by Jan Pieter Abrahams on 19.03.23.
//

#include <iostream>
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
        std::cout << "  compresses all files with .tiff or .tif extensions to terse files with .trs extensions, and expands terse files to tiff files.\n";
        std::cout << "Examples:\n";
        std::cout << "   terse *                   // all tiff files in this directory are compressed to terse files and all terse files are expanded to tiff files.\n";
        std::cout << "   terse ˜/dir/my_img*.tiff  // compresses all tiff files in the directory ~/dir that start with my_img\n";
        std::cout << "   terse ˜/dir/my_img*.trs   // expands all terse files in the directory ~/dir that start with my_img\n";
        std::cout << input.help() << std::endl;
        for (auto f : input.data())
            std::cout << "   input file: " << f << "\n";
        return 0;
    }

    for (const auto& filename : input.data()) {
        fs::path entry = filename;
        const bool is_tif = entry.extension() == ".tiff" || entry.extension() == ".tif";
        if (fs::is_regular_file(entry) && (is_tif || entry.extension() == ".trs")) {
            const fs::path input_file_path = entry;
            const fs::path output_file_path = entry.replace_extension(is_tif ? ".trs" : ".tif");

            // Open input & output files.
            std::ifstream input_file(input_file_path, std::ios::binary);
            std::ofstream output_file(output_file_path, std::ios::binary);
            if (!input_file.is_open())
                std::cerr << "Failed to open input file " << input_file_path << std::endl;
            else if (!output_file.is_open())
                std::cerr << "Failed to open output file " << output_file_path << std::endl;
            else {
                std::vector<std::uint16_t> img(512*512);
                if (is_tif) { // read tif data and write out terse file
                    jpa::read_tiff_Medipix(input_file, img);
                   output_file << Terse(img);
                }
                else { // read terse data and write out tiff file
                    Terse(input_file).prolix(img.begin());
                    jpa::write_tiff_Medipix(output_file, img);
                }
                output_file.close();
                input_file.close();

                // Delete input file.
                fs::remove(input_file_path);
                if (input.found("-verbose"))
                    std::cout << "Compressed: " << input_file_path << std::endl;
            }
        }
    }
    return 0;
}
