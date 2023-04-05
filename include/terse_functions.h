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
#include <bit>
#include "Operators.hpp"

#ifndef COMPRESSION_APP_FUNCTIONS_H
#define COMPRESSION_APP_FUNCTIONS_H


//Function declarations

bool is_machine_little_endian();

void write_Medipix_quad(std::ofstream& ostream, const std::vector<uint16_t>& img);

std::vector<uint16_t> read_Medipix_quad(std::ifstream& istream);

std::filesystem::path create_temp_image_file(const std::vector<uint16_t>& img_data);


#endif //COMPRESSION_APP_FUNCTIONS_H
