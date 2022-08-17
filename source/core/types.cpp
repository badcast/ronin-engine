#include "ronin.h"

namespace RoninEngine {
template <>
const xRect<float> xRect<float>::zero = {};
template <>
const xRect<float> xRect<float>::one = {1, 1, 1, 1};

template <>
const xRect<int> xRect<int>::zero = {};
template <>
const xRect<int> xRect<int>::one = {1, 1, 1, 1};

template <>
Vec2 xRect<float>::getXY() const {
    return Runtime::Vec2(x, y);
}

template <>
Vec2 xRect<float>::getWH() const {
    return Runtime::Vec2(w, h);
}

template <>
Vec2 xRect<int>::getXY() const {
    return Runtime::Vec2(x, y);
}

template <>
Vec2 xRect<int>::getWH() const {
    return Runtime::Vec2(w, h);
}

}  // namespace RoninEngine
