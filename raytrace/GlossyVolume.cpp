// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "HitInfo.h"
#include "int_pow.h"
#include "GlossyVolume.h"

using namespace optix;

#ifndef M_1_PIf
#define M_1_PIf 0.31830988618379067154
#endif

float3 GlossyVolume::shade(const Ray& r, HitInfo& hit, bool emit) const
{
  // Compute the specular part of the glossy shader and attenuate it
  // by the transmittance of the material if the ray is inside (as in
  // the volume shader).

	//const float3 rho_d = get_diffuse(hit);
	const float3 rho_s = get_specular(hit);
	const  float s = get_shininess(hit);
	float3 result = make_float3(0);
	/*if (length(rho_d + rho_s) > 1) {
		return result;
	}*/
	float3 dir = make_float3(0);
	float3 L = make_float3(0);
	//const float3 frac = rho_d * M_1_PIf;
	const float3 shine_frac = rho_s * M_1_PIf * (s + 2) / 2;
	float3 dir_reflect = make_float3(0);


	for (int i = 0; i < lights.size(); ++i) {
		if (lights[i]->sample(hit.position, dir, L)) {
			dir_reflect = reflect(-dir, hit.shading_normal);
			result += (/*frac+*/ shine_frac * pow(fmaxf(dot(-r.direction, dir_reflect), 0), s)) * L * fmaxf(dot(dir, hit.shading_normal), 0);
		}
	}



	return Volume::shade(r, hit, emit) + result;
}
