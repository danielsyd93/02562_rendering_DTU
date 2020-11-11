// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "HitInfo.h"
#include "ObjMaterial.h"
#include "fresnel.h"
#include "RayTracer.h"

using namespace optix;

bool RayTracer::trace_reflected(const Ray& in, const HitInfo& in_hit, Ray& out, HitInfo& out_hit) const
{
  // Initialize the reflected ray and trace it.
  //
  // Input:  in         (the ray to be reflected)
  //         in_hit     (info about the ray-surface intersection)
  //
  // Output: out        (the reflected ray)
  //         out_hit    (info about the reflected ray)
  //
  // Return: true if the reflected ray hit anything
  //
  // Hints: (a) There is a reflect function available in the OptiX math library.
  //        (b) Set out_hit.ray_ior and out_hit.trace_depth.
	
	if (in_hit.has_hit) {
		out.direction = in.direction-2*dot(in.direction, in_hit.geometric_normal)*in_hit.geometric_normal;
		out.origin = in_hit.position;
		out.tmax = RT_DEFAULT_MAX;
		out.tmin = 1e-3;
		out_hit.ray_ior = in_hit.ray_ior;
		out_hit.trace_depth = in_hit.trace_depth+1;
		if(trace_to_closest(out, out_hit))
			return true;
	}
  return false;


}

bool RayTracer::trace_refracted(const Ray& in, const HitInfo& in_hit, Ray& out, HitInfo& out_hit) const
{
  // Initialize the refracted ray and trace it.
  //
  // Input:  in         (the ray to be refracted)
  //         in_hit     (info about the ray-surface intersection)
  //
  // Output: out        (the refracted ray)
  //         out_hit    (info about the refracted ray)
  //
  // Return: true if the refracted ray hit anything
  //
  // Hints: (a) There is a refract function available in the OptiX math library.
  //        (b) Set out_hit.ray_ior and out_hit.trace_depth.
  //        (c) Remember that the function must handle total internal reflection.

	float3 normal;
	const float eta1 = in_hit.ray_ior;
	const float eta2 = get_ior_out(in, in_hit, normal);

	const bool ref= refract(out.direction,in.direction,normal,eta2/eta1);
	if (!ref) {
		return false;
	}
	out.origin = (in_hit.position);
	out.tmax = RT_DEFAULT_MAX;
	out.tmin = 1e-4;
	out_hit.ray_ior = eta2;
	if (trace_to_closest(out, out_hit)) {
		out_hit.trace_depth = in_hit.trace_depth + 1;
		return true;
	}

  return false;
}

bool RayTracer::trace_refracted(const Ray& in, const HitInfo& in_hit, Ray& out, HitInfo& out_hit, float& R) const
{
  // Initialize the refracted ray and trace it.
  // Compute the Fresnel reflectance (see fresnel.h) and return it in R.
  //
  // Input:  in         (the ray to be refracted)
  //         in_hit     (info about the ray-surface intersection)
  //
  // Output: out        (the refracted ray)
  //         out_hit    (info about the refracted ray)
  //
  // Return: true if the refracted ray hit anything
  //
  // Hints: (a) There is a refract function available in the OptiX math library.
  //        (b) Set out_hit.ray_ior and out_hit.trace_depth.
  //        (c) Remember that the function must handle total internal reflection.

	float3 normal;
	const float eta1 = in_hit.ray_ior;
	const float eta2 = get_ior_out(in, in_hit, normal);
	
	const float cos_thate1 = (dot(-in.direction, normal));
	const float k = (1 - (eta1 / eta2) * (eta1 / eta2) * (1 - cos_thate1*cos_thate1));
	if (k < 0.0f) {
		R = 1;
	}
	else {
		const float cos_theta2 = (sqrt(k));
		R = fresnel_R(cos_thate1, cos_theta2, eta1, eta2);
	}
	return trace_refracted(in, in_hit, out, out_hit);
	
}

float RayTracer::get_ior_out(const Ray& in, const HitInfo& in_hit, float3& normal) const
{
  normal = in_hit.shading_normal;
	if(dot(normal, in.direction) > 0.0)
	{
    normal = -normal;
    return 1.0f;
  }
  const ObjMaterial* m = in_hit.material;
  return m ? m->ior : 1.0f;
}
