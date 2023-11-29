#pragma once
#include <stdlib.h>
#include <glm/glm.hpp>
/**
    Simple ray segment data structure.

    Along with the ray origin and direction, this data structure additionally
    stores the segment interval [mint, maxt] (whose entries may include
    positive/ negative infinity).
 */


const float Epsilon=0.0001f;


struct Ray
{
    glm::vec3 o;     ///< The origin of the ray
    glm::vec3 d;     ///< The direction of the ray
    float mint;         ///< Minimum distance along the ray segment
    float maxt;         ///< Maximum distance along the ray segment

    float t = std::numeric_limits<float>::infinity(); // current intersection?
    /// Construct a new ray
    Ray() : mint(Epsilon),
        maxt(std::numeric_limits<float>::infinity()) { }

    /// Construct a new ray
    Ray(const glm::vec3 &o, const glm::vec3 &d) : o(o), d(d),
            mint(Epsilon), maxt(std::numeric_limits<float>::infinity()) { }

    /// Construct a new ray
    Ray(const glm::vec3 &o, const glm::vec3 &d,
        float mint, float maxt) : o(o), d(d), mint(mint), maxt(maxt) { }

    /// Copy a ray, but change the covered segment of the copy
    Ray(const Ray &ray, float mint, float maxt)
     : o(ray.o), d(ray.d), mint(mint), maxt(maxt){ }

    /// Return the position of a point along the ray
    glm::vec3 operator() (float time) const { return o + time * d; }
    
    Ray normalizeRay() const
    {
        float rayLength = glm::length(d);

        glm::vec3 normalizedRayDir = d / rayLength;
        float normalizedMint = mint * rayLength;
        float normalizedMaxt = maxt * rayLength;

        return Ray(o, normalizedRayDir, normalizedMint, normalizedMaxt);
    }

};