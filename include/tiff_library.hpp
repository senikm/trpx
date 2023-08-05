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

// THIS IS NOT A GENERAL TIFF LIBRARY! It does not handle binary, compressed or color TIFF files.
//
// Reads and writes baseline greyscale TIFF files containing one or more images.
// The images must have 4-, 8-, 16-, or 32-bits unsigned integer-valued pixels in little- and big-endian order.
// Allows inserting/appending and extracting unsigned 2-byte or 4-byte data into and out of any container type.
// Pixel values in a Grey_tif are readonly.
// Pixel values are always extracted from a Grey_tif as unsigned 4-byte integers, even when they are stored internally as 2-byte ints.
//
// Constructor:
//  Grey_tif();
//      Constructs an empty object without images.
// Member functions:
//  template <typename C> requires (requires (C& c) {std::begin(c), std::end(c), std::size(c);})
//  void push_back(C const& container, std::array<std::ptrdiff_t,2> const& dim)
//      Pushes a new image into the Grey_tif object. It must an image with either 16- or 32-bits
//      unsigned integers. The dimension of te image must be specified as a 2D array of long.
//  begin(), end(), cbegin(), cend(), rbegin(), crbegin(), rend(), crend(), size(), operator[];
//      STL const_iterators, const_reversed_iterators and operators for reading the pixels of
//      first (or only image) in the object. All are noexcept;
//  std::size_t const bits_per_pixel() const noexcept;
//      Returns the number of bit per pixel: 16 or 32, for images with 2-byte or 4-byte unsigned integral values.
//  std::array<std::size_t, 2> const dim() const noexcept;
//      returns the size in x and y, of the first (or only image) in the object.
//  unsigned char* data() noexcept;
//      A pointer to the first data item of the image. The data() member function allows direct acces
//      to the data without going through iterators and changing the data of a Grey_tif object.
//      NOTE: The byte order of the data is correct for the specified pixel size. So if pixel_size() == 2,
//      *reinterpret_cast<std::uint16_t>(Grey_tif.data()) returns the correct value of the first pixel.
//      And *reinterpret_cast<std::uint16_t>(Grey_tif.data() + 2) returns the correct value of the second
//      pixel. This is not necessarily the case if pixel_size() == 4! Then it depends on the native byte
//      order.
//      USE WITH CAUTION: the pointer provided by data() can be invalidated by a call to push_back().
// Member objects:
//  std::vector<> const& stack;
//      Stack of the images stored in the tif object. All the member functions of Grey_tif also apply to
//      its images in the TIFF stack. For instance, Grey_tif::stack[2][10] returns the 10th pixelvalue
//      of the 3rd image of the stack as a 4-byte unsigned int. Note that Grey_tif::begin() is equivalent
//      to Grey_tif.stack[0].begin().
// Non-member functions:
//  operator>>(std::istream, Grey_tif) and operator<<(std::istream, Grey_tif)
//      Reading and writing a Grey_tif object from and to a any stream (file). Upon input, byte-swapping
//      is performed if required. The TIFF files are always written to the output stream in the native byte
//      order (and have the appropriate TIFF header).
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
//        std::cout << "The pixels of the 3rd image have " << tif.stack[2].bits_per_pixel() / 8 << " bytes." << std::endl;
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
    auto rbegin()  const noexcept { return std::make_reverse_iterator(end()); }
    auto crbegin() const noexcept { return std::make_reverse_iterator(cend()); }
    auto rend()    const noexcept { return std::make_reverse_iterator(begin()); }
    auto crend()   const noexcept { return std::make_reverse_iterator(cbegin()); }
    unsigned char* data() noexcept {return d_stack[0].data();}

    std::size_t const bits_per_pixel() const { return stack[0].bits_per_pixel(); }
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
        using const_iterator = Iterator;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
 
        c_Raw_img(std::vector<unsigned char> const& tif,
                  std::uint32_t const index,
                  int const bits_per_pixel,
                  std::array<std::ptrdiff_t,2> const& dim,
                  bool native = true) noexcept :
        d_data(tif),
        d_index(index),
        d_bits_per_pixel(bits_per_pixel),
        d_dim(dim) {}
                            
        std::size_t const bits_per_pixel() const noexcept { return d_bits_per_pixel; }
        std::size_t const size() const noexcept { return dim()[0] * dim()[1]; }
        std::array<std::ptrdiff_t,2> const& dim() const noexcept { return d_dim; }
        Iterator begin()   const noexcept { return Iterator(*this);}
        Iterator end()     const noexcept { return begin() + size(); }
        Iterator cbegin()  const noexcept { return begin(); }
        Iterator cend()    const noexcept { return end(); }
        auto rbegin()  const noexcept { return std::make_reverse_iterator(end()); }
        auto crbegin() const noexcept { return std::make_reverse_iterator(cend()); }
        auto rend()    const noexcept { return std::make_reverse_iterator(begin()); }
        auto crend()   const noexcept { return std::make_reverse_iterator(cbegin()); }
        std::uint32_t operator[](std::ptrdiff_t offset) const { return begin()[offset]; }
        unsigned char* data() noexcept {return const_cast<unsigned char*>(d_data.data()) + d_index;}

    private:
        std::vector<unsigned char> const& d_data;
        std::uint32_t const d_index;
        int const d_bits_per_pixel;
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
                if (d_img.d_bits_per_pixel == 16)
                   return reinterpret_cast<std::uint16_t const&>(d_img.d_data[d_img.d_index + 2 * d_position]);
                else if (d_img.d_bits_per_pixel == 32)
                    return reinterpret_cast<std::uint32_t const&>(d_img.d_data[d_img.d_index + 4 * d_position]);
                else if (d_img.d_bits_per_pixel == 8)
                    return reinterpret_cast<std::uint8_t const&>(d_img.d_data[d_img.d_index + d_position]);
                else 
                    return reinterpret_cast<std::uint8_t const&>(d_img.d_data[d_img.d_index + d_position / 2]) >> ((d_position & 1) << 4);
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
        std::uint16_t& r = reinterpret_cast<std::uint16_t&>(*cursor);
        cursor += 2;
        return NATIVE ? r : r = (r << 8) | (r >> 8);
    }

    template <bool NATIVE>
    constexpr std::uint32_t f_int32(unsigned char* &cursor) {
        std::uint32_t& r = reinterpret_cast<std::uint32_t&>(*cursor);
        cursor += 4;
        return NATIVE ? r : (r << 24) | ((r & 0xff00) << 8) | ((r >> 8) & 0xff00) | (r >> 24);
    }

    template <bool NATIVE>
    constexpr std::uint64_t f_int64(unsigned char* &cursor) {
        std::uint64_t& r = reinterpret_cast<std::uint64_t&>(*cursor);
        cursor += 8;
        return NATIVE ? r : ((r << 56) | ((r & 0xff00) << 40) | ((r & 0xff0000) << 24) | ((r & 0xff000000) << 8) |
                             ((r >> 8) & 0xff000000) | ((r >> 24) & 0xff0000) | ((r >> 40) & 0xff00) | (r >> 56));
    }

    template <bool NATIVE = true>
    c_Raw_img const f_Raw_img(std::uint32_t& index) noexcept {
        std::array<std::ptrdiff_t,2> dim = {0,0};
        int bits_per_pixel = 0;
        unsigned char* cursor = d_data.data() + index;
        int const tag_count = f_int16<NATIVE>(cursor);
        std::int32_t rows_per_strip;
        std::vector<std::int32_t> strip_offsets(1);
        std::vector<std::int32_t> strip_byte_counts(1);
        for (int i = 0; i != tag_count; ++i) {
            uint16_t tag = f_int16<NATIVE>(cursor);
            uint16_t type = f_int16<NATIVE>(cursor);
            uint32_t count = f_int32<NATIVE>(cursor);
            uint32_t val(0);
            double double_val(0);
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
            else if (type == 5 || type == 10) {
                unsigned char* p = d_data.data() + f_int32<NATIVE>(cursor);
                double_val = f_int32<NATIVE>(p);
                double_val /= f_int32<NATIVE>(p);
            }
            else if (type == 11) {
                val = f_int32<NATIVE>(cursor);
                double_val = reinterpret_cast<float&>(val);
                val = 0;
            }
            else if (type == 12) {
                unsigned char* p = d_data.data() + f_int32<NATIVE>(cursor);
                std::uint64_t tmp = f_int64<NATIVE>(p);
                double_val = reinterpret_cast<double&>(tmp);
            }
            if (tag == 0x0100) dim[0] = val;
            else if (tag == 0x0101) dim[1] = val;
            else if (tag == 0x0102 && (val == 16 || val == 32)) bits_per_pixel = val;
            else if (tag == 0x0102 && val != 16 && val != 32)
                std::cerr << "Warning: Grey_tif can only read greyscale tiff files with 16- or 32-bit pixels" << std::endl;
            else if (tag == 0x0103 && val != 1)
                std::cerr << "Warning: Grey_tif cannot read compressed tiff files"<< std::endl;
            else if (tag == 0x0106 && val > 1)
                std::cerr << "Warning: Grey_tif cannot read colour tiff files"<< std::endl;
            else if ((tag == 0x0107 || tag == 0x0108 || tag == 0x0109 || tag == 0x010A) && val != 1)
                std::cerr << "Warning: Grey_tif cannot read black & white tiff files"<< std::endl;
            else if (tag == 0x0111 && count == 1) strip_offsets[0] = val;
            else if (tag == 0x0111 && count > 1) {
                strip_offsets.resize(count);
                unsigned char* p = d_data.data() + val;
                for (int i=0; i != count; ++i)
                    strip_offsets[i] = f_int32<NATIVE>(p);
            }
            else if (tag == 0x0115 && val != 1)
                std::cerr << "Warning: Grey_tif cannot read RGB colour tiff files"<< std::endl;
            else if (tag == 0x0116) rows_per_strip = val;
            else if (tag == 0x0117 && count == 1) strip_byte_counts[0] = val;
            else if (tag == 0x0117 && count > 1) {
                strip_byte_counts.resize(count);
                unsigned char* p = d_data.data() + val;
                for (int i=0; i != count; ++i)
                    strip_byte_counts[i] = f_int32<NATIVE>(p);
            };
        }
        bool contiguous = strip_offsets.size() == 1;
        for (int i=0; !contiguous && i != strip_offsets.size() - 1; ++i)
            contiguous = strip_byte_counts[i] == strip_offsets[i + 1] - strip_offsets[i];
        if (!contiguous) {
            std::cerr << "Warning: Grey_tif cannot read tiff files with non-consecutive strips" << std::endl;
            std::cerr << "         most likely the tiff file is corrupted" << std::endl;
        }
        d_last_ifd_offset = static_cast<std::uint32_t>(cursor - d_data.data());
        index = f_int32<NATIVE>(cursor);
        if (!NATIVE && bits_per_pixel >= 16) {
            unsigned char* begin = d_data.data() + strip_offsets[0];
            unsigned char* end = begin + dim[0] * dim[1] * bits_per_pixel / 8;
            if (bits_per_pixel == 16)
                while (begin != end) f_int16<false>(begin);
            else
                while (begin != end) f_int32<false>(begin);
        }
        return c_Raw_img(d_data, strip_offsets[0], bits_per_pixel, dim);
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

// Reads image data of the first image in a tif file into a container, and returns the dimensions
// of this image, as read from the tif file. If the container is smaller than the tif file, the data
// are not transferred into the container. If there is an error reading the file, a message on std::err
// is written and {0, 0} is returned.
template <typename C> requires (requires (C& c) {std::begin(c), std::end(c), std::size(c);})
std::array<std::ptrdiff_t, 2> read_tiff_Medipix(std::filesystem::path const& path, C& container) {
    Grey_tif tif;
    std::string extension = path.extension();
    for (auto& c : extension)
        c = std::tolower(c);
    if (!is_regular_file(path))
        std::cerr << "Failed to open input file " << path << std::endl;
    if (extension == ".tiff" || extension == ".tif") {
        std::ifstream is(path, std::ios::binary);
        if (!is.is_open())
            std::cerr << "Failed to open input file " << path << std::endl;
        else
            is >> tif;
        if (tif.stack[0].size() <= container.size())
            std::copy(tif.stack[0].begin(), tif.stack[0].end(), container.begin());
        else
            std::cerr << "Image too large for container; " << path << " has " << tif.stack[0].dim()[0] << "*"
            << tif.stack[0].dim()[1] << " pixels. The size of the container is: " << container.size() << std::endl;
        return tif.stack[0].dim();
    }
    return {0,0};
}
} // namespace jpa
#endif /* Grey_tif_h */
