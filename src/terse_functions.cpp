//
// Created by Senik Matinyan on 24.03.23.
//


#include "terse_functions.h"

//Function definitons

std::vector<uint16_t> read_Medipix_quad(std::ifstream& istream) {
    // Read tiff header
    char tiff_header[8];
    istream.read(tiff_header, 8);

    // Tiff header indicates if the data are little- or big-endian
    bool swapped = std::string(tiff_header, 2) == "II" && std::endian::native == std::endian::little;

    // Check if this is an uncompressed Medipix tiff file, and if the byte order was correctly deduced
    std::uint16_t fortytwo = reinterpret_cast<uint16_t&>(tiff_header[2]);
    std::uint32_t data_size = reinterpret_cast<uint32_t&>(tiff_header[4]);
    if (swapped) {
        fortytwo = Operator::swap_bytes(fortytwo);
        data_size = Operator::swap_bytes(data_size);
    }
    /*
    if (fortytwo != 42 || data_size != 512*512*2) {
        std::cerr << "Error: not a Medipix tiff file" << std::endl;
        exit(0);
    }
    */

    // Read the image data and return it
    std::vector<uint16_t> img(512*512*2);
    istream.read(reinterpret_cast<char*>(img.data()), data_size);
    if (swapped)
        for (uint16_t& pixel : img)
            pixel = Operator::swap_bytes(pixel);
    return img;
}

std::filesystem::path create_temp_image_file(const std::vector<uint16_t>& img_data) {
    std::filesystem::path temp_file_path = std::filesystem::temp_directory_path() / "temp_image.tiff";
    std::ofstream temp_file(temp_file_path, std::ios::binary);

    // Create a vector of size 512*512*2
    std::vector<uint16_t> img_data_resized(512*512*2);

    // Fill the new vector with the original test data
    for (size_t i = 0; i < img_data.size(); ++i) {
        img_data_resized[i] = img_data[i];
    }

    // Write the test image data to the temporary file
    write_Medipix_quad(temp_file, img_data_resized);
    temp_file.close();

    return temp_file_path;
}


//Function definitions

bool is_machine_little_endian() {
    uint32_t x = 0x01020304;
    uint8_t* p = reinterpret_cast<uint8_t*>(&x);
    return *p == 0x04;
}

void write_Medipix_quad(std::ofstream& ostream, const std::vector<uint16_t>& img) {
    // Determine endianess
    bool is_little_endian = is_machine_little_endian();


    // TIFF header
    const char tiff_header[8] = {
            is_little_endian ? 'I' : 'M',
            is_little_endian ? 'I' : 'M',
            0x2A, 0x00,
            0x00, 0x02,
            0x00, 0x00
    };
    ostream.write(tiff_header, 8);

    // Image data
    if (is_little_endian) {
        ostream.write(reinterpret_cast<const char*>(img.data()), img.size() * sizeof(uint16_t));
    } else {
        std::vector<uint16_t> swapped_img(img.size());
        std::transform(img.begin(), img.end(), swapped_img.begin(), [](const auto& val) { return __builtin_bswap16(val); });
        ostream.write(reinterpret_cast<const char*>(swapped_img.data()), swapped_img.size() * sizeof(uint16_t));
    }
}
