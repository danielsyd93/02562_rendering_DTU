// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <iostream>
#include <vector>
#include <algorithm>
#include <optix_world.h>
#include "HitInfo.h"
#include "ObjMaterial.h"
#include "mt_random.h"
#include "ParticleTracer.h"

#ifdef _OPENMP
  #include <omp.h>
#endif

using namespace std;
using namespace optix;

void ParticleTracer::build_maps(int no_of_caustic_particles, unsigned int max_no_of_shots)
{
  // Retrieve light sources
  const vector<Light*>& lights = scene->get_lights();
  if(lights.size() == 0)
    return;

  // Check requested photon counts
  if(no_of_caustic_particles > caustics.get_max_photon_count())
  {
    cerr << "Requested no. of caustic particles exceeds the maximum no. of particles." << endl;
    no_of_caustic_particles = caustics.get_max_photon_count();
  }

  // Choose block size
  int block = std::max(1, no_of_caustic_particles/100);

  // Shoot particles
  unsigned int nshots = 0;
  unsigned int caustics_done = no_of_caustic_particles == 0 ? 1 : 0;
  while(!caustics_done)
  {
    // Stop if we cannot find the desired number of photons.
    if(nshots >= max_no_of_shots)
    {
      cerr << "Unable to store enough particles." << endl;
      if(!caustics_done)
        caustics_done = nshots;
      break;
    }
    
    // Trace a block of photons at the time
    //#pragma omp parallel for private(randomizer)
    for(int i = 0; i < block; ++i)
    {
      // Sample a light source
      unsigned int light_idx = static_cast<unsigned int>(lights.size()*mt_random_half_open());

      // Shoot a particle from the sampled source
      trace_particle(lights[light_idx], caustics_done);
    }
    nshots += block;

    // Check particle counts
    if(!caustics_done && caustics.get_photon_count() >= no_of_caustic_particles)
      caustics_done = nshots;
  }
  cout << "Particles in caustics map: " << caustics.get_photon_count() << endl;

  // Finalize photon maps
  caustics.scale_photon_power(lights.size()/static_cast<float>(caustics_done));
  caustics.balance();
}

float3 ParticleTracer::caustics_irradiance(const HitInfo& hit, float max_distance, int no_of_particles)
{
  return caustics.irradiance_estimate(hit.position, hit.shading_normal, max_distance, no_of_particles);
}

void ParticleTracer::draw_caustics_map()
{
  caustics.draw();
}

void ParticleTracer::trace_particle(const Light* light, const unsigned int caustics_done)
{
  if(caustics_done)
    return;

  // Shoot a particle from the sampled source
  Ray r;
  HitInfo hit;
  float3 Phi;
  light->emit(r,hit,Phi);
  if(!hit.has_hit||!scene->is_specular(hit.material)){
      return;
  }
  
  // Forward from all specular surfaces
  while(scene->is_specular(hit.material) && hit.trace_depth < 4)
  {
    switch(hit.material->illum)
    {
    case 3:  // mirror materials
      {
        // Forward from mirror surfaces here
        return;
      }
      break;
    case 11: // absorbing volume
    case 12: // absorbing glossy volume
      {
        // Handle absorption here (Worksheet 8)
        if (dot(r.direction, hit.shading_normal) > 0) {
            Phi *= get_transmittance(hit);
        }
      }
    case 2:  // glossy materials
    case 4:  // transparent materials
    {
        float3 d = get_diffuse(hit);
        float probability = (d.x + d.y + d.z) / 3;
        float chi = mt_random();
        Ray ray_new;
        HitInfo hit_new;
        float R ;
        bool check = trace_refracted(r, hit, ray_new, hit_new, R);
         
        if (chi < probability)
        {
            hit_new.has_hit = false;
            if (!trace_reflected(r, hit, ray_new, hit_new))
            {
                return;
            }
            r = ray_new;
            hit = hit_new;
        }
        else if (chi < (1 - probability))
        {
            if (!check) 
            {
                return;
            }
            r = ray_new;
            hit = hit_new;
        }
        
        // Forward from transparent surfaces here
        //return; 
        
    }
      break;
    default: 
      return;
    }
  }

  // Store in caustics map at first diffuse surface
  // Hint: When storing, the convention is that the photon direction
  //       should point back toward where the photon came from.
  
  caustics.store(Phi, hit.position, -r.direction);
}

float3 ParticleTracer::get_diffuse(const HitInfo& hit) const
{
  const ObjMaterial* m = hit.material;
  return m ? make_float3(m->diffuse[0], m->diffuse[1], m->diffuse[2]) : make_float3(0.8f);
}

float3 ParticleTracer::get_transmittance(const HitInfo& hit) const
{
  if(hit.material)
  {
    // Compute and return the transmittance using the diffuse reflectance of the material.
    // Diffuse reflectance rho_d does not make sense for a specular material, so we can use 
    // this material property as an absorption coefficient. Since absorption has an effect
    // opposite that of reflection, using 1/rho_d-1 makes it more intuitive for the user.
    float3 rho_d = make_float3(hit.material->diffuse[0], hit.material->diffuse[1], hit.material->diffuse[2]);
    const float3 sigma_a = 1 / rho_d - 1;
    return expf(-sigma_a * hit.dist);
  }
  return make_float3(1.0f);
}
