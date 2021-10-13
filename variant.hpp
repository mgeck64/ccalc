#ifndef VARIANT_HPP
#define VARIANT_HPP

#include "basics.hpp"
#include "complex_type.hpp"
#include <variant>

namespace calc_val {

using variant_type = std::variant<complex_type, uint_type, int_type>;

} // namespace calc_val

// make sure these get included wherever variant_type is used
#include "complex_extras.hpp"
#include "pow_int.hpp"

#endif // VARIANT_HPP
