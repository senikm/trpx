//
//  Terse.hpp
//  Terse
//
//  Created by Jan Pieter Abrahams on 30/04/2019.
//  Copyright © 2019 Jan Pieter Abrahams. All rights reserved.
//

#ifndef Terse_h
#define Terse_h

#include <fstream>
#include <vector>
#include <cassert>
#include "Bit_pointer.hpp"
#include "XML_element.hpp"

// Terse<T> allows efficient and fast compression of integral diffraction data and other integral greyscale
// data into a Terse object that can be decoded by the member function Terse<T>::prolix(iterator). The
// prolix(iterator) member function decompresses the data starting at the location defined by 'iterator'
// (which can also be a pointer). A Terse object is constructed by supplying it with uncompressed data or a
// stream that contains Terse data.
//
// Unpacking is only possible into values that have at least the same size as the original values (so
// unpacking uint16_t into uint8_t is not possible). Also it is not possible to unpack signed integral data
// into unsigned integral data (so unpacking int16_t into uint16_t is not possible, but unpacking uint16_t
// into int16_t is allowed). Otherwise all is allowed, including unpacking into floats and doubles. When
// unpacking unsigned integral values into signed integral values, be aware that an overload in the unsigned
// data (all bits are equal to 1), will result in a '-1' value in the unpacked signed integral data.
//
// A Terse object can be written or appended to a file using the overloaded << operator. The resulting file
// is independent of the endian-nes of the machine: both big- and small-endian machines produce identical
// files, making data transfer optimally transparent. Terse files can be read directly into a Terse object.
//
// If the data are known to be positive, compressing as unsigned will yield a tighter compression.
//
// The algorithm is a run-length encoding type. Each data block (by default 8 integral values in the
// constructor, but this can be changed) is preceded by one or more data block header bits. The values in the
// data block are stripped of their most significant bits, provided they are all zero (for unsigned values),
// or either all zero or one (for signed values). In the latter case, the sign bit is maintained. So, for a
// block size of 3 with values 3, 4, 2, the encoded bits would be: 011 (denoting 3) 100 (denoting 4) 010
// (denoting 2). So 011100010 would be pushed into the Terse object. In case of signed values -3, 4, 2, the
// encoded bits would be 1011 (denoting -3) 0100 (denoting +4) 0010 (denoting +2), resulting in a data block
// 101101000010. So if the values that need to be encoded are all positive or zero, they should be encoded as
// unsigned for optimal compression: it saves 1 bit per encoded value.
//
// The header bits define how the values are encoded. They have the following following structure:
// bit 1:    If the first bit of the block header is set, then there are no more bits in the block header,
//           and the parameters of the previous block header are used.
// bit 2-4:  The first header bit is 0. The three bits 2 to 4 define how many bits are used per value of the
//           encoded block. If bits 2 to 4 are all set, 7 or more bits per value are required and the
//           header is expanded by a further 2 bits.
// bit 5-6:  The first 4 header bits are 0111. The number encoded bits 5 and 6 is added to 7 and this defines
//           how many bits are used to encode the block. So if bits 5 & 6 are 00 then 7 bits are used, 01 means
//           8 bits, 10 means 9 bits and 11 means at least 10 bits. If bits 5 & 6 are both set, the header is
//           expanded by another 6 bits.
// bit 7-12: The first 6 header bits are 011111. The number encoded by bits 7 to 12 is added to decimal 10 and
//           this defines how many bits are used to encode the block. So if bits 7 to 12 are 000000, then the
//           number of bits per value in the data block is decimal 10. If bits 7 to 12 are 110110, then the
//           number of bits per value in the data block is 10 + 54 = 64.
//
// Constructors:
//  Terse(std::ifstream& istream)
//      Reads in a Terse object that has been written to a file by the overloaded Terse output operator '<<'.
//  Terse(container_type const& data)
//      Creates a Terse object from data (which can be a std::vector, Lattice, etc.). Only containers of
//      integral types are allowed.
//  Terse(iterator begin, std::size_t size)
//      Creates a Terse object given a starting iterator or pointer and the number of elements that need to be
//      encoded.
//
// Member functions:
//  std::size_t size()
//      Returns the number of encoded elements.
//  bool is_signed()
//      Returns true if the encoded data are signed, false if unsigned. Signed data cannot be decompressed into
//      unsigned data.
//  bits_per_val()
//      Returns the maximum number of bits per element that can be expected. So for uncompressed uint_16 type
//      data, bits_per_val() returns 16. Terse data cannot be decompressed into a container type with elements
//      that are smaller in bits than bits_per_val().
//  terse_size()
//      Returns the number of bytes used for encoding the Terse data.
//  prolix(iterator begin)
//      Unpacks the Terse data starting from the location defined by 'begin'. Terse integral signed data cannot be
//      unpacked into integral unsigned data. Terse data cannot be decompressed into elements that are smaller
//      in bits than bits_per_val(), but can be decompressed into larger values. Terse data can always be unpacked
//      into signed intergral, double and float data and will have the correct sign (with one exception: an
//      unsigned overflowed - all 1's - value will be unpacked as -1 signed value. As all other values are positive
//      in this case, such a situation is easy to recognise).
//
// Operator overloads:
//  Streamtype& operator<< (Streamtype &ostream, Terse const& data)
//      Writes Terse data to 'ostream'. The Terse data are preceded by an XML element containing the parameters
//      that are required for constructing a Terse object from the stream. Data are written as a byte stream
//      and are therefore independent of endian-ness. A small-endian memory lay-out produces the a Terse file
//      that is identical to a big-endian machine.
//
//
// Example:
//
//    std::vector<int> numbers(1000);            // Uncompressed data location
//    std::iota(numbers.begin(), numbers.end(), -500);   // Fill with numbers -500, -499, ..., 499
//    Terse compressed(numbers);                      // Compress the data to less than 30% of memory
//    std::cout << "compression rate " << float(compressed.terse_size()) / (numbers.size() * sizeof(unsigned)) << std::endl;
//    std::ofstream outfile("junk.terse");
//    outfile << compressed;                          // Write Terse data to disk
//    std::ifstream infile("junk.terse");
//    Terse from_file(infile);                        // Read it back in again
//    std::vector<int> uncompressed(1000);
//    from_file.prolix(uncompressed.begin());         // Decompress the data...
//    for (int i=0; i != 5; ++i)
//    std::cout << uncompressed[i] << std::endl;
//    for (int i=995; i != 1000; ++i)
//    std::cout << uncompressed[i] << std::endl;
//
// Produces as output:
//
//    compression rate 0.29
//    -500
//    -499
//    -498
//    -497
//    -496
//    495
//    496
//    497
//    498
//    499

template <typename T=uint64_t>
class Terse {
public:
    
    template <typename Container>
    Terse(Container const& data) : Terse(data.begin(), data.size()) {};
    
    template <typename Iterator>
    Terse(Iterator const data, size_t const size, unsigned int const block=12) :
    d_prolix_bits(sizeof(decltype(*data)) * 8),
    d_signed(std::is_signed_v<typename std::iterator_traits<Iterator>::value_type>),
    d_block(block),
    d_size(size),
    d_terse_data(_compress(data)) {}
    
    Terse(std::ifstream& istream) : Terse(istream, XML_element(istream, "Terse")) {};
    
    template <typename Iterator>
    void prolix(Iterator begin) {
        assert(d_prolix_bits <= (8 * sizeof(decltype(begin[0]))));
        assert(d_signed == std::is_signed_v<typename std::iterator_traits<Iterator>::value_type>);
        Bit_pointer<const T*> bitp(d_terse_data.data());
        uint8_t significant_bits = 0;
        for (size_t from = 0; from < size(); from += d_block) {
            auto const to = std::min(size(), from + d_block);
            if (*bitp++ == 0) {
                significant_bits = Bit_range<const T*>(bitp,3);
                bitp += 3;
                if (significant_bits == 7) {
                    significant_bits += uint8_t(Bit_range<const T*>(bitp, 2));
                    bitp += 2;
                    if (significant_bits == 10) {
                        significant_bits += uint8_t(Bit_range<const T*>(bitp, 6));
                        bitp += 6;
                    }
                }
            }
            if (significant_bits == 0)
                std::fill(begin + from, begin + to, 0);
            else {
                Bit_range<const T*> bitr(bitp, significant_bits);
                if constexpr (std::is_integral<typename std::iterator_traits<Iterator>::value_type>::value)
                    bitr.get_range(begin + from, begin + to);
                else if (!is_signed())
                    for (auto i = from; i < to; ++i, bitr.next())
                        begin[i] = double(uint64_t(bitr));
                else for (auto i = from; i < to; ++i, bitr.next())
                    begin[i] = double(int64_t(bitr));
                bitp = bitr.begin();
            }
        }
    }

    std::size_t const size() const {return d_size;}
    bool const is_signed() const {return d_signed;}
    unsigned const bits_per_val() const {return d_prolix_bits;}
    std::size_t const terse_size() const {return d_terse_data.size() * sizeof(T);}
    
    template <typename Streamtype>
    friend Streamtype& operator<< (Streamtype &ostream, Terse const& data){
        ostream << "<Terse prolix_bits=\"" << data.d_prolix_bits << "\" ";
        ostream << "signed=\"" << data.d_signed << "\" ";
        ostream << "block=\"" << data.d_block << "\" ";
        ostream << "memory_size=\"" << data.d_terse_data.size() * sizeof(T) << "\" ";
        ostream << "number_of_values=\"" << data.size() << "\"/>";
        if constexpr (std::is_same_v<T, uint8_t>)
            ostream.write((char*)&data.d_terse_data[0], data.d_terse_data.size());
        else {
            std::vector<uint8_t> buffer;
            for (auto val : data.d_terse_data)
                for (int i = 0; i != sizeof(T); ++i, val >>= 8)
                    buffer.push_back(uint8_t(val)) ;
            ostream.write((char*)&buffer[0], buffer.size());
        }
        ostream.flush();
        return ostream;
    };
    
private:
    unsigned const d_prolix_bits;
    bool const d_signed;
    unsigned const d_block;
    std::size_t const d_size;
    std::vector<T> const d_terse_data;
    
    Terse(std::ifstream& istream, XML_element const& xmle) :
    d_prolix_bits(unsigned(std::stoul(xmle.attribute("prolix_bits")))),
    d_signed(std::stoul(xmle.attribute("signed"))),
    d_block(int(std::stoul(xmle.attribute("block")))),
    d_size(std::stoull(xmle.attribute("number_of_values"))),
    d_terse_data([&]() {
        std::vector<uint8_t> buffer(std::stold(xmle.attribute("memory_size")), 0);
        istream.read((char*)&buffer[0], buffer.size());
        if constexpr (std::is_same_v<T, uint8_t>)
            return buffer;
        else {
            std::vector<T> data(std::ceil(std::stold(xmle.attribute("memory_size")) / sizeof(T)), 0);
            for (std::size_t j = 0; auto& val : data)
                for (int i = 0; i < sizeof(T); ++i)
                    val |= T(buffer[j++]) << 8*i;
            return data;
        }
    } ())
    {};

    template <typename Iterator>
    std::vector<T> const _compress(Iterator data) {
        std::vector<T> terse_data(std::ceil(d_size * (sizeof(decltype(*data)) + (long double)(12.0) / (d_block * 8)) / sizeof(T)), 0);
        Bit_pointer bitp (terse_data.data());
        int prevbits = 0;
        for (size_t from = 0; from < d_size; from += d_block) {
            auto const to = std::min(d_size, from + d_block);
            typename std::iterator_traits<Iterator>::value_type setbits(0);
            auto p = data;
            for (auto i = from; i != to; ++i, ++p)
                if constexpr (std::is_unsigned_v<decltype(setbits)>)
                    setbits |= *p;
                else if constexpr (std::is_signed_v<decltype(setbits)>)
                    setbits |= std::abs(*p);
            unsigned significant_bits = Operator::highest_set_bit(setbits);
            if (prevbits == significant_bits) {
                (*bitp).set();
                ++bitp;
            }
            else {
                if (significant_bits < 7) {
                    Bit_range<T*>(++bitp, 3) |= significant_bits;
                    bitp += 3;
                }
                else if (significant_bits < 10) {
                    Bit_range<T*>(++bitp, 5) |= 0b111 + ((significant_bits - 7) << 3);
                    bitp += 5;
                }
                else {
                    Bit_range<T*>(++bitp, 11) |= 0b11111 + ((significant_bits - 10) << 5);
                    bitp += 11;
                }
                prevbits = significant_bits;
            }
            if (significant_bits != 0) {
                Bit_range<T*> r(bitp, significant_bits);
                r.append_range(data, data + (to - from));
                data += (to - from);
                bitp = r.begin();
            }
            else if constexpr (std::is_same_v<std::random_access_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>)
                data += d_block;
            else
                for (int i = 0; i != d_block; ++i, ++data);
        }
        terse_data.resize(1 + (bitp - terse_data.data()) / (sizeof(T) * 8));
        terse_data.shrink_to_fit();
        return terse_data;
    }
};


#endif /* Terse_h */
