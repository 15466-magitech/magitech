// from 15-462 by Jim McCann and Michael Stroucken
#include "spline.h"

template<typename T> T Spline<T>::at(float time) const {

    //A1T1b: Evaluate a Catumull-Rom spline

    // Given a time, find the nearest positions & tangent values
    // defined by the control point map.

    // Transform them for use with cubic_unit_spline

    // Be wary of edge cases! What if time is before the first knot,
    // before the second knot, etc...

    // special 1
    if (knots.empty()) {
      return T();
    }

    // special 2
    if (knots.size() == 1) {
      return knots.begin()->second;
    }

    // special 3
    const auto& begin = knots.begin();
    float begintime = begin->first;
    if (time <= begintime) {
      return begin->second;
    }

    // special 4
    const auto& end = std::prev(knots.end());
    float endtime = end->first;
    if (time >= endtime) {
      return end->second;
    }

    auto k2i = knots.upper_bound(time);
    T p2 = k2i->second;
    float t2 = k2i->first;
    auto k1i = std::prev(k2i);
    T p1 = k1i->second;
    float t1 = k1i->first;
    T p0;
    float t0;
    // mirror 1
    if (k1i == knots.begin()) {
      p0 = p1 - (p2 - p1);
      t0 = t1 - (t2 - t1);
    } else {
      auto k0i = std::prev(k1i);
      p0 = k0i->second;
      t0 = k0i->first;
    }

    T p3;
    float t3;
    // mirror 2
    if (k2i == std::prev(knots.end())) {
      p3 = p2 + (p2 - p1);
      t3 = t2 + (t2 - t1);
    } else {
      auto k3i = std::next(k2i);
      p3 = k3i->second;
      t3 = k3i->first;
    }

    T m0 = (p2 - p0) / (t2 - t0);
    T m1 = (p3 - p1) / (t3 - t1);

    // tangents are defined per time unit
    // need to have equivalents in [0, 1] space
    float localfactor = t2 - t1;

    float unittime = (time - t1) / localfactor;
    return cubic_unit_spline(unittime, p1, p2, localfactor * m0, localfactor * m1);

    // return cubic_unit_spline(0.0f, T(), T(), T(), T());
}

template<typename T>
T Spline<T>::cubic_unit_spline(float time, const T& position0, const T& position1,
                               const T& tangent0, const T& tangent1) {

	//A4T1a: Hermite Curve over the unit interval

    // Given time in [0,1] compute the cubic spline coefficients and use them to compute
    // the interpolated value at time 'time' based on the positions & tangents

    // Note that Spline is parameterized on type T, which allows us to create splines over
    // any type that supports the * and + operators.

    //return T();
    assert(time >= 0.0f);
    assert(time <= 1.0f);
    float squaretime = time * time;
    float cubetime = squaretime * time;
    glm::mat4 hermite(
      2.0f, 1.0f, -2.0f, 1.0f,
      -3.0f, -2.0f, 3.0f, -1.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f, 0.0f
    );
    glm::vec4 times(cubetime, squaretime, time, 1.0f);
    glm::vec4 ht = hermite * times;

    T result(ht[0] * position0 + ht[1] * tangent0 + ht[2] * position1 + ht[3] * tangent1); 

    return result;
}

template class Spline<float>;
template class Spline<double>;
template class Spline<glm::vec4>;
template class Spline<glm::vec3>;
template class Spline<glm::vec2>;
template class Spline<glm::mat4>;
