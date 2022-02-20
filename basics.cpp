#include "basics.hpp"
#include <boost/math/constants/constants.hpp>
#include <limits>

namespace calc_val {

// basic assumptions
static_assert(uint_type(-1) == ~uint_type(0)); // two's compliment integer type
static_assert(int_type(-2) >> int_type(1) == int_type(-1)); // right-shift is arithmetic
static_assert(sizeof(uint_type) == sizeof(int_type));

// basic assumptions about library support for __int128; may need to compile
// with std=gnu++XX flag instead of std=c++XX
static_assert(std::is_integral_v<__int128>);
static_assert(std::is_signed_v<__int128>);
static_assert(std::numeric_limits<__int128>::is_integer);
static_assert(std::numeric_limits<__int128>::is_signed);
static_assert(std::is_integral_v<unsigned __int128>);
static_assert(std::is_unsigned_v<unsigned __int128>);
static_assert(std::numeric_limits<unsigned __int128>::is_integer);
static_assert(!std::numeric_limits<unsigned __int128>::is_signed);
static_assert(std::numeric_limits<__int128>::max() < std::numeric_limits<unsigned __int128>::max());
static_assert(std::numeric_limits<std::int64_t>::max() < std::numeric_limits<__int128>::max());
static_assert(std::numeric_limits<std::uint64_t>::max() < std::numeric_limits<unsigned __int128>::max());
static_assert(std::numeric_limits<unsigned __int128>::digits10 > 0);
static_assert(std::numeric_limits<__int128>::digits10 > 0);
// static_assert(sizeof(std::uintmax_t) >= sizeof(unsigned __int128)); // this fails! need to avoid using uintmax_t
// static_assert(sizeof(std::intmax_t) >= sizeof(__int128)); // this fails! need to avoid using intmax_t

static_assert(BOOST_VERSION == 107400);
// this version of boost produces the correct result of 0 for sin(pi)
// (indirectly used in the expression exp(pi*i) where i is the imaginary unit).
// more recent versions of boost produce a near-zero result; thus we are
// sticking with this version to get the desired result of 0

} // namespace calc_val