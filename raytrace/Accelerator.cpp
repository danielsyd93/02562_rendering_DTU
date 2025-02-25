// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <vector>
#include <optix_world.h>
#include "AccObj.h"
#include "Object3D.h"
#include "Plane.h"
#include "HitInfo.h"
#include "Accelerator.h"

using namespace std;
using namespace optix;

Accelerator::~Accelerator()
{
  for(unsigned int i = 0; i < primitives.size(); ++i)
    delete primitives[i];
}

void Accelerator::init(const vector<Object3D*>& geometry, const vector<const Plane*>& scene_planes)
{
  for(unsigned int i = 0; i < geometry.size(); ++i)
  {
    Object3D* obj = geometry[i];
    unsigned int no_of_prims = primitives.size();
    primitives.resize(no_of_prims + obj->get_no_of_primitives());
    for(unsigned int j = 0; j < obj->get_no_of_primitives(); ++j)
      primitives[j + no_of_prims] = new AccObj(obj, j);
  }
  planes = scene_planes;
}

bool Accelerator::closest_hit(optix::Ray& r, HitInfo& hit) const
{
  closest_plane(r, hit);

  // Loop through all the primitives to find the closest intersection (if any).
  //
  // Input:  r    (the ray to be checked for intersection)
  // Output: hit  (info about the ray-surface intersection)
  // Return: True if the ray intersects something, false otherwise
  //
  // Relevant data fields that are available (see Accelerator.h and OptiX math library reference)
  // primitives   (array of primitive objects)
  // r.tmax       (maximum intersection distance allowed)
  // hit.dist     (distance to the first intersection along the ray)
  // hit.has_hit  (true if the ray hit a surface, false otherwise)
  //
  // Hint: Call the intersect(...) function for each primitive object in
  //       the scene. See the functions below this one for inspiration.
  
  //bool has_hit = false;

  for (int i = 0; i< primitives.size(); ++i) {
      if (primitives[i]->geometry->intersect(r, hit, primitives[i]->prim_idx)) {
          r.tmax = hit.dist;
      }
  }
  



  return hit.has_hit;

}

bool Accelerator::any_hit(optix::Ray& r, HitInfo& hit) const
{
  if(!any_plane(r, hit))
  {
    unsigned int i = 0;
    while(i < primitives.size() && !hit.has_hit)
    {
      AccObj* obj = primitives[i++];
      obj->geometry->intersect(r, hit, obj->prim_idx);
    }
  }
  return hit.has_hit;
}

void Accelerator::closest_plane(Ray& r, HitInfo& hit) const
{
  for(unsigned int i = 0; i < planes.size(); ++i)
    if(planes[i]->intersect(r, hit, 0))
      r.tmax = hit.dist;
}

bool Accelerator::any_plane(Ray& r, HitInfo& hit) const
{
  for(unsigned int i = 0; i < planes.size(); ++i)
    if(planes[i]->intersect(r, hit, 0))
      return true;
  return false;
}
