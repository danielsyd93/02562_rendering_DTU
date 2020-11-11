// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#ifndef FRESNEL_H
#define FRESNEL_H

inline float fresnel_r_s(float cos_theta1, float cos_theta2, float ior1, float ior2)
{
  // Compute the perpendicularly polarized component of the Fresnel reflectance
	const float num = ior1 * cos_theta1 - ior2 * cos_theta2;
	const float den = ior1 * cos_theta1 + ior2 * cos_theta2;
  return num/den;
}

inline float fresnel_r_p(float cos_theta1, float cos_theta2, float ior1, float ior2)
{
  // Compute the parallelly polarized component of the Fresnel reflectance
	const float num = ior2 * cos_theta1 - ior1 * cos_theta2;
	const float den=ior2* cos_theta1 + ior1 * cos_theta2;
  return num/den;
}

inline float fresnel_R(float cos_theta1, float cos_theta2, float ior1, float ior2)
{
  // Compute the Fresnel reflectance using fresnel_r_s(...) and fresnel_r_p(...)
	const float r1 = fresnel_r_s(cos_theta1, cos_theta2, ior1, ior2);
	const float r2= fresnel_r_p(cos_theta1, cos_theta2, ior1, ior2);
  return 0.5*(fabs(r1)* fabs(r1) + fabs(r2)*fabs(r2));
}

#endif // FRESNEL_H
