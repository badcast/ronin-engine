#include "ronin.h"

namespace RoninEngine::Runtime
{
    template <>
    const xRect<float> xRect<float>::zero = {};
    template <>
    const xRect<float> xRect<float>::one = { 1, 1, 1, 1 };

    template <>
    const xRect<int> xRect<int>::zero = {};
    template <>
    const xRect<int> xRect<int>::one = { 1, 1, 1, 1 };
} // namespace RoninEngine
