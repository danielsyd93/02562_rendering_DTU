// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "HitInfo.h"
#include "Phong.h"

using namespace optix;

#ifndef M_1_PIf
#define M_1_PIf 0.31830988618379067154
#endif

float3 Phong::shade(const Ray& r, HitInfo& hit, bool emit) const
{
 const float3 rho_d = get_diffuse(hit);
 const float3 rho_s = get_specular(hit);
 const  float s = get_shininess(hit);
  float3 result = make_float3(0);
  if (length(rho_d + rho_s) > 1) {
	  return result;
  }

  // Implement Phong reflection here.
  //
  // Input:  r          (the ray that hit the material)
  //         hit        (info about the ray-surface intersection)
  //         emit       (passed on to Emission::shade)
  //
  // Return: radiance reflected to where the ray was coming from
  //
  // Relevant data fields that are available (see Lambertian.h, Ray.h, and above):
  // lights             (vector of pointers to the lights in the scene)
  // hit.position       (position where the ray hit the material)
  // hit.shading_normal (surface normal where the ray hit the material)
  // rho_d              (difuse reflectance of the material)
  // rho_s              (specular reflectance of the material)
  // s                  (shininess or Phong exponent of the material)
  //
  // Hint: Call the sample function associated with each light in the scene.
  float3 dir = make_float3(0);
  float3 L = make_float3(0);
  const float3 frac = rho_d * M_1_PIf;
  const float3 shine_frac = rho_s*M_1_PIf * (s + 2) / 2;
  float3 dir_reflect = make_float3(0);
  

  for (int i = 0; i < lights.size(); ++i) {
	  if (lights[i]->sample(hit.position, dir, L)) {
		  dir_reflect=reflect(-dir, hit.shading_normal);
		  result += (frac+shine_frac*pow(fmaxf(dot(-r.direction,dir_reflect), 0),s))* L * fmaxf(dot(dir,hit.shading_normal),0);
	  }
  }
  
  return result+ Lambertian::shade(r, hit, emit);
}
