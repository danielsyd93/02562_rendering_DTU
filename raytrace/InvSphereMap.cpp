#include <optix_world.h>
#include "InvSphereMap.h"

using namespace optix;

void InvSphereMap::project_direction(const float3& d, float& u, float& v) const
{
	float theta = acos(d.z);
	float phi=atan2(d.y,d.x);
	u = theta * M_1_PIf;
	v = phi / 2 * M_1_PIf;


  // Implement an inverse sphere map here.
}