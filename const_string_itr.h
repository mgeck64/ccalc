#ifndef CONST_STRING_ITR_H
#define CONST_STRING_ITR_H

#include <string_view>
#include <string>
#include <cstring>
#include <cassert>

class const_string_itr {
// a read-only iterator to make iterating over a contiguous range of characters
// that's not necessarily a c-style string (e.g., a string view) more convenient
// and reliable than with simple iterators but still efficient. this is achieved
// via member functions such as operator bool(), which returns whether the
// iterator (if valid) is dereferencable, and via assertions that validate
// character access (including dereferencing), incrementing and decrementing
// operations in debug code

// an iterator holds a current position pointer, an end position pointer (one
// past the last element of the range) and, in debug code, a start position
// pointer (the start of the range)

// (this class was originally designed to have the null character returned upon
// an invalid character access, and to prevent increment and decrement
// operations from exceeding the limits of the range in non-debug code, but i
// decided against incurring the extra runtime overhead that entailed)

public:
    // basic traits per std::iterator_traits:

    using iterator_category = std::random_access_iterator_tag;
    using value_type = char;
    using pointer = const value_type*;
    using reference = const value_type&;
    using difference_type = std::ptrdiff_t;

    // extended trait:

    using size_type = std::size_t;

    // construction:

    const_string_itr() noexcept = default; // empty range
    const_string_itr(const const_string_itr&) noexcept = default;
    const_string_itr(std::string_view view) noexcept;
    const_string_itr(const std::string& str) noexcept;
    const_string_itr(pointer c_str) noexcept; // c-style string
    const_string_itr(pointer p, size_type length) noexcept;

    // basic iterator functionality; this should fully model std random access
    // iterator concept:

    auto operator*() const noexcept -> reference;
    auto operator[](difference_type i) const noexcept -> reference;

    auto operator++() noexcept -> const_string_itr&;
    auto operator++(int) noexcept -> const_string_itr;
    auto operator--() noexcept -> const_string_itr&;
    auto operator--(int) noexcept -> const_string_itr;
    auto operator=(const const_string_itr&) noexcept -> const_string_itr& = default;
    auto operator+=(size_type n) noexcept -> const_string_itr&;
    auto operator-=(size_type n) noexcept -> const_string_itr&;
    auto operator+(size_type n) const noexcept -> const_string_itr;
    friend auto operator+(size_type n, const const_string_itr& itr) noexcept -> const_string_itr; // oddball case required by random access iterator concept
    auto operator-(size_type n) const noexcept -> const_string_itr;
    auto operator-(const const_string_itr& other) const noexcept -> difference_type;

    auto operator==(const const_string_itr& other) noexcept -> bool;
    auto operator!=(const const_string_itr& other) noexcept -> bool;
    auto operator<(const const_string_itr& other) noexcept -> bool;
    auto operator>(const const_string_itr& other) noexcept -> bool;
    auto operator<=(const const_string_itr& other) noexcept -> bool;
    auto operator>=(const const_string_itr& other) noexcept -> bool;
    // comparison operators consider only the current positions of the iterators
    // being compared; e.g., *this == other if *this's current position ==
    // other's current position, regardless of their ranges

    // extended functionality. gives iterator properties of a range:

    auto at_end() const noexcept -> bool;
    // returns current position == end position.

    explicit operator bool() const noexcept;
    // returns whether the iterator, if valid, is dereferencable (i.e., current
    // position != end positon). (does defining operator bool() this way violate
    // the principle of least astonishment?) assumes the iterator is valid
    // (start position <= current position <= end position)

    auto length() const noexcept -> size_type;
    // returns the remaining size of the range (end position - current position)

    auto back() const noexcept -> reference;
    // returns the last char in the range; assumes the range is not empty

    auto remove_suffix(size_type n) noexcept -> void;
    // decrements the end position by n; n is assumed to be <= length()

    // support for ranged for loops and std algorithms:

    auto begin() const noexcept -> pointer; // note: returns the current position
    auto end() const noexcept -> pointer;

    // conversions:

    operator std::string_view() const noexcept;
    auto view() const noexcept -> std::string_view;
    auto string() const noexcept -> std::string;
    // note: the returned view/string will be from the current position to the
    // end of the range

    // note: string comparison and other higher level operations are not
    // supported; to perform those, convert to std::string_view or std::string

private:
#ifndef NDEBUG
    const_string_itr(pointer start, pointer pos, pointer end) noexcept;
    pointer start_ = 0;
#else
    const_string_itr(pointer pos, pointer end) noexcept;
#endif
    pointer pos_ = 0; // current position
    pointer end_ = 0;
};



// note: constructors assert validity of initial state even if redundantly

inline const_string_itr::const_string_itr(std::string_view view) noexcept :
#ifndef NDEBUG
    start_{view.data()},
#endif
    pos_{view.data()}, end_{pos_ + view.size()} {assert(start_ == pos_ && pos_ <= end_);}

inline const_string_itr::const_string_itr(const std::string& str) noexcept :
#ifndef NDEBUG
    start_{str.data()},
#endif
    pos_{str.data()}, end_{pos_ + str.size()} {assert(start_ == pos_ && pos_ <= end_);}

inline const_string_itr::const_string_itr(pointer c_str) noexcept :
#ifndef NDEBUG
    start_{c_str},
#endif
    pos_{c_str}, end_{c_str + std::strlen(c_str)} {assert(start_ == pos_ && pos_ <= end_);}

inline const_string_itr::const_string_itr(pointer p, size_type length) noexcept :
#ifndef NDEBUG
    start_{p},
#endif
    pos_{p}, end_{p + length} {assert(start_ == pos_ && pos_ <= end_);}

#ifndef NDEBUG
inline const_string_itr::const_string_itr(pointer start, pointer pos, pointer end) noexcept :
    start_{start}, pos_{pos}, end_{end} {assert(start_ <= pos_ && pos <= end_);}
#else
inline const_string_itr::const_string_itr(pointer pos, pointer end) noexcept :
    pos_{pos}, end_{end} {}
#endif

inline auto const_string_itr::operator*() const noexcept -> reference
{assert(start_ <= pos_ && pos_ < end_); return *pos_;}

inline auto const_string_itr::operator[](difference_type i) const noexcept -> reference
{assert(start_ <= pos_ + i && pos_ + i < end_); return pos_[i];}

inline auto const_string_itr::operator++() noexcept -> const_string_itr&
{assert(pos_ != end_); ++pos_; return *this;}

inline auto const_string_itr::operator++(int) noexcept -> const_string_itr
{assert(pos_ != end_); auto t = *this; ++pos_; return t;}

inline auto const_string_itr::operator--() noexcept -> const_string_itr&
{assert(pos_ != start_); --pos_; return *this;}

inline auto const_string_itr::operator--(int) noexcept -> const_string_itr
{assert(pos_ != start_); auto t = *this; --pos_; return t;}

inline auto const_string_itr::operator+=(size_type n) noexcept -> const_string_itr& {
    assert(start_ <= pos_ + n && pos_ + n <= end_); // (test for overflow too)
    pos_ += n;
    return *this;
}

inline auto const_string_itr::operator-=(size_type n) noexcept -> const_string_itr& {
    assert(start_ <= pos_ - n && pos_ - n <= end_); // (test for underflow too)
    pos_ -= n;
    return *this;
}

inline auto const_string_itr::operator+(size_type n) const noexcept -> const_string_itr {
    assert(start_ <= pos_ + n && pos_ + n <= end_); // (test for overflow too)
#ifndef NDEBUG
    return {start_, pos_ + n, end_};
#else
    return {pos_ + n, end_};
#endif
}

inline auto operator+(const_string_itr::size_type n, const const_string_itr& itr) noexcept -> const_string_itr {
    assert(itr.start_ <= n + itr.pos_ && n + itr.pos_ <= itr.end_); // (test for overflow too)
#ifndef NDEBUG
    return const_string_itr{itr.start_, n + itr.pos_, itr.end_};
#else
    return const_string_itr{n + itr.pos_, itr.end_};
#endif
}

inline auto const_string_itr::operator-(size_type n) const noexcept -> const_string_itr {
    assert(start_ <= pos_ - n && pos_ - n <= end_); // (test for underflow too)
#ifndef NDEBUG
    return {start_, pos_ - n, end_};
#else
    return {pos_ - n, end_};
#endif
}

inline auto const_string_itr::operator-(const const_string_itr& other) const noexcept -> difference_type
{return pos_ - other.pos_;}

inline auto const_string_itr::operator==(const const_string_itr& other) noexcept -> bool
{return pos_ == other.pos_;}

inline auto const_string_itr::operator<(const const_string_itr& other) noexcept -> bool
{return pos_ < other.pos_;}

inline auto const_string_itr::operator>(const const_string_itr& other) noexcept -> bool
{return pos_ > other.pos_;}

inline auto const_string_itr::operator<=(const const_string_itr& other) noexcept -> bool
{return pos_ <= other.pos_;}

inline auto const_string_itr::operator>=(const const_string_itr& other) noexcept -> bool
{return pos_ >= other.pos_;}

inline auto const_string_itr::operator!=(const const_string_itr& other) noexcept -> bool
{return pos_ != other.pos_;}

inline auto const_string_itr::at_end() const noexcept -> bool
{return pos_ == end_;}

inline const_string_itr::operator bool() const noexcept
{assert(start_ <= pos_ && pos_ <= end_); return pos_ != end_;}

inline auto const_string_itr::length() const noexcept -> size_type
{return end_ - pos_;}

inline auto const_string_itr::back() const noexcept -> reference
{assert(start_ < end_); return end_[-1];}

inline auto const_string_itr::remove_suffix(size_type n) noexcept -> void {
    assert(end_ - n >= pos_ && end_ - n <= end_); // (test for underflow too)
    end_ -= n;
}

inline auto const_string_itr::begin() const noexcept -> pointer
{return pos_;}

inline auto const_string_itr::end() const noexcept -> pointer
{return end_;}

inline const_string_itr::operator std::string_view() const noexcept
{return {pos_, static_cast<std::string_view::size_type>(end_ - pos_)};}

inline auto const_string_itr::view() const noexcept -> std::string_view
{return {pos_, static_cast<std::string_view::size_type>(end_ - pos_)};}

inline auto const_string_itr::string() const noexcept -> std::string
{return {pos_, end_};}

#endif // CONST_STRING_ITR_H
