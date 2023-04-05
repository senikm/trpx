//
//  Bit_pointer.hpp
//  Bit_pointer
//
//  Created by Jan Pieter Abrahams on 15/04/2019.
//  Copyright © 2019 Jan Pieter Abrahams. All rights reserved.
//

#ifndef Bit_pointer_h
#define Bit_pointer_h

//#include <vector>
//#include <type_traits>
#include "Operators.hpp"

// Bit<T>, Bit_pointer<T> and Bit_range<T> (where T is an iterator or pointer referring to an integral
// type), are three classes that provide an alternative to std::vector<bool>, std::bitset and
// boost::bitset. They allow bit-wise views of a container<*T>.
//
// Bit<T> is a reference to a bit in an object of type *T. It allows retrieving and manipulating the
// referenced bit. It has the following member functions:
//
//
//  operator bool()
//      Returns the value of the referenced bit by implicit typecasting to bool.
//  bool const operator =(bool const val)
//      Sets the referenced bit to val.
//  bool const operator =(Bit& const other)
//      Sets the referenced bit to the value of other.
//  Bit_pointer<Iter> operator &()
//      Returns a Bit_pointer<Iter> to the referenced bit.
//  void set()
//      Sets the referenced bit to 1 / 'true'.
//  void reset()
//      Sets the referenced bit to 0 / 'false'.
//  void flip()
//      Flips the referenced bit to from 1 to 0 or vice versa.
//  void swap(Bit_reference& other)
//      Swaps the bit referenced to by *this with the bit referenced to by other.
//  Bit<Iter>& operator &=(bool b) const noexcept
//  Bit<Iter>& operator |=(bool b) const noexcept
//  Bit<Iter>& operator ^=(bool b) const noexcept
//      And, or and xor assignment operators.
//
//
// Bit_pointer<T> is a pointer to a Bit<T> and supports all pointer operators. It is also an C++17 iterator
// and therefor compatible with the STL. If a pointer operator causes a Bit_pointer to point beyond the
// size of *T, it moves to another object as defined by the pointer arithmatic of container<*T> and the
// native byte order of *T (which is determined by the endedness of the hardware).
//
// Bit_pointer<T> assumes native byte-order, allowing easy and transparent data exchange between
// little-endian and big-endian hardware, by translating a multi-byte data type to a Bit_range<char>,
// transferring to an alternative hardware and then translating back to the original data type.
// Thus, there is no need to encode byte order during data transit, only the data type from which it
// was translated. Sending and receiving hardware can therefore both be byte-order naive.
// Bit_pointer<T> has the following member functions:
//
//  Bit_pointer<Iter>()
//      Default constructor of unspecified Bit_pointer<Iter> object.
//  Bit_pointer<Iter>(Iter offset)
//      Constructor of a Bit_pointer<Iter> object pointing to the first bit of *offset.
//  Bit_pointer<Iter>(Iter offset, std::ptrdiff_t n)
//      Constructor of a Bit_pointer<Iter> object pointing to the 'n'-th bit of *offset. If 'n' is
//      negative or larger than sizeof(*T), a bit in a preceeding / succeeding object is pointed to.
//  Bit_pointer<Iter>& operator =(Bit_pointer<Iter>) or =(Iter)
//      Assignment operators.
//  ++(); ()++; --(); ()--; += std::ptrdiff_t; -= std::ptrdiff_t; + std::size_t; - std::ptrdiff_t;
//      - Bit_pointer<Iter>; < ; <= ; > ; >= ; == ; !=
//      All the usual pointer arithmatic operators are allowed and return the expected values (i.e.
//  (references to) Bit_pointer<Iter> or std::ptrdiff_t
//
// Bit_range<T> defines a view of a range of Bit<T> in a container<*T>, providing an interface with the
// STL. It also allows packing and unpacking integral types into and out of a Bit_range<T>. It has the
// following member functions:
//
//  Bit_range(Bit_pointer<Iterator> const& bit_p, int const size)
//      Constructor; the number of bits is the range is fixed, but the range can be shifted to a new
//      position.
//  Bit_pointer<Iterator> const begin()
//      Bit_pointer to the first bit of the range.
//  Bit_pointer<Iterator> const end()
//      Bit pointer to the first bit beyond the range.
//  int const size()
//      Size of the Bit_range.
//  Bit_range<Iterator>& next()
//      Alters the Bit_range by changing the start of the Bit_range to the bit pointed to by Bit_range::end().
//      The size of the Bit_range does not change. 
//  template <typename T>
//  operator T() const noexcept
//      Casts a Bit_range to an integral type. If T is a signed type, the value of the most significant bit
//      of the Bit_range determines the sign of the returned value. The Bit_range cannot have a size exceeding
//      that of the number of bits of T.
//  Bit_range& operator =(T const value)
//      Assigns an integral type to a Bit_range. The Bit_range cannot have a size exceeding that of the
//      number of bits of T.

template <typename Iterator> class Bit;
template <typename Iterator> class Bit_range;

template <typename Iterator>
class Bit_pointer {
    friend Bit<Iterator>;
    friend Bit_range<Iterator>;
    
public:
    using difference_type = std::ptrdiff_t;
    using value_type = bool;
    using pointer = Bit_pointer;
    using reference = Bit<Iterator>;
    using iterator_category = std::random_access_iterator_tag;
    
    using Type = typename std::remove_reference<decltype(*Iterator())>::type;
    
    Bit_pointer() {
        static_assert(std::is_integral_v<Type> && std::is_unsigned_v<Type>, "Bit_pointer requires a pointer to, or iterator of, an unsigned integral type");
    };
    
    Bit_pointer(Iterator const offset) :
    d_bit(0),
    d_offset(offset) {
        static_assert(std::is_integral_v<Type> && std::is_unsigned_v<Type>, "Bit_pointer requires a pointer to, or iterator of, an unsigned integral type");
    };
    
    Bit_pointer(Iterator const offset, std::ptrdiff_t const bit) :
    d_bit(int(Operator::euclidian_remainder<sizeof(Type) * 8>(bit))),
    d_offset(offset + (bit - d_bit) / int(sizeof(Type) * 8)) {
        static_assert(std::is_integral_v<Type> && std::is_unsigned_v<Type>, "Bit_pointer requires a pointer to, or iterator of, an unsigned integral type");
    };
    
    constexpr auto& operator=(Iterator const other)  {
        d_offset = other;
        d_bit = 0;
        return *this;
    }
    
    constexpr auto& operator=(Bit_pointer<Iterator> const& other)  {
        d_offset = other.d_offset;
        d_bit = other.d_bit;
        return *this;
    }
    
    Bit_pointer& operator ++() {
        if (++d_bit == (sizeof(*d_offset) * 8)) {
            d_bit = 0;
            ++d_offset;
        };
        return *this;
    }
    
    constexpr Bit_pointer operator++(int) noexcept { Bit_pointer r(d_offset, d_bit); ++*this; return r;}
    
    Bit_pointer& operator --() {
        if (--d_bit == -1) {
            d_bit = (sizeof(*d_offset) * 8) - 1;
            --d_offset;
        };
        return *this;
    }
    
    constexpr Bit_pointer operator--(int) noexcept { Bit_pointer r(d_offset, d_bit); --*this; return r;}
    
    Bit_pointer& operator +=(std::ptrdiff_t const shift) noexcept {
        d_bit += shift;
        if ((d_bit >= (sizeof(*d_offset) * 8)) || (d_bit < 0)){
            auto off_bit = Operator::euclidian_division<sizeof(Type) * 8>(d_bit);
            d_offset = d_offset + off_bit.quot;
            d_bit = int(off_bit.rem);
        }
        return *this;
    }
    
    constexpr Bit_pointer const operator + (std::ptrdiff_t const shift) const noexcept { return Bit_pointer(d_offset, d_bit + shift);}
    Bit_pointer& operator -=(std::ptrdiff_t const shift) noexcept { return *this += - shift;}
    constexpr Bit_pointer const operator - (std::ptrdiff_t const shift) const noexcept { return Bit_pointer(d_offset, d_bit - shift);}
    
    constexpr std::ptrdiff_t const operator - (Bit_pointer<Iterator> const& other) const noexcept {
        return (d_offset - other.d_offset) * sizeof(*d_offset) * 8 + (d_bit - other.d_bit);
    }
    
    constexpr bool const operator *() const noexcept {  return Bit<Iterator>(*this); } //check
    constexpr Bit<Iterator> operator*() noexcept { return Bit<Iterator>(*this);}
    constexpr bool const operator[](std::ptrdiff_t const index) const noexcept { return Bit<Iterator>(*this + index); } //check
    constexpr Bit<Iterator> operator[](std::ptrdiff_t const index)  noexcept { return Bit<Iterator>(d_offset, index + d_bit); }
    
    constexpr bool const operator < (Bit_pointer const& other) const noexcept { return (d_offset <= other.d_offset) && (d_bit <  other.d_bit);}
    constexpr bool const operator <=(Bit_pointer const& other) const noexcept { return (d_offset <= other.d_offset) && (d_bit <= other.d_bit);}
    constexpr bool const operator > (Bit_pointer const& other) const noexcept { return (d_offset >= other.d_offset) && (d_bit >  other.d_bit);}
    constexpr bool const operator >=(Bit_pointer const& other) const noexcept { return (d_offset >= other.d_offset) && (d_bit >= other.d_bit);}
    constexpr bool const operator ==(Bit_pointer const& other) const noexcept { return (d_offset == other.d_offset) && (d_bit == other.d_bit);}
    constexpr bool const operator !=(Bit_pointer const& other) const noexcept { return (d_offset != other.d_offset) || (d_bit != other.d_bit);}
    
private:
    int d_bit;
    Iterator d_offset;
};

template <typename Iterator>
constexpr Bit_pointer<Iterator> const operator + (std::ptrdiff_t const shift, Bit_pointer<Iterator>& second) { return second + shift; }

template <typename Iterator, typename Proxy_t>
void swap(typename Bit_pointer<Iterator>::Proxy_t& first, typename Bit_pointer<Iterator>::Proxy_t& second) { return first.swap(second); }

/*************************************************************************/

template <typename Iter>
class Bit {
    friend  Bit_pointer<Iter>;
    using Type = typename Bit_pointer<Iter>::Type;
    
private:
    Bit(Bit_pointer<Iter> const& bitp) :
    d_bitp(bitp) {}
    
public:
    Bit(Iter location, std::ptrdiff_t const shift) :
    d_local_bitp(location, shift) {}
    
    constexpr                   operator bool()  const noexcept { return ((*d_bitp.d_offset) >> d_bitp.d_bit) & 1; }
    constexpr Bit_pointer<Iter> operator    &()  const noexcept { return d_bitp;} //check
    constexpr const Bit& operator  =(bool const b    ) const noexcept { return *this ^= b ^ bool(*this); }
    constexpr const Bit& operator  =(Bit const& other) const noexcept { return *this = bool(other);}
    constexpr const Bit& operator ^=(bool const b    ) const noexcept { *d_bitp.d_offset ^=   Type( b) << d_bitp.d_bit ; return *this;}
    constexpr const Bit& operator |=(bool const b    ) const noexcept { *d_bitp.d_offset |=   Type( b) << d_bitp.d_bit ; return *this;}
    constexpr const Bit& operator &=(bool const b    ) const noexcept { *d_bitp.d_offset &= ~(Type(~b) << d_bitp.d_bit); return *this;}
    constexpr void set()            const noexcept  { *d_bitp.d_offset |=   Type(1) << d_bitp.d_bit ; };
    constexpr void reset()          const noexcept  { *d_bitp.d_offset &= ~(Type(1) << d_bitp.d_bit); };
    constexpr void flip()           const noexcept  { *d_bitp.d_offset ^=   Type(1) << d_bitp.d_bit ; };
    
    constexpr void swap(Bit& other) const noexcept  { //check
        Type tmp = (*d_bitp.d_offset ^ *other.d_bitp.d_offset) & ((Type(1)) << d_bitp.d_bit);
        *d_bitp.d_offset ^= tmp;
        *other.d_bitp.d_offset ^= tmp;
    }
    
private:
    Bit_pointer<Iter> const& d_bitp = d_local_bitp;
    Bit_pointer<Iter> const d_local_bitp;
};

/*************************************************************************/

template <typename Iterator>
class Bit_range {
public:
    using Type = typename std::remove_reference<decltype(*Iterator())>::type;

    Bit_range(Bit_pointer<Iterator> const& bit_p, std::ptrdiff_t const size) :
    d_bit_pointer(bit_p),
    d_size(size) {};
    
    Bit_range(Iterator start, std::ptrdiff_t const size) :
    d_bit_pointer(start),
    d_size(size) {};
    
    Bit_pointer<Iterator> const begin() const noexcept {return d_bit_pointer;}
    Bit_pointer<Iterator> const end() const noexcept   {return d_bit_pointer + d_size;}
    std::ptrdiff_t const size() const noexcept {return d_size;}
    Bit_range<Iterator>& next() {
        d_bit_pointer.d_bit += size();
        while (d_bit_pointer.d_bit >= (sizeof(Type) * 8)) {
            d_bit_pointer.d_bit -= (sizeof(Type) * 8);
            ++d_bit_pointer.d_offset;
        }
        return *this;
    }
    
    template <typename T>
    operator T() const noexcept {
        static_assert(std::is_integral<T>::value, "Bit_range must be cast to an integral type");
        if (size() == 0)
            return 0;
        T result = T(*d_bit_pointer.d_offset >> d_bit_pointer.d_bit);
        if constexpr (sizeof(T) <= sizeof(Type)) {
            if ((d_bit_pointer.d_bit + size()) > sizeof(Type)*8)
                result |= d_bit_pointer.d_offset[1] << (sizeof(Type)*8 - d_bit_pointer.d_bit);
        }
        else {
            auto bp = d_bit_pointer.d_offset;
            for (int i = sizeof(Type)*8 - d_bit_pointer.d_bit; i < size(); i += sizeof(Type)*8)
                result |= *++bp << i;
        }
        T const mask = ((T(1) << d_size) - 1);
        result &= mask;
        if (std::is_signed_v<T> && (result & (T(1) << (size() - 1))))
            return result | ~mask;
        return result;
    }
    
    template <typename I, std::enable_if_t<std::is_integral_v<typename std::iterator_traits<I>::value_type>, int> = 0>
     void pull_out_series(I const series_start, I const series_end) noexcept {
         using T = typename std::iterator_traits<I>::value_type;
         if (size() == 0)
             std::fill(series_start, series_end, 0);
         else {
             T const mask = ((T(1) << d_size) - 1);
             std::remove_cv_t<Type> buffer = *d_bit_pointer.d_offset >> d_bit_pointer.d_bit;
             for (auto p = series_start; p != series_end; ++p) {
                 T result = T(buffer);
                 buffer >>= this->size();
                 d_bit_pointer.d_bit += this->size();
                 if (d_bit_pointer.d_bit >= sizeof(Type) * 8) {
                     buffer = *++d_bit_pointer.d_offset;
                     d_bit_pointer.d_bit -= sizeof(Type) * 8;
                     result |= buffer << (this->size() - d_bit_pointer.d_bit);
                     buffer >>= d_bit_pointer.d_bit;
                 }
                 if constexpr (std::is_unsigned_v<T>)
                     *p = result & mask;
                 else if (result & (T(1) << (size() - 1)))
                     *p = (result & mask) | ~mask;
                 else
                     *p = result & mask;
             }
         }
     }
//    template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
//     void pull_out_series(T* const series_start, T* const series_end) noexcept {
//         if (size() == 0)
//             std::fill(series_start, series_end, 0);
//         else {
//             T const mask = ((T(1) << d_size) - 1);
//             std::remove_cv_t<Type> buffer = *d_bit_pointer.d_offset >> d_bit_pointer.d_bit;
//             for (auto p = series_start; p != series_end; ++p) {
//                 T result = T(buffer);
//                 buffer >>= this->size();
//                 d_bit_pointer.d_bit += this->size();
//                 if (d_bit_pointer.d_bit >= sizeof(Type) * 8) {
//                     buffer = *++d_bit_pointer.d_offset;
//                     d_bit_pointer.d_bit -= sizeof(Type) * 8;
//                     result |= buffer << (this->size() - d_bit_pointer.d_bit);
//                     buffer >>= d_bit_pointer.d_bit;
//                 }
//                 if constexpr (std::is_unsigned_v<T>)
//                     *p = result & mask;
//                 else if (result & (T(1) << (size() - 1)))
//                     *p = (result & mask) | ~mask;
//                 else
//                     *p = result & mask;
//             }
//         }
//     }

    template <typename T>
    Bit_range& operator |=(T value) noexcept {
        static_assert(std::is_integral<T>::value, "Bit_range assignment requires an integral type");
        if constexpr (std::is_signed_v<T>)
            value &= ((T(1) << d_size) - 1);
        if ((sizeof(T) <= sizeof(Type)) || (size() <= (sizeof(Type) * 8))) {
            *d_bit_pointer.d_offset |= Type(value) << d_bit_pointer.d_bit;
            int const shift_right = int(sizeof(Type)*8) - d_bit_pointer.d_bit;
            if (shift_right < size())
                d_bit_pointer.d_offset[1] |= value >> shift_right;
        }
        else {
            auto bp = d_bit_pointer.d_offset;
            *bp |= value << d_bit_pointer.d_bit;
            value >>= int(sizeof(Type)*8) - d_bit_pointer.d_bit;
            if constexpr (sizeof(Type) < sizeof(T))
                do
                    *++bp |= value;
                while (value >>= sizeof(Type)*8);
        }
        return *this;
    }
    
    template <typename T>
    Bit_range& push_in_series(T* series_start, size_t n_elem) noexcept {
        static_assert(std::is_integral<T>::value, "Bit_range assignment requires an integral type");
        Type buffer = *d_bit_pointer.d_offset;
        while (n_elem--) {
            T value = *series_start++;
            if constexpr (std::is_signed_v<T*>)
                value &= (((T*)(1) << d_size) - 1);
            buffer |= Type(value) << d_bit_pointer.d_bit;
            d_bit_pointer.d_bit += this->size();
            if ((sizeof(T*) <= sizeof(Type)) || (size() <= (sizeof(Type) * 8))) {
                if (d_bit_pointer.d_bit >= (sizeof(Type) * 8)) {
                    *(d_bit_pointer.d_offset++) = buffer;
                    d_bit_pointer.d_bit -= sizeof(Type) * 8;
                    buffer = value >> (this->size() - d_bit_pointer.d_bit);
                }
            }
            else {
                while (d_bit_pointer.d_bit >= (sizeof(Type) * 8)) {
                    *(d_bit_pointer.d_offset++) = buffer;
                    d_bit_pointer.d_bit -= sizeof(Type) * 8;
                    buffer = value >> (this->size() - d_bit_pointer.d_bit);
                }
            }
        }
        *d_bit_pointer.d_offset = buffer;
        return *this;
    }

    template <typename I, std::enable_if_t<std::is_integral_v<typename std::iterator_traits<I>::value_type>, int> = 0>
    Bit_range& push_in_series(I const series_start, I const series_end) noexcept {
        if (this->size()) {
            Type buffer = *d_bit_pointer.d_offset;
            Type value;
            for (auto p = series_start; p != series_end; ++p) {
                if constexpr (std::is_signed_v<typename std::iterator_traits<I>::value_type>)
                    value = *p & (((typename std::iterator_traits<I>::value_type)(1) << d_size) - 1);
                else
                    value = *p;
                buffer |= value << d_bit_pointer.d_bit;
                d_bit_pointer.d_bit += this->size();
                if (d_bit_pointer.d_bit >= (sizeof(Type) * 8)) {
                    *(d_bit_pointer.d_offset++) = buffer;
                    d_bit_pointer.d_bit -= sizeof(Type) * 8;
                    buffer = value >> (this->size() - d_bit_pointer.d_bit);
                    if constexpr (sizeof(typename std::iterator_traits<I>::value_type) > sizeof(Type))
                        while (d_bit_pointer.d_bit >= (sizeof(Type) * 8)) {
                            *(d_bit_pointer.d_offset++) = buffer;
                            d_bit_pointer.d_bit -= sizeof(Type) * 8;
                            buffer = value >> (this->size() - d_bit_pointer.d_bit);
                        }
                }
            }
            *d_bit_pointer.d_offset = buffer;
        }
        return *this;
    }

//template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
//Bit_range& push_in_series(T* const series_start, T* const series_end) noexcept {
//    if (this->size()) {
//        Type buffer = *d_bit_pointer.d_offset;
//        Type value;
//        for (auto p = series_start; p != series_end; ++p) {
//            if constexpr (std::is_signed_v<T>)
//                value = *p & ((T(1) << d_size) - 1);
//            else
//                value = *p;
//            buffer |= value << d_bit_pointer.d_bit;
//            d_bit_pointer.d_bit += this->size();
//            if (d_bit_pointer.d_bit >= (sizeof(Type) * 8)) {
//                *(d_bit_pointer.d_offset++) = buffer;
//                d_bit_pointer.d_bit -= sizeof(Type) * 8;
//                buffer = value >> (this->size() - d_bit_pointer.d_bit);
//                if constexpr (sizeof(T) > sizeof(Type))
//                    while (d_bit_pointer.d_bit >= (sizeof(Type) * 8)) {
//                        *(d_bit_pointer.d_offset++) = buffer;
//                        d_bit_pointer.d_bit -= sizeof(Type) * 8;
//                        buffer = value >> (this->size() - d_bit_pointer.d_bit);
//                    }
//            }
//        }
//        *d_bit_pointer.d_offset = buffer;
//    }
//    return *this;
//}

//    template <typename T, std::enable_if_t<sizeof(T) <= sizeof(Type) && std::is_signed_v<T> && std::is_integral_v<T>, int> = 0>
//    Bit_range& push_in_series(T* const series_start, T* const series_end) noexcept {
//        Type buffer = *d_bit_pointer.d_offset;
//        for (auto p = series_start ;p != series_end; ++p) {
//            Type value = *p & ((T(1) << d_size) - 1);
//            buffer |= value << d_bit_pointer.d_bit;
//            d_bit_pointer.d_bit += this->size();
//            if (d_bit_pointer.d_bit >= (sizeof(Type) * 8)) {
//                *(d_bit_pointer.d_offset++) = buffer;
//                d_bit_pointer.d_bit -= sizeof(Type) * 8;
//                buffer = value >> (this->size() - d_bit_pointer.d_bit);
//            }
//        }
//        *d_bit_pointer.d_offset = buffer;
//        return *this;
//    }

    //    template <typename T>
//    Bit_range& push_in_series(T* series_start, size_t n_elem) noexcept {
//        static_assert(std::is_integral<T>::value, "Bit_range assignment requires an integral type");
//        Type buffer = *d_bit_pointer.d_offset;
//        while (n_elem--) {
//            T value = *series_start++;
//            if constexpr (std::is_signed_v<T*>)
//                value &= (((T*)(1) << d_size) - 1);
//            buffer |= Type(value) << d_bit_pointer.d_bit;
//            d_bit_pointer.d_bit += this->size();
//            while (d_bit_pointer.d_bit >= (sizeof(Type) * 8)) {
//                *(d_bit_pointer.d_offset++) = buffer;
//                d_bit_pointer.d_bit -= sizeof(Type) * 8;
//                buffer = value >> (this->size() - d_bit_pointer.d_bit);
//            }
//        }
//        *d_bit_pointer.d_offset = buffer;
//        return *this;
//    }

    template <typename T>
    Bit_range& operator =(T value) noexcept {
        static_assert(std::is_integral<T>::value, "Bit_range assignment requires an integral type");
        value &= ((T(1) << d_size) - 1);
        auto bp = d_bit_pointer.d_offset;
        if constexpr (sizeof(T) <= sizeof(Type)) {
            typename std::make_unsigned_t<typename Bit_pointer<Iterator>::Type> mask = ((Type(1) << d_size) - 1);
            *bp &= ~(mask << d_bit_pointer.d_bit);
            *bp |= Type(value) << d_bit_pointer.d_bit;
            int const shift_right = int(sizeof(Type)*8) - d_bit_pointer.d_bit;
            if (shift_right < size()) {
                *++bp &= ~(mask >> shift_right);
                *bp |= value >> shift_right;
            }
        }
        else {
            typename std::make_unsigned_t<T> mask = ((T(1) << d_size) - 1);
            *bp &= ~(mask << d_bit_pointer.d_bit);
            *bp |= value << d_bit_pointer.d_bit;
            int const shift_right = int(sizeof(Type)*8) - d_bit_pointer.d_bit;
            value >>= shift_right;
            int bits_left = int(size()) - shift_right;
            for ( ; bits_left > int(sizeof(Type)*8); bits_left -= int(sizeof(Type)*8), value >>= int(sizeof(Type)*8))
                *++bp = value;
            if (bits_left) {
                *++bp &= ~(mask >> (size() - bits_left));
                *bp |= value;
            }
        }
        return *this;
    }
    
private:
    Bit_pointer<Iterator> d_bit_pointer;
    std::size_t const d_size;
};


#endif /* Bit_pointer_h */

