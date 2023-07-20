//
//  Grey_tif.h
//  Grey_tif
//
//  Created by Jan Pieter Abrahams on 23.05.23.
//

#ifndef Grey_tif_h
#define Grey_tif_h


#include <fstream>
#include <filesystem>
#include <bit>
#include <array>
#include <span>
#include <cassert>

// THIS IS NOT A GENERAL TIFF LIBRARY!
//
// Reads and writes greyscale baseline TIFF files and stacks with 2-byte or 4-byte unsigned integer-valued pixels in little- and big-endian order.
// Requires the images to be encoded in the TIFF in a single stripe.
// Allows pushing in and extracting unsigned 2- byte or 4-byte data into and out of any container type.
// Pixel values in a Grey_tif are readonly.
// Pixel values are always extracted from a Grey_tif as unsigned 4-byte integers, even when they are stored internally as 2-byte ints.
//
// Constructor:
//  Grey_tif();
//      Constructs an empty object without images.
// Member functions:
//  begin(), end(), cbegin(), cend(), size(), operator[];
//      STL const iterators and operators for reading the pixels of first (or only image) in the object.
//  std::size_t const sizeof_pixel() const;
//      Returns 2 or 4, for images with 2-byte or 4 byte unsigned integral values.
//  std::array<std::size_t, 2> const dim() const;
//      returns the size in x and y, of the first (or only image) in the object.
// Member objects:
//  std::vector<> const& stack;
//      Stack of the images stored in the tif object. All the member functions of Grey_tif also apply to
//      its images on the stack. For instance, Grey_tif::stack[2][10] returns the 10th pixelvalue
//      of the 3rd image of the stack as a 4-byte unsigned int.
// Non-member functions:
//  operator>>(std::istream, Grey_tif) and operator<<(std::istream, Grey_tif)
//      Reading and writing a Grey_tif object from and to a any stream (file). Upon input, byte-swapping
//      is performed if required. The TIFF files are always written to the output stream in the native byte
//      order (and have the appropriate TIFF header).
//
//
// Example:
//
//    #include <iostream>
//    #include <vector>
//    #include "Grey_tif.hpp"
//
//    int main(int argc, const char * argv[]) {
//        using namespace jpa;
//        Grey_tif tif; // Construct a tif object.
//
//        std::filesystem::path input = "/Users/abrahams/Dropbox/develop/jpa/Grey_tif/Grey_tif/infile.tif";
//        std::filesystem::path output = "/Users/abrahams/Dropbox/develop/jpa/Grey_tif/Grey_tif/outfile.tif";
//        std::ifstream istream(input, std::ios::binary);
//        istream >> tif; // read in data from dis into the tif object
//
//        std::vector<std::uint16_t> img0({42,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});
//        tif.push_back(img0, {4,4}); // apend a (very small) image, creating a tiff stack.
//
//        std::vector<std::uint32_t> img1({4242,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});
//        tif.push_back(img1, {4,4}); // you can also append 4-byte image data.
//
//        // The images of the stacks can be accessed using an STL vector, that is a member of the Grey_tif object.
//        std::cout << "The tif stack now contains "<< tif.stack.size() << " images."<< std::endl;
//
//        // All the STL functions and C++ features that work on STL containers also are valid for images in a tif object:
//        for (auto val : tif.stack[1])
//            std::cout << val << " ";
//        std::cout << std::endl;
//
//        // Besides the STL iterators like begin() and end(), images in a tif object have two additional member functions determine its dimensions:
//        std::cout << "The dimensions of the 2nd image are: " << tif.stack[1].dim()[0] << " by "<< tif.stack[1].dim()[0] << " pixels." << std::endl;
//        std::cout << "The pixels of the 3rd image have " << tif.stack[2].sizeof_pixel() << " bytes." << std::endl;
//
//        std::vector<std::uint16_t> img3(512*512);
//        std::vector<std::uint16_t> img4(512*512);
//
//        // The first image (or only) of a stack can be extracted without having to specify its stack number.
//        std::copy(tif.begin(), tif.end(), img3.begin());
//        std::copy(tif.stack[0].begin(), tif.stack[0].end(), img4.begin());
//        assert (img3 == img4);
//
//        // A tif object can be written to disk using the overloaded <<() operator.
//        std::ofstream ostream(output, std::ios::binary);
//        ostream << tif;
//        return 0;
//    }
//
// Produces the following output:
//    The tif stack now contains 3 images.
//    42 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
//    4242 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
//    The dimensions of the 2nd image are: 4 by 4 pixels.
//    The pixels of the 3rd image have 4 bytes.

namespace jpa {

class Grey_tif {
    class c_Raw_img;
    
public:
    Grey_tif() {
        d_data = std::vector<unsigned char>(8, 0x00);
        d_data[0] = d_data[1] = (std::endian::native == std::endian::little) ? 'I' : 'M';
        reinterpret_cast<std::uint16_t&>(d_data[2]) = 42;
        d_last_ifd_offset = 4;
    }
        
    std::vector<c_Raw_img> const& stack = d_stack;
    
    std::uint32_t operator[](std::ptrdiff_t offset) const { return stack[0][offset]; }
    auto begin() const {return stack[0].begin(); };
    auto cbegin() const {return stack[0].cbegin(); };
    auto end() const {return stack[0].end(); };
    auto cend() const {return stack[0].cend(); };
    std::size_t const sizeof_pixel() const { return stack[0].sizeof_pixel(); }
    std::size_t const size() const { return stack[0].size(); }
    std::array<std::ptrdiff_t,2> const dim() const { return stack[0].dim(); }

    template <typename C> requires (requires (C& c) {std::begin(c), std::end(c), std::size(c);})
    void push_back(C const& container, std::array<std::ptrdiff_t,2> const& dim) {
        using T = typename C::value_type;
        static_assert (std::is_same_v<T, std::uint16_t> || std::is_same_v<T, std::uint32_t>);
        assert(container.size() == dim[0] * dim[1]);
        assert((d_data.size() + dim[0] * dim[1] + 6 * 12 + 4) < std::numeric_limits<uint32_t>::max());
        std::uint32_t index = static_cast<uint32_t>(d_data.size());
        std::uint32_t data_start = index;
        d_data.resize(d_data.size() + dim[0] * dim[1] * sizeof(T) + 6 * 12 + 6);
        for (auto const val : container) {
            reinterpret_cast<T&>(d_data[index]) = static_cast<T>(val);
            index += sizeof(T);
        }
        if ((d_data.size() & 1) == 1) {
            d_data.push_back(0);
            ++index;
        }
        reinterpret_cast<std::uint32_t&>(d_data[d_last_ifd_offset]) = index;
        reinterpret_cast<std::uint16_t&>(d_data[index]) = 6;
        index += 2;
        f_set_ifd(index, 0x0100, 3, static_cast<uint32_t>(dim[0]));
        f_set_ifd(index, 0x0101, 3, static_cast<uint32_t>(dim[1]));
        f_set_ifd(index, 0x0102, 3, 8 * sizeof(T));
        f_set_ifd(index, 0x0103, 3, 1);
        f_set_ifd(index, 0x0106, 3, 1);
        f_set_ifd(index, 0x0111, 4, data_start);
        d_data[index] = d_data[index + 1] = d_data[index + 2] = d_data[index + 3] = 0;
        d_last_ifd_offset = static_cast<uint32_t>(index);
        d_stack.push_back(f_Raw_img(index -= 6 * 12 + 2));
    }
    
    friend std::ostream& operator<<(std::ostream& os, Grey_tif& tif) {
        return os.write(reinterpret_cast<char*>(tif.d_data.data()), tif.d_data.size());
    }

    friend std::istream& operator>>(std::istream& is, Grey_tif& tif) {
        // Get the file size
        is.seekg(0, std::ios::end);
        std::streampos fileSize = is.tellg();
        is.seekg(0, std::ios::beg);
        std::vector<unsigned char>& data = tif.d_data;
        data.resize(fileSize);
        if (!is.read(reinterpret_cast<char*>(data.data()), fileSize) ||
            (data[0] != 'I' && data[0] != 'M') || data[0] != data[1] || (data[0] == 'I' ? data[2] : data[3]) != 42)
            is.setstate(std::ios::failbit);
        else {
            if (!((data[0] == 'I' || data[0] == 'M')) && data[0] == data[1] && ((data[0] == 'I' && data[2] == 42 && data[3] == 0) || (data[0] == 'M' && data[2] == 0 && data[3] == 42)))
                std::cerr << "Error: not a tiff file" << std::endl;
            bool native = data[0] == 'I' && std::endian::native == std::endian::little;
            if (!native)  data[0] = data[1] = (data[0] == 'M' ? 'I': 'M');
            reinterpret_cast<std::uint16_t&>(data[2]) = 42;
            std::uint32_t index = reinterpret_cast<std::uint32_t&>(data[4]);
            if (!native) {
                index = (index << 24) | ((index & 0xff00) << 8) | ((index >> 8) & 0xff00) | (index >> 24);
                reinterpret_cast<std::uint32_t&>(data[4]) = index;
            }
            if (native)
                while (index != 0) {
                    tif.d_stack.push_back(tif.f_Raw_img<true>(index));
                    index = reinterpret_cast<std::uint32_t&>(data[tif.d_last_ifd_offset]);
                }
            else
                while (index != 0) {
                    tif.d_stack.push_back(tif.f_Raw_img<false>(index));
                    index = reinterpret_cast<std::uint32_t&>(data[tif.d_last_ifd_offset]);
                }
        }
        return is;
    }
private:
    class c_Raw_img {
        class Iterator;
    public:
        using iterator = Iterator;
        using const_iterator = Iterator;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
 
        c_Raw_img(std::vector<unsigned char> const& tif, std::uint32_t const index, int const pixel_depth, std::array<std::ptrdiff_t,2> const& dim, bool native = true) :
        d_data(tif),
        d_index(index),
        d_pixel_depth(pixel_depth),
        d_dim(dim) {
            if (!native && sizeof_pixel() == 2) {
                auto begin = reinterpret_cast<std::uint16_t const*>(d_data.data() + d_index);
                for (auto i = begin; i != begin + size(); ++i)
                    const_cast<std::uint16_t&>(*i) = (*i << 8) | (*i >> 8);
            }
            if (!native && sizeof_pixel() == 4) {
                auto begin = reinterpret_cast<std::uint32_t const*>(d_data.data() + d_index);
                for (auto i = begin; i != begin + size(); ++i)
                    const_cast<std::uint32_t&>(*i) = (*i << 24) | ((*i & 0xff00) << 8) | ((*i >> 8) & 0xff00) | (*i >> 24);
            }
        }
                            
        std::size_t const sizeof_pixel() const { return d_pixel_depth; }
        std::size_t const size() const { return dim()[0] * dim()[1]; }
        std::array<std::ptrdiff_t,2> const dim() const { return d_dim; }
        std::uint32_t operator[](std::ptrdiff_t offset) const { return begin()[offset]; }
        Iterator begin() const { return Iterator(*this);}
        Iterator end() const { return begin() + size(); }
        Iterator cbegin() const { return begin(); }
        Iterator cend() const { return end(); }

    private:
        std::vector<unsigned char> const& d_data;
        std::uint32_t const d_index;
        int const d_pixel_depth;
        std::array<std::ptrdiff_t,2> const d_dim;
        
        class Iterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = std::uint32_t;
            using difference_type = std::ptrdiff_t;
            using pointer = const value_type*;
            using reference = const value_type&;

            Iterator(c_Raw_img const& img) : d_img(img) {}
            
             value_type operator*() const {
                if (d_img.d_pixel_depth == 2)
                   return reinterpret_cast<std::uint16_t const&>(d_img.d_data[d_img.d_index + 2 * d_position]);
                else
                    return reinterpret_cast<std::uint32_t const&>(d_img.d_data[d_img.d_index + 4 * d_position]);
            }

            value_type operator[](difference_type offset) const { return *(*this + offset); }
            Iterator& operator++() { ++d_position; return *this; }
            Iterator& operator--() { --d_position; return *this; }
            Iterator operator++(int) { Iterator temp(*this); ++d_position; return temp; }
            Iterator operator--(int) { Iterator temp(*this); --d_position; return temp; }
            Iterator& operator+=(difference_type offset) { d_position += offset; return *this; }
            Iterator& operator-=(difference_type offset) { d_position -= offset; return *this; }
            auto operator<=>(const Iterator& other) const { assert(&d_img == &other.d_img); return d_position <=> other.d_position; }
            auto operator!=(const Iterator& other) const { assert(&d_img == &other.d_img); return d_position != other.d_position; }
            Iterator operator+(difference_type offset) const { Iterator result = *this; return result += offset; }
            Iterator operator-(difference_type offset) const { Iterator result = *this; return result -= offset; }
            difference_type operator-(Iterator const& other) const { return d_position - other.d_position; }
        private:
            c_Raw_img const& d_img;
            std::size_t d_position = 0;
        };
     };

    std::uint32_t d_last_ifd_offset = 0;
    std::vector<unsigned char> d_data;
    std::vector<c_Raw_img> d_stack;

    std::uint8_t f_int8(unsigned char* &cursor)  { return *cursor++; }
    
    template <bool NATIVE>
    constexpr std::uint16_t f_int16(unsigned char* &cursor) {
        std::uint16_t& r = reinterpret_cast<uint16_t&>(*cursor);
        cursor += 2;
        return NATIVE ? r : r = (r << 8) | (r >> 8);
    }

    template <bool NATIVE>
    constexpr std::uint32_t f_int32(unsigned char* &cursor) {
        std::uint32_t& r = reinterpret_cast<uint32_t&>(*cursor);
        cursor += 4;
        return NATIVE ? r : (r << 24) | ((r & 0xff00) << 8) | ((r >> 8) & 0xff00) | (r >> 24);
    }

    template <bool NATIVE = true>
    c_Raw_img const f_Raw_img(std::uint32_t& index) noexcept {
        std::array<std::ptrdiff_t,2> dim = {0,0};
        int pixel_depth = 0;
        std::int32_t location = 0;
        unsigned char* cursor = d_data.data() + index;
        int const tag_count = f_int16<NATIVE>(cursor);
        for (int i = 0; i != tag_count; ++i) {
            uint16_t tag = f_int16<NATIVE>(cursor);
            uint16_t type = f_int16<NATIVE>(cursor);
            uint32_t count = f_int32<NATIVE>(cursor);
            if (count != 1)
                std::cerr << "Tiff has multiple values for tag " << tag << " which the current version of Grey_tif cannot read" << std::endl;
            uint32_t val(0);
            if ((type == 1) || (type == 2) || (type == 6) || (type == 7)) {
                val = f_int8(cursor);
                cursor += 3;
            }
            else if ((type == 3) || (type == 8)) {
                val = f_int16<NATIVE>(cursor);
                cursor += 2;
            }
            else if ((type == 4) || (type == 9))
                val = f_int32<NATIVE>(cursor);
            else
                std::cerr << "Tiff tag " << tag <<
                " has a type requiring more than 4 bytes, which the current version of Grey_tif cannot read" << std::endl;
            if (tag == 0x0100) dim[0] = val;
            else if (tag == 0x0101) dim[1] = val;
            else if (tag == 0x0102) pixel_depth = val / 8;
            else if (tag == 0x0103 && val != 1)
                std::cerr << "The current version of Grey_tif cannot read compressed tiff files"<< std::endl;
            else if (tag == 0x0111) location = val;
            else if (tag == 0x0116 && val != 1)
                std::cerr << "The current version of Grey_tif cannot read tiff files with images divided into strips"<< std::endl;
        }
        d_last_ifd_offset = static_cast<std::uint32_t>(cursor - d_data.data());
        index = f_int32<NATIVE>(cursor);
        return c_Raw_img(d_data, location, pixel_depth, dim);
    };
    
    void f_set_ifd(std::uint32_t& index, uint16_t tag, uint16_t type, uint32_t val) {
        reinterpret_cast<uint16_t&>(d_data[index]) = tag;
        reinterpret_cast<uint16_t&>(d_data[index + 2]) = type;
        reinterpret_cast<uint32_t&>(d_data[index + 4]) = 1;
        if (type == 1)
            reinterpret_cast<uint8_t&>(d_data[index + 8]) = val;
        else if (type == 3)
            reinterpret_cast<uint16_t&>(d_data[index + 8]) = val;
        else if (type == 4)
            reinterpret_cast<uint32_t&>(d_data[index + 8]) = val;
        index += 12;
    };
};

template <typename C> requires (requires (C& c) {std::begin(c), std::end(c), std::size(c);})
std::array<std::ptrdiff_t, 2> read_tiff_Medipix(std::filesystem::path const& path, C& container) {
    Grey_tif tif;
    std::string extension = path.extension();
    for (auto& c : extension)
        c = std::tolower(c);
    if (is_regular_file(path) && (extension == ".tiff" || extension == ".tif")) {
        std::ifstream is(path, std::ios::binary);
        if (!is.is_open())
            std::cerr << "Failed to open input file " << path << std::endl;
        else
            is >> tif;
    }
    assert(tif.stack[0].size() == container.size());
    std::copy(tif.stack[0].begin(), tif.stack[0].end(), container.begin());
    return tif.stack[0].dim();
}
} // namespace jpa
#endif /* Grey_tif_h */
