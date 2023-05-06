//
//  Medipix_tiff.hpp
//  Medipix_tiff
//
//  Created by Jan Pieter Abrahams on 25.03.23.
//

#ifndef Medipix_tiff_h
#define Medipix_tiff_h

#include <fstream>
#include <filesystem>
#include <bit>
#include <array>
#include "Operators.hpp"

// THIS IS NOT A GENERAL TIFF LIBRARY!
//
// Routines for reading and writing Medipix TIFF files. These are TIFF IMAGE files that have the following in common:
//  - one image per TIFF file;
//  - one grayscale intensity value per pixel;
//  - the intensity data always begin on position 8 in the TIFF file, to allow them being read as raw
//    data by programs without TIFF libraries;
//  - by default, the intensities are stored in the file 16-bit unsigned integers, but this can be overridden
//    and both 8-bit and 32-bit unsigned integers are also allowed;
//  - by default, the image size is 512*512 pixels, but this can also be overridden;
// Data can be read into or written out from any type of container storing any type of numerical values.
//
// The library contains four functions that live in the namespace jpa:
//  - read_tiff_Medipix(std::filesystem::path const& path, C& container)
//    read_tiff_Medipix(std::ifstream& tiff, C& container)
//      These functions read a Medipix TIFF file into any 1- or 2-dimensional container type.
//  - write_tiff_Medipix(std::filesystem::path path, auto const& data, std::size_t const size_x, std::size_t const size_y)
//  - write_tiff_Medipix(std::ifstream& tiff, auto const& data, std::size_t const size_x, std::size_t const size_y)
//      These functions write any 1- or 2-dimensional container type to a Medipix TIFF file.
//
// Example:
//    #include <iostream>
//    #include <vector>
//    #include "Medipix_tiff.hpp"
//
//    int main(int argc, const char * argv[]) {
//        using namespace jpa;
//        std::vector<std::uint16_t> img0;
//        float img1[512][512];
//        jpa::read_tiff_Medipix("infile", img0); // reads data into a 1D vector
//        jpa::read_tiff_Medipix("infile.tiff", img1); // reads data into a 2D array of float
//        jpa::write_tiff_Medipix("outfile0.tiff", img0); // writes 16-bit tiff
//        jpa::write_tiff_Medipix<std::uint32_t>("outfile1", img1); // writes 32-bit tiff
//        return 0;
//    }
//
// Functions for reading:
//
// template <typename C>
//    requires (requires (C& c) {std::begin(c), std::end(c), std::size(c);})
//    std::array<std::size_t, 2> read_tiff_Medipix(std::ifstream& tiff,
//                                                 C& container)
//  Reads the tiff file 'tiff' into 'container'. The container can be any type of 1D or 2D container.
//  If required, the values in the tiff file are converted to the values of the container.
//  Returns the size of the TIFF image as a 2D array.
//  The size of the container is checked and if this is different from the tiff file, compilation fails
//  (for static arrays), the container is resized (if possible), or an error is reported on std::cerr and
//  an image size of {0,0} is returned.
//
// template <typename C>
//    requires (requires (C& c) {std::begin(c), std::end(c), std::size(c);})
//    std::array<std::size_t, 2> read_tiff_Medipix(std::filesystem::path const& path,
//                                                 C& container)
//  Opens the file defined by 'path' and calls read_tiff_Medipix(std::ifstream& tiff, C& container). In
//  case of a file error, reports on std::cerr and returns {0,0}. Checks if the extension is .tiff or .tif.
//  The extension does not have to be specified in the 'path' parameter.
//
// Functions for writing:
//
// template <typename TiffT = uint16_t>
//    requires std::is_same_v<uint8_t, TiffT> || std::is_same_v<uint16_t, TiffT> || std::is_same_v<uint32_t, TiffT>
//    std::size_t write_tiff_Medipix(std::ofstream& tiff,
//                                   auto const& data,
//                                   std::size_t const size_x = 0,
//                                   std::size_t const size_y = 0)
//  Writes 'data' to the tiff file 'tiff'. The parameter 'data' can be a 1D- or 2D container, or an iterator.
//  Returns the number of pixels written to disk.
//  The function has an optional a template argument:
//      write_tiff_Medipix()           writes an 16-bit tiff file;
//      write_tiff_Medipix<uint8_t >() writes an  8-bit tiff file;
//      write_tiff_Medipix<uint16_t>() writes an 16-bit tiff file;
//      write_tiff_Medipix<uint32_t>() writes an 32-bit tiff file;
//  If 'size_x' and 'size_y' are both zero (default values), the image size is set to 512*512 if 'data'is a
//  1D container or an iterator. In case of a 2D container, the file size is set to the values inferred from the
//  sizes of the 2D container if 'size_x' and 'size_y' are both zero. In case of a size mismatch, an error
//  is reported on std::cerr and 0 is returned.
//
// template <typename TiffT = uint16_t>
//    requires std::is_same_v<uint8_t, TiffT> || std::is_same_v<uint16_t, TiffT> || std::is_same_v<uint32_t, TiffT>
//    std::size_t write_tiff_Medipix(std::filesystem::path const& path,
//                                   auto const& data,
//                                   std::size_t const size_x = 0,
//                                   std::size_t const size_y = 0)
//  Opens the file defined by 'path' and calls write_tiff_Medipix(std::ifstream& tiff, C& container). In
//  case of a file error, reports on std::cerr and returns 0. Checks if the extension is .tiff or .tif.
//  The extension does not have to be specified in the 'path' parameter, in which case '.tiff' is appended.

namespace jpa {

template <typename C> requires (requires (C& c) {std::begin(c), std::end(c), std::size(c);})
std::array<std::size_t, 2> read_tiff_Medipix(std::ifstream& tiff, C& container) {
    if constexpr (std::is_integral_v<std::remove_cvref_t<decltype(container[0])>>) { // 1D container
        std::array<std::size_t, 2> dim{0, 0};
        int pixel_size(1);
        char header[8];
        tiff.read(header, 8);
        bool native = std::string_view(header, 2) == "II" && std::endian::native == std::endian::little;
        auto as_uint16  = [native](char const& val) { uint16_t r = reinterpret_cast<uint16_t const&>(val); return native ? r : Operator::swap_bytes(r); };
        auto as_uint32  = [native](char const& val) { uint32_t r = reinterpret_cast<uint32_t const&>(val); return native ? r : Operator::swap_bytes(r); };
        std::uint16_t fortytwo = as_uint16(header[2]);
        std::uint32_t data_size = as_uint32(header[4]);
        if ((header[0] != 'I' && header[0] != 'M') || header[0] != header[1] || fortytwo != 42) {
            std::cerr << "Error: not a tiff file" << std::endl;
            return dim;
        }
        data_size -= 8;
        bool buffered = !std::is_integral_v<std::remove_cvref_t<decltype(container[0])>> || data_size != sizeof(container[0]) * (1 + &container[std::size(container) - 1] - &container[0]);
        std::vector<uint8_t> buffer;
        if (!buffered)
            tiff.read(reinterpret_cast<char*>(&container[0]), data_size);
        else {
            buffer.resize(data_size);
            tiff.read(reinterpret_cast<char*>(buffer.data()), data_size);
        }
        char tiff_IDF_entries[2];
        tiff.read(tiff_IDF_entries, 2);
        for (int i = as_uint16(tiff_IDF_entries[0]); i != 0; --i) {
            char tiff_IDF[12];
            tiff.read(tiff_IDF, 12);
            uint16_t tag = as_uint16(tiff_IDF[0]);
            uint32_t val = as_uint32(tiff_IDF[8]);
            if (tag == 0x0100) dim[0] = val;
            else if (tag == 0x0101) dim[1] = val;
            else if (tag == 0x0102)  pixel_size = val / 8;
            else if (tag == 0x0111 && val != 8 && dim[0] * dim[1] * pixel_size != data_size) {
                std::cerr << "Error: not a Medipix tiff file" << std::endl;
                return {0, 0};
            }
        }
        if (!buffered && !native)
            std::for_each_n (std::begin(container), dim[0] * dim[1], [](auto& val) { val = Operator::swap_bytes(val); });
        else if (buffered) {
            if constexpr (requires (C& c) {c.resize(1);})
                container.resize(dim[0] * dim[1]);
            else if (std::size(container) != data_size / pixel_size) {
                std::cerr << "Error: size of Medipix tiff file {" << dim[0] << ", " << dim[1] << "} ";
                std::cerr << "and 1D container {" << std::size(container) << "} differ" << std::endl;
                return {0, 0};
            }
            if (pixel_size == 1)
                std::copy_n(buffer.data(), dim[0] * dim[1], container.begin());
            else if (native && pixel_size == 2)
                std::copy_n(reinterpret_cast<uint16_t*>(buffer.data()), dim[0] * dim[1], container.begin());
            else if (native && pixel_size == 4)
                std::copy_n(reinterpret_cast<uint32_t*>(buffer.data()), dim[0] * dim[1], container.begin());
            else if (!native && pixel_size == 2)
                std::transform(reinterpret_cast<uint16_t*>(buffer.data()), reinterpret_cast<uint16_t*>(buffer.data()) + dim[0] * dim[1], container.begin(),
                               [](auto const& val) {return Operator::swap_bytes(val);});
            else if (!native && pixel_size == 4)
                std::transform(reinterpret_cast<uint32_t*>(buffer.data()), reinterpret_cast<uint32_t*>(buffer.data()) + dim[0] * dim[1], container.begin(),
                               [](auto const& val) {return Operator::swap_bytes(val);});
        }
        return dim;
    }
    else if constexpr (std::is_integral_v<std::remove_cvref_t<decltype(container[0][0])>>) { // 2D container
        std::vector<std::remove_cvref_t<decltype(container[0][0])>> buffer;
        std::array<std::size_t,2> dim = read_tiff_Medipix(tiff, buffer);
        if (dim[0] != std::size(container) || dim[1] != std::size(container[0])) {
            if constexpr (requires (C& c) { c.dim(), c.dim({1,1}); }) {
                if (container.dim()[0] == 0 && container.dim()[1] == 0) {
                    container(dim);
                    std::copy(buffer.begin(), buffer.end(), container.data());
                    return dim;
                }
                else {
                    std::cerr << "Error: size of Medipix tiff file {" << dim[0] << ", " << dim[1] << "} ";
                    std::cerr << "and 2D array-type container {" << std::size(container) << ", " << std::size(container[0]) << "} differ" << std::endl;
                    return {0, 0};
                }
            }
            else {
                std::cerr << "Error: size of Medipix tiff file {" << dim[0] << ", " << dim[1] << "} ";
                std::cerr << "and 2D array-type container {" << std::size(container) << ", " << std::size(container[0]) << "} differ" << std::endl;
                return {0,0};
            }
        }
        auto iter = buffer.begin();
        for (int i=0; i != dim[0]; ++i)
            for (int j=0; j != dim[0]; ++j)
                container[i][j] = *iter++;
        return dim;
    }
    else {
        static_assert(std::is_integral_v<std::remove_cvref_t<decltype(container[0])>>, "Cannot read Medipix tiff data into specified container.");
        return {0,0};
    }
}



template <typename C> requires (requires (C& c) {std::begin(c), std::end(c), std::size(c);})
std::array<std::size_t, 2> read_tiff_Medipix(std::filesystem::path const& path, C& container) {
    std::string extension = path.extension();
    for (auto& c : extension)
        c = std::tolower(c);
    if (is_regular_file(path) && (extension == ".tiff" || extension == ".tif")) {
        std::ifstream tiff(path, std::ios::binary);
        if (!tiff.is_open()) {
            std::cerr << "Failed to open input file " << path << std::endl;
            return {0, 0};
        };
        return read_tiff_Medipix(tiff, container);
    }
    std::cerr << "Error: not a Medipix tiff file" << std::endl;
    return {0, 0};
}

template <typename TiffT = uint16_t> requires std::is_same_v<uint8_t, TiffT> || std::is_same_v<uint16_t, TiffT> || std::is_same_v<uint32_t, TiffT>
std::size_t write_tiff_Medipix(std::ofstream& tiff, auto const& data, std::size_t const size_x = 0, std::size_t const size_y = 0) {
    if constexpr (requires (decltype(data) c) {std::begin(data);}) {
        if constexpr (std::is_integral_v<std::remove_cvref_t<decltype(data[0])>>)  // 1D container
            return write_tiff_Medipix<TiffT>(tiff, std::begin(data), size_x, size_y);
        else if constexpr (std::is_integral_v<std::remove_cvref_t<decltype(data[0][0])>>) { // 2D container)
            if (std::size(data) * std::size(data[0]) - 1 != &data[std::size(data)-1][std::size(data[0])-1] - &data[0][0]) {
                std::cerr << "non-contiguous data in 2D container"  << std::endl;
                return 0;
            };
            if (size_x == 0 && size_y == 0)
                return write_tiff_Medipix<TiffT>(tiff, &data[0][0], std::size(data), std::size(data[0]));
            else if (std::size(data) == size_x && std::size(data[0]) == size_y)
                return write_tiff_Medipix<TiffT>(tiff, &data[0][0], size_x, size_y);
            else {
                std::cerr << "Error: specified size of Medipix tiff file {" << size_x << ", " << size_y << "} ";
                std::cerr << "and 2D array-type container {" << std::size(data) << ", " << std::size(data[0]) << "} differ" << std::endl;
                return 0;
            };
        }
        else {
            static_assert(std::is_integral_v<std::remove_cvref_t<decltype(data[0])>>, "Cannot write container with more than 2 dimensions to Medipix tiff file.");
            return 0;
        }
    }
    else {
        std::array<std::size_t, 2> dim = {size_x == 0 ? 512 : size_x, size_y == 0 ? 512 : size_y};
        char header[8];
        if (std::endian::native == std::endian::little)
            header[1] = header[0] = 'I';
        else
            header[1] = header[0] = 'M';
        reinterpret_cast<uint16_t&>(header[2]) = 42;
        reinterpret_cast<uint32_t&>(header[4]) = static_cast<uint32_t>(dim[0] * dim[1] * sizeof(TiffT) + 8);
        char tiff_IDF[78];
        reinterpret_cast<uint16_t&>(tiff_IDF[0]) = 6;
        char* IDF_entry = &tiff_IDF[2];
        auto set_IDF  = [&IDF_entry](uint16_t tag, uint16_t type, uint32_t val) {
            reinterpret_cast<uint16_t&>(IDF_entry[0]) = tag;
            reinterpret_cast<uint16_t&>(IDF_entry[2]) = type;
            reinterpret_cast<uint32_t&>(IDF_entry[4]) = 1;
            if (type == 1)
                reinterpret_cast<uint8_t&>(IDF_entry[8]) = val;
            else if (type == 3)
                reinterpret_cast<uint16_t&>(IDF_entry[8]) = val;
            else if (type == 4)
                reinterpret_cast<uint32_t&>(IDF_entry[8]) = val;
            IDF_entry += 12;
        };
        set_IDF(0x0100, 3, static_cast<uint32_t>(dim[0]));
        set_IDF(0x0101, 3, static_cast<uint32_t>(dim[1]));
        set_IDF(0x0102, 3, 8 * sizeof(TiffT));
        set_IDF(0x0103, 3, 1);
        set_IDF(0x0106, 3, 1);
        set_IDF(0x0111, 4, 8);
        tiff_IDF[77] = tiff_IDF[76] = tiff_IDF[75] = tiff_IDF[74] = 0;
        tiff.write(header, 8);
        if constexpr (sizeof(TiffT) == sizeof *data && std::is_integral_v<std::remove_cvref_t<decltype(*data)>>)
            tiff.write(reinterpret_cast<const char*>(&data[0]), dim[0] * dim[1] * sizeof(TiffT));
        else for (std::size_t i = 0; i != dim[0] * dim[1]; ++i) {
            TiffT tmp = data[i];
            tiff.write(reinterpret_cast<const char*>(&tmp), sizeof(TiffT));
        }
        tiff.write(tiff_IDF, 78);
        return dim[0] * dim[1];
    }
    
}


template <typename TiffT = uint16_t> requires std::is_same_v<uint8_t, TiffT> || std::is_same_v<uint16_t, TiffT> || std::is_same_v<uint32_t, TiffT>
std::size_t write_tiff_Medipix(std::filesystem::path path, auto const& data, std::size_t const size_x = 0, std::size_t const size_y = 0) {
    std::string extension = path.extension();
    for (auto& c : extension)
        c = std::tolower(c);
    if (extension != ".tiff" && extension != ".tif")
        path.replace_extension("tiff");
    std::ofstream tiff(path, std::ios::binary);
    return write_tiff_Medipix<TiffT>(tiff, data, size_x, size_y);
}
}
#endif /* Medipix_tiff_h */
