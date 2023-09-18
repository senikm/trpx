//
//  Bit_pointer.hpp
//  Bit_pointer
//
//  Created by Jan Pieter Abrahams on 15/04/2019.
//  Copyright © 2019 Jan Pieter Abrahams. All rights reserved.
//

#ifndef Bit_pointer_h
#define Bit_pointer_h

#include <iterator>
#include <limits>
#include <cstdint>
#include <algorithm>

// Bit<T>, Bit_pointer<T> and Bit_range<T> (where T is a random access iterator or pointer referring
// to an integral type), are three classes that provide a more versatile and powerful alternative to
// std::bitset and std::vector<bool>. Unlike, the number of bits is not limited to 64. Unlike
// std::vector<bool>, these classes allow direct random access any type of container, where the bits
// are guaranteed to be consecutive.
//
// Bit<T> is a reference to a bit in an object of type *T. It allows retrieving and manipulating the
// referenced bit. It has the following member functions:
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
// and therefore compatible with the STL. If a pointer operator causes a Bit_pointer to point beyond the
// size of *T, it moves to another object as defined by the pointer arithmatic of container<T> and the
// native byte order of *T (which is determined by the endedness of the hardware).
//
// Bit_pointer<T> assumes native byte-order. It has the following member functions:
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
//  Bit_range(Bit_pointer<Iter> const& bit_p, int const size)
//      Constructor; the number of bits is the range is fixed, but the range can be shifted to a new
//      position.
//  Bit_pointer<Iter> const begin()
//      Bit_pointer to the first bit of the range.
//  Bit_pointer<Iter> const end()
//      Bit pointer to the first bit beyond the range.
//  int const size()
//      Size of the Bit_range.
//  Bit_range<Iter>& next()
//      Alters the Bit_range by changing the start of the Bit_range to the bit pointed to by Bit_range::end().
//      The size of the Bit_range does not change.
//  template <typename T> operator T() const noexcept
//      Casts a Bit_range to an integral type. If T is a signed type, the value of the most significant bit
//      of the Bit_range determines the sign of the returned value. The Bit_range cannot have a size exceeding
//      that of the number of bits of T.
//  Bit_range& operator =(T const value)
//      Assigns an integral type to a Bit_range. The Bit_range cannot have a size exceeding that of the
//      number of bits of T.
//  Bit_range& operator |=(T const value)
//      Bitwise or-assigns an integral type to a Bit_range. The Bit_range cannot have a size exceeding that of the
//      number of bits of T. It is faster than direct assignment, provided all bits in the Bit_range are zero.
//  template <typename T_iter, typename T = typename std::iterator_traits<T_iter>::value_type> requires std::is_integral_v<T>
//  Bit_range& append_range(T_iter const from, T_iter const to) noexcept {
//      Assigns integral values defined by the range between the iterators 'from' and 'to' to the Bit_range and
//      succeeding Bit_ranges. Updates the Bit_range start to point to the next unassigned Bit_range. Before the
//      values defined by the iterator range 'from/to' are appended to the Bit_range, they are cast to the type T,
//      which defaults to the type of decltype(*from).
//  template <typename T_iter> requires std::is_integral_v<typename std::iterator_traits<T_iter>::value_type>
//      void get_range(T_iter const from, T_iter const to) noexcept
//      Extracts integral values from Bit_range and its succeeding Bit_ranges into the range defined by the
//      iterators from and to. Updates the Bit_range start to point to the next unassigned Bit_range.
//      If T_iter::value type has insufficient precision to store the extracted value, this value is clamped
//      to the maximum representable value (or minimum value in case T_iter::value is signed and underflow occurs).

namespace jpa {

template <typename Iter> class Bit;
template <typename Iter> class Bit_range;

/**
 * @brief Bit_pointer<T> is an STL pointer to a Bit<T> and supports all pointer operators. T is an iterator of an integral type.
 * If a pointer operator causes a Bit_pointer to point beyond the size of *T, it moves to the next T-type object as defined by the
 * pointer arithmetic of container<T>.
 *
 * @tparam Iter Type of the iterator to which this Bit_pointer points.
 */
template <typename Iter>
class Bit_pointer {
    friend Bit<Iter>;
    friend Bit_range<Iter>;

    using Type = typename std::remove_reference<decltype(*Iter())>::type;

public:
    using difference_type = std::ptrdiff_t;
    using value_type = bool;
    using pointer = Bit_pointer;
    using reference = Bit<Iter>;
    using iterator_category = std::random_access_iterator_tag;

    /**
     * @brief Default constructor of an unspecified Bit_pointer<Iter> object.
     */
    Bit_pointer() {
        static_assert(std::is_integral_v<Type> && std::is_unsigned_v<Type>, "Bit_pointer requires a pointer to, or iterator of, an unsigned integral type");
    };

    /**
     * @brief Constructor of a Bit_pointer<Iter> object pointing to the first bit of *offset.
     *
     * @param offset The iterator to the container to which this Bit_pointer points.
     */
    Bit_pointer(Iter const offset) :
        d_bit(0),
        d_offset(offset) {
        static_assert(std::is_integral_v<Type> && std::is_unsigned_v<Type>, "Bit_pointer requires a pointer to, or iterator of, an unsigned integral type");
    };

    /**
     * @brief Constructor of a Bit_pointer<Iter> object pointing to the 'n'-th bit of *offset. If 'n' is
     * negative or larger than sizeof(*T), a bit in a preceding / succeeding object is pointed to.
     *
     * @param offset The iterator to the container to which this Bit_pointer points.
     * @param bit The index of the bit within the container.
     */
    Bit_pointer(Iter const offset, std::ptrdiff_t const bit) :
        d_bit(int(bit & (sizeof(Type) * 8 - 1))),
        d_offset(offset + (bit - d_bit) / int(sizeof(Type) * 8)) {
        static_assert(std::is_integral_v<Type> && std::is_unsigned_v<Type>, "Bit_pointer requires a pointer to, or iterator of, an unsigned integral type");
    };

    /**
     * @brief Assignment operator for assigning an iterator to this Bit_pointer.
     *
     * @param other The iterator to assign.
     * @return A reference to this Bit_pointer after assignment.
     */
    constexpr auto& operator=(Iter const other) {
        d_offset = other;
        d_bit = 0;
        return *this;
    }

    /**
     * @brief Assignment operator for assigning another Bit_pointer to this Bit_pointer.
     *
     * @param other The Bit_pointer to assign.
     * @return A reference to this Bit_pointer after assignment.
     */
    constexpr auto& operator=(Bit_pointer<Iter> const& other) {
        d_offset = other.d_offset;
        d_bit = other.d_bit;
        return *this;
    }

    /**
     * @brief Prefix increment operator (++bitp).
     *
     * @return A reference to this Bit_pointer after increment.
     */
    Bit_pointer& operator++() {
        if (++d_bit == (sizeof(*d_offset) * 8)) {
            d_bit = 0;
            ++d_offset;
        };
        return *this;
    }

    /**
     * @brief Postfix increment operator (bitp++).
     *
     * @return A Bit_pointer to the previous position before increment.
     */
    constexpr Bit_pointer operator++(int) noexcept {
        Bit_pointer r(d_offset, d_bit);
        ++*this;
        return r;
    }

    /**
     * @brief Prefix decrement operator (--bitp).
     *
     * @return A reference to this Bit_pointer after decrement.
     */
    Bit_pointer& operator--() {
        if (--d_bit == -1) {
            d_bit = (sizeof(*d_offset) * 8) - 1;
            --d_offset;
        };
        return *this;
    }

    /**
     * @brief Postfix decrement operator (bitp--).
     *
     * @return A Bit_pointer to the previous position before decrement.
     */
    constexpr Bit_pointer operator--(int) noexcept {
        Bit_pointer r(d_offset, d_bit);
        --*this;
        return r;
    }

    /**
     * @brief Compound assignment operator for adding an offset to this Bit_pointer.
     *
     * @param shift The offset to add.
     * @return A reference to this Bit_pointer after addition.
     */
    Bit_pointer& operator+=(std::ptrdiff_t const shift) noexcept {
        d_bit += shift;
        if ((d_bit >= (sizeof(*d_offset) * 8)) || (d_bit < 0)) {
            auto tmp = int(d_bit & std::ptrdiff_t(sizeof(Type) * 8 - 1));
            d_offset = d_offset + (d_bit - tmp) / std::ptrdiff_t(sizeof(Type) * 8);
            d_bit = tmp;
        }
        return *this;
    }

    /**
     * @brief Addition operator for adding an offset to this Bit_pointer.
     *
     * @param shift The offset to add.
     * @return A new Bit_pointer pointing to the result of the addition.
     */
    constexpr Bit_pointer const operator+(std::ptrdiff_t const shift) const noexcept {
        return Bit_pointer(d_offset, d_bit + shift);
    }

    /**
     * @brief Compound assignment operator for subtracting an offset from this Bit_pointer.
     *
     * @param shift The offset to subtract.
     * @return A reference to this Bit_pointer after subtraction.
     */
    Bit_pointer& operator-=(std::ptrdiff_t const shift) noexcept {
        return *this += -shift;
    }

    /**
     * @brief Subtraction operator for subtracting an offset from this Bit_pointer.
     *
     * @param shift The offset to subtract.
     * @return A new Bit_pointer pointing to the result of the subtraction.
     */
    constexpr Bit_pointer const operator-(std::ptrdiff_t const shift) const noexcept {
        return Bit_pointer(d_offset, d_bit - shift);
    }

    /**
     * @brief Subtraction operator for calculating the difference between two Bit_pointers.
     *
     * @param other The Bit_pointer to subtract from this Bit_pointer.
     * @return The difference as a std::ptrdiff_t value.
     */
    constexpr std::ptrdiff_t const operator-(Bit_pointer<Iter> const& other) const noexcept {
        return (d_offset - other.d_offset) * sizeof(*d_offset) * 8 + (d_bit - other.d_bit);
    }

    /**
     * @brief Dereference operator

 to access the bit pointed to by this Bit_pointer.
     *
     * @return A const bool representing the value of the referenced bit.
     */
    constexpr bool const operator*() const noexcept {
        return Bit<Iter>(*this);
    }

    /**
     * @brief Dereference operator to access the bit pointed to by this Bit_pointer.
     *
     * @return A Bit<Iter> object representing the referenced bit.
     */
    constexpr Bit<Iter> operator*() noexcept {
        return Bit<Iter>(*this);
    }

    /**
     * @brief Subscript operator to access bits at a specified index relative to this Bit_pointer.
     *
     * @param index The index relative to this Bit_pointer.
     * @return A const bool representing the value of the referenced bit.
     */
    constexpr bool const operator[](std::ptrdiff_t const index) const noexcept {
        return Bit<Iter>(*this + index);
    }

    /**
     * @brief Subscript operator to access bits at a specified index relative to this Bit_pointer.
     *
     * @param index The index relative to this Bit_pointer.
     * @return A Bit<Iter> object representing the referenced bit.
     */
    constexpr Bit<Iter> operator[](std::ptrdiff_t const index) noexcept {
        return Bit<Iter>(d_offset, index + d_bit);
    }

    /**
     * @brief Less than comparison operator for Bit_pointers.
     *
     * @param other The Bit_pointer to compare with.
     * @return True if this Bit_pointer is less than the other Bit_pointer, false otherwise.
     */
    constexpr bool const operator<(Bit_pointer const& other) const noexcept {
        return (d_offset <= other.d_offset) && (d_bit < other.d_bit);
    }

    /**
     * @brief Less than or equal to comparison operator for Bit_pointers.
     *
     * @param other The Bit_pointer to compare with.
     * @return True if this Bit_pointer is less than or equal to the other Bit_pointer, false otherwise.
     */
    constexpr bool const operator<=(Bit_pointer const& other) const noexcept {
        return (d_offset <= other.d_offset) && (d_bit <= other.d_bit);
    }

    /**
     * @brief Greater than comparison operator for Bit_pointers.
     *
     * @param other The Bit_pointer to compare with.
     * @return True if this Bit_pointer is greater than the other Bit_pointer, false otherwise.
     */
    constexpr bool const operator>(Bit_pointer const& other) const noexcept {
        return (d_offset >= other.d_offset) && (d_bit > other.d_bit);
    }

    /**
     * @brief Greater than or equal to comparison operator for Bit_pointers.
     *
     * @param other The Bit_pointer to compare with.
     * @return True if this Bit_pointer is greater than or equal to the other Bit_pointer, false otherwise.
     */
    constexpr bool const operator>=(Bit_pointer const& other) const noexcept {
        return (d_offset >= other.d_offset) && (d_bit >= other.d_bit);
    }

    /**
     * @brief Equality comparison operator for Bit_pointers.
     *
     * @param other The Bit_pointer to compare with.
     * @return True if this Bit_pointer is equal to the other Bit_pointer, false otherwise.
     */
    constexpr bool const operator==(Bit_pointer const& other) const noexcept {
        return (d_offset == other.d_offset) && (d_bit == other.d_bit);
    }

    /**
     * @brief Inequality comparison operator for Bit_pointers.
     *
     * @param other The Bit_pointer to compare with.
     * @return True if this Bit_pointer is not equal to the other Bit_pointer, false otherwise.
     */
    constexpr bool const operator!=(Bit_pointer const& other) const noexcept {
        return (d_offset != other.d_offset) || (d_bit != other.d_bit);
    }

private:
    int d_bit;
    Iter d_offset;
};

/**
 * @brief Addition operator for adding an offset to a Bit_pointer.
 *
 * @param shift The offset to add.
 * @return A new Bit_pointer pointing to the result of the addition.
 */
template <typename Iter>
constexpr Bit_pointer<Iter> const operator + (std::ptrdiff_t const shift, Bit_pointer<Iter>& second) { return second + shift; }

/*************************************************************************/

/**
 * @brief The `Bit` class represents a reference to a bit in an object of type *T.
 *
 * It allows retrieving and manipulating the referenced bit.
 *
 * @tparam Iter A random access iterator or pointer referring to an integral type.
 */
template <typename Iter>
class Bit {
    friend Bit_pointer<Iter>;
    using Type = typename Bit_pointer<Iter>::Type;
    static_assert(std::is_integral_v<typename std::iterator_traits<Iter>::value_type>, "Bit<Iter> only allowed when *Iter is an integral type");

    Bit(Bit_pointer<Iter> const& bitp) : d_bitp(bitp) {}
    
public:
    /**
     * @brief Constructs a `Bit` object at the specified location.
     *
     * @param location The iterator or pointer to the bit's location.
     * @param shift The shift amount to position the `Bit` within the referenced object.
     */
    Bit(Iter location, std::ptrdiff_t const shift) : d_local_bitp(location, shift) {}
    
    /**
     * @brief Implicitly converts the `Bit` to a boolean value.
     *
     * @return The value of the referenced bit as a boolean.
     */
    constexpr operator bool() const noexcept { return ((*d_bitp.d_offset) >> d_bitp.d_bit) & 1; }
    
    /**
     * @brief Returns a `Bit_pointer` to the referenced bit.
     *
     * @return A `Bit_pointer` to the referenced bit.
     */
    constexpr Bit_pointer<Iter> operator&() const noexcept { return d_bitp; }
    
    /**
     * @brief Sets the referenced bit to the specified boolean value.
     *
     * @param b The boolean value to set the bit to.
     * @return A reference to the modified `Bit` object.
     */
    constexpr const Bit& operator=(bool const b) const noexcept { return *this ^= b ^ bool(*this); }
    
    /**
     * @brief Sets the referenced bit to the value of another `Bit`.
     *
     * @param other The `Bit` object to copy the value from.
     * @return A reference to the modified `Bit` object.
     */
    constexpr const Bit& operator=(Bit const& other) const noexcept { return *this = bool(other); }
    
    /**
     * @brief Performs a bitwise XOR operation between the referenced bit and a boolean value.
     *
     * @param b The boolean value to XOR with.
     * @return A reference to the modified `Bit` object.
     */
    constexpr const Bit& operator^=(bool const b) const noexcept { *d_bitp.d_offset ^= Type(b) << d_bitp.d_bit; return *this; }
    
    /**
     * @brief Performs a bitwise OR operation between the referenced bit and a boolean value.
     *
     * @param b The boolean value to OR with.
     * @return A reference to the modified `Bit` object.
     */
    constexpr const Bit& operator|=(bool const b) const noexcept { *d_bitp.d_offset |= Type(b) << d_bitp.d_bit; return *this; }
    
    /**
     * @brief Performs a bitwise AND operation between the referenced bit and a boolean value.
     *
     * @param b The boolean value to AND with.
     * @return A reference to the modified `Bit` object.
     */
    constexpr const Bit& operator&=(bool const b) const noexcept { *d_bitp.d_offset &= ~(Type(~b) << d_bitp.d_bit); return *this; }
    
    /**
     * @brief Sets the referenced bit to 1 (true).
     */
    constexpr void set() const noexcept { *d_bitp.d_offset |= Type(1) << d_bitp.d_bit; }
    
    /**
     * @brief Sets the referenced bit to 0 (false).
     */
    constexpr void reset() const noexcept { *d_bitp.d_offset &= ~(Type(1) << d_bitp.d_bit); }
    
    /**
     * @brief Flips the value of the referenced bit (from 1 to 0 or vice versa).
     */
    constexpr void flip() const noexcept { *d_bitp.d_offset ^= Type(1) << d_bitp.d_bit; }
    
    /**
     * @brief Swaps the bit referenced by this `Bit` with another `Bit`.
     *
     * @param other The `Bit` object to swap with.
     */
    constexpr void swap(Bit& other) const noexcept {
        Type tmp = (*d_bitp.d_offset ^ *other.d_bitp.d_offset) & ((Type(1)) << d_bitp.d_bit);
        *d_bitp.d_offset ^= tmp;
        *other.d_bitp.d_offset ^= tmp;
    }
    
private:
    Bit_pointer<Iter> const& d_bitp = d_local_bitp;
    Bit_pointer<Iter> const d_local_bitp;
};

/*************************************************************************/

/**
 * @brief Bit_range<T> defines a view of a range of Bit<T> in a container<T>, providing an interface with the
 * STL. It also allows packing and unpacking integral types into and out of a Bit_range<T>.
 *
 * @tparam Iter The iterator type.
 */
template <typename Iter>
class Bit_range {
    using Type = typename std::remove_reference<decltype(*Iter())>::type;
public:

    /**
     * @brief Constructor; the number of bits is the range is fixed at run-time, but the range can be shifted
     * to a new position.
     *
     * @param bit_p The Bit_pointer specifying the start of the Bit_range.
     * @param size The number of bits in the Bit_range.
     */
    Bit_range(Bit_pointer<Iter> const& bit_p, std::ptrdiff_t const size) :
    d_bit_pointer(bit_p),
    d_size(size) {};
    
    /**
     * @brief Constructor; initializes a Bit_range from an iterator and a size.
     *
     * @param start The iterator specifying the start of the Bit_range.
     * @param size The number of bits in the Bit_range.
     */
    Bit_range(Iter const start, std::ptrdiff_t const size) :
    d_bit_pointer(start),
    d_size(size) {};
    
    /**
     * @brief Returns a Bit_pointer to the first bit of the Bit_range.
     *
     * @return A Bit_pointer to the beginning of the Bit_range.
     */
    Bit_pointer<Iter> const begin() const noexcept {return d_bit_pointer;}
    
    /**
      * @brief Returns a Bit_pointer to the first bit beyond the Bit_range.
      *
      * @return A Bit_pointer to the end of the Bit_range.
      */
    Bit_pointer<Iter> const end() const noexcept   {return d_bit_pointer + d_size;}
    
    /**
     * @brief Returns the size (number of bits) of the Bit_range.
     *
     * @return The size of the Bit_range.
     */
    std::ptrdiff_t const size() const noexcept {return d_size;}
    
    
    /**
     * @brief Alters the Bit_range by changing the start of the Bit_range to the bit pointed to by Bit_range::end().
     * The size of the Bit_range does not change.
     *
     * @return A reference to the modified Bit_range.
     */
    Bit_range<Iter>& next() noexcept {
        d_bit_pointer.d_bit += size();
        while (d_bit_pointer.d_bit >= (sizeof(Type) * 8)) {
            d_bit_pointer.d_bit -= (sizeof(Type) * 8);
            ++d_bit_pointer.d_offset;
        }
        return *this;
    }
    
    /**
     * @brief Casts a Bit_range to an integral type. If T is a signed type, the value of the most significant bit
     * of the Bit_range determines the sign of the returned value. If the size of the Bit_range exceeds that of the
     * number of bits of T, the result is undefined.
     *
     * @tparam T The integral type to cast to.
     * @return The Bit_range cast to the specified integral type.
     */
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
    
    /**
      * @brief Bitwise OR-assigns an integral type to a Bit_range. The Bit_range cannot have a size exceeding
      * that of the number of bits of T. It is faster than direct assignment, provided all bits in the Bit_range
      * are zero. If the size of the Bit_range is smaller, the higher bits are truncated.
      *
      * @tparam T The integral type to OR-assign.
      * @param value The value to OR-assign to the Bit_range.
      * @return A reference to the modified Bit_range.
      */
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
    
    /**
     * @brief Assigns an integral type to a Bit_range. The Bit_range cannot have a size exceeding that of the
     * number of bits of T. If the size of the Bit_range is smaller, the higher bits are truncated.
     *
     * @tparam T The integral type to assign.
     * @param value The value to assign to the Bit_range.
     * @return A reference to the modified Bit_range.
     */
    template <typename T>
    Bit_range& operator =(T value) noexcept {
        static_assert(std::is_integral<T>::value, "Bit_range assignment requires an integral type");
        value &= ((T(1) << d_size) - 1);
        auto bp = d_bit_pointer.d_offset;
        if constexpr (sizeof(T) <= sizeof(Type)) {
            typename std::make_unsigned_t<typename Bit_pointer<Iter>::Type> mask = ((Type(1) << d_size) - 1);
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
    
    /**
     * @brief Assigns integral values defined by the range between the iterators 'from' and 'to' to the Bit_range and
     * succeeding Bit_ranges. Updates the Bit_range start to point to the next unassigned Bit_range.
     *
     * @tparam T_iter The iterator type.
     * @param from The starting iterator of the range to append.
     * @param to The ending iterator of the range to append.
     * @return A reference to the modified Bit_range.
     */
    template <typename T_iter> requires std::is_integral_v<typename std::iterator_traits<T_iter>::value_type>
    Bit_range& append_range(T_iter const from, T_iter const to) noexcept {
        using T = typename std::iterator_traits<T_iter>::value_type;
        if (this->size()) {
            Type buffer = *d_bit_pointer.d_offset;
            std::conditional_t<(sizeof(Type) > sizeof(T)), Type, T> value;
            for (auto p = from; p != to; ++p) {
                if constexpr (std::is_signed_v<T>)
                    value = static_cast<T>(*p) & ((T(1) << d_size) - 1);
                else
                    value = static_cast<T>(*p);
                buffer |= value << d_bit_pointer.d_bit;
                d_bit_pointer.d_bit += this->size();
                if (d_bit_pointer.d_bit >= (sizeof(Type) * 8)) {
                    *(d_bit_pointer.d_offset++) = buffer;
                    d_bit_pointer.d_bit -= sizeof(Type) * 8;
                    buffer = value >>= (this->size() - d_bit_pointer.d_bit);
                    if constexpr (sizeof(T) > sizeof(Type)) {
                        while (d_bit_pointer.d_bit >= (sizeof(Type) * 8)) {
                            *(d_bit_pointer.d_offset++) = buffer;
                            d_bit_pointer.d_bit -= sizeof(Type) * 8;
                            buffer = value >>= sizeof(Type) * 8;
                        }
                    }
                    
                }
            }
            *d_bit_pointer.d_offset = buffer;
        }
        return *this;
    }

    /**
     * @brief Extracts integral values from Bit_range and its succeeding Bit_ranges into the range defined by the
     * iterators from and to. Updates the Bit_range start to point to the next unassigned Bit_range.
     * If T_iter::value_type has insufficient precision to store the extracted value, this value is clamped
     * to the maximum representable value (or minimum value in case T_iter::value is signed and underflow occurs).
     *
     * @tparam T_iter The iterator type.
     * @param from The starting iterator of the range to extract to.
     * @param to The ending iterator of the range to extract to.
     */
    template <typename T_iter> requires std::is_integral_v<typename std::iterator_traits<T_iter>::value_type>
    void get_range(T_iter const from, T_iter const to) noexcept {
        using T = typename std::iterator_traits<T_iter>::value_type;
        if (size() == 0)
            std::fill(from, to, 0);
        else if (sizeof(T) * 8 < this->size()) {
            if constexpr (std::is_unsigned_v<T>) {
                std::uint64_t arr[to - from];
                get_range(arr, arr + (to - from));
                constexpr std::uint64_t upper = static_cast<std::int64_t>(std::numeric_limits<T>::max());
                for (int i=0; i != (to - from); ++i)
                    from[i] = static_cast<T>(std::min(arr[i], upper));
            }
            else {
                std::int64_t arr[to - from];
                get_range(arr, arr + (to - from));
                constexpr std::int64_t lower = static_cast<std::int64_t>(std::numeric_limits<T>::min());
                constexpr std::int64_t upper = static_cast<std::int64_t>(std::numeric_limits<T>::max());
                for (int i=0; i != (to - from); ++i)
                    from[i] = static_cast<T>(std::clamp(arr[i], lower, upper));
            }
        }
        else {
            T const mask = ((T(1) << d_size) - 1);
            std::remove_cv_t<Type> buffer = *d_bit_pointer.d_offset >> d_bit_pointer.d_bit;
            for (auto p = from; p != to; ++p) {
                T result = T(buffer);
                buffer >>= this->size();
                d_bit_pointer.d_bit += this->size();
                if constexpr (sizeof(T) > sizeof(Type))
                    while (d_bit_pointer.d_bit >= (sizeof(Type) * 8)) {
                        buffer = *++d_bit_pointer.d_offset;
                        d_bit_pointer.d_bit -= sizeof(Type) * 8;
                        result |= buffer << (this->size() - d_bit_pointer.d_bit);
                        buffer >>= d_bit_pointer.d_bit;
                    }
                else if (d_bit_pointer.d_bit >= sizeof(Type) * 8) {
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
    
private:
    Bit_pointer<Iter> d_bit_pointer;
    std::size_t const d_size;
};

}


#endif /* Bit_pointer_h */

