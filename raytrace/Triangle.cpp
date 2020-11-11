// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "HitInfo.h"
#include "Triangle.h"

using namespace optix;

bool intersect_triangle(const Ray& ray,
    const float3& v0,
    const float3& v1,
    const float3& v2,
    float3& n,
    float& t,
    float& v,
    float& w)
{
    // Implement ray-triangle intersection here (see Listing 1 in the lecture note).
    // Note that OptiX also has an implementation, so you can get away
    // with not implementing this function. However, I recommend that
    // you implement it for completeness.

   const float3 e0 = v1 - v0;
   const float3 e1 = v0 - v2;
    n = cross(e0, e1);

    float tTop = dot((v0 - ray.origin), n);
    float tBot = dot(ray.direction, n);
    if (abs(tBot) < 1e-7) {
        return false;
    }
    t = tTop / tBot;
    const float3 tempTop = cross((v0 - ray.origin), ray.direction);
    v = dot(tempTop, e1) / tBot;
    w = dot(tempTop, e0) / tBot;
    if (v > 0.0 && w > 0.0 && (v + w) <= 1 && ray.tmin<=t && t<=ray.tmax){
        n = -n;
        return true;
    }
    else {
        return false;
    }

}


bool Triangle::intersect(const Ray& r, HitInfo& hit, unsigned int prim_idx) const
{
  // Implement ray-triangle intersection here.
  //
  // Input:  r                    (the ray to be checked for intersection)
  //         prim_idx             (index of the primitive element in a collection, not used here)
  //
  // Output: hit.has_hit          (set true if the ray intersects the triangle)
  //         hit.dist             (distance from the ray origin to the intersection point)
  //         hit.position         (coordinates of the intersection point)
  //         hit.geometric_normal (the normalized normal of the triangle)
  //         hit.shading_normal   (the normalized normal of the triangle)
  //         hit.material         (pointer to the material of the triangle)
  //        (hit.texcoord)        (texture coordinates of intersection point, not needed for Week 1)
  //
  // Return: True if the ray intersects the triangle, false otherwise
  //
  // Relevant data fields that are available (see Triangle.h)
  // r                            (the ray)
  // v0, v1, v2                   (triangle vertices)
  // (t0, t1, t2)                 (texture coordinates for each vertex, not needed for Week 1)
  // material                     (material of the triangle)
  //
  // Hint: Use the function intersect_triangle(...) to get the hit info.
  //       Note that you need to do scope resolution (optix:: or just :: in front
  //       of the function name) to choose between the OptiX implementation and
  //       the function just above this one.
    
    float3 normal=make_float3(0);
    float t,v,w;
    
    if (::intersect_triangle(r, v0, v1, v2, normal, t, v, w)) {
        if (r.tmin <= t && t <= r.tmax) {
            hit.has_hit = true;
            hit.dist = t;
            hit.geometric_normal = normalize(normal);
            hit.shading_normal = normalize(normal);
            hit.position = r.origin + r.direction * t;
            hit.material = &get_material();
            return true;
        }
    }
    return false;
    
}

void Triangle::transform(const Matrix4x4& m) 
{ 
  v0 = make_float3(m*make_float4(v0, 1.0f)); 
  v1 = make_float3(m*make_float4(v1, 1.0f)); 
  v2 = make_float3(m*make_float4(v2, 1.0f)); 
}

Aabb Triangle::compute_bbox() const
{
  Aabb bbox;
  bbox.include(v0);
  bbox.include(v1);
  bbox.include(v2);
  return bbox;
}
