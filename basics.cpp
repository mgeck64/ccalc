#include "basics.hpp"
#include <limits>

namespace calc_val {

const float_type pi     = boost::math::constants::pi<float_type>();
const float_type two_pi = boost::math::constants::two_pi<float_type>();
const float_type e      = boost::math::constants::e<float_type>();
const float_type nan    = std::numeric_limits<float_type>::quiet_NaN();

}