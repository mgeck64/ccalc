#pragma once
#ifndef VARIANT_H
#define VARIANT_H

#include "basics.h"
#include "complex_type.h"
#include <variant>

namespace calc_val {

using variant_type = std::variant<complex_type, uint_type, int_type>;

} // namespace calc_val

// make sure these get included wherever variant_type is used
#include "complex_extras.h"
#include "pow_int.h"

#endif // VARIANT_H
