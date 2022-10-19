#pragma once

#include "dependency.h"

namespace RoninEngine {
// default square pixel/100
constexpr float pixelsPerPoint = 100;
constexpr float spaceDiv = pixelsPerPoint / 2.f;
constexpr float maxWorldScalar = 10000;

namespace Runtime {
constexpr auto const_storm_dimensions = 0xFFFFFF;
constexpr auto const_storm_steps_flag = std::numeric_limits<std::uint32_t>::max();
constexpr auto const_storm_xDeterminant = 0xF000000;
constexpr auto const_storm_yDeterminant = 0xF0000000;
constexpr auto const_storm_yDeterminant_start = 0x20000000;
constexpr auto const_storm_yDeterminant_inverse = 0x30000000;
}  // namespace Runtime

}  // namespace RoninEngine
