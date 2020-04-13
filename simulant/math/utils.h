#pragma once

#include <cmath>
#include <limits>

namespace smlt {

const float PI = std::acos(-1.0f);
const float TWO_PI = PI * 2.0f;
const float PI_OVER_180 = PI / 180.0f;
const float PI_UNDER_180 = 180.0f / PI;
const float EPSILON = std::numeric_limits<float>::epsilon();

float clamp(float x, float l, float h);
float lerp(float x, float y, float t);
float smoothstep(float e0, float e1, float x);
float smootherstep(float e0, float e1, float x);

template<typename T>
bool almost_equal(const T& lhs, const T& rhs) {
    return lhs + std::numeric_limits<T>::epsilon() > rhs &&
           lhs - std::numeric_limits<T>::epsilon() < rhs;
}


}
