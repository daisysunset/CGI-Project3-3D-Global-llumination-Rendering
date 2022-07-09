#include "scene.h"
#include <iostream>
namespace VCL {

Object *Scene::Intersect(const Ray &ray, Vec3 &pos) const
{
  Object *collider = nullptr;
  real dist = std::numeric_limits<real>::infinity();
  for (const auto &object : objs_) {
    const real temp = object->Intersect(ray);
    if (temp < dist) {
      const Vec3 pos_t = ray.ori_ + ray.dir_ * temp;
      if (((POSMIN_ - pos_t).array() <= EPS_).all() && ((pos_t - POSMAX_).array() <= EPS_).all()) {
        dist = temp;
        pos = pos_t;
        collider = object.get();
      }
    }
  }
  pos = pos.cwiseMax(POSMIN_).cwiseMin(POSMAX_);
  return collider;
}

}