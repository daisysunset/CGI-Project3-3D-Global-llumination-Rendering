#pragma once

#include "graphics/object.h"
#include "graphics/light.h"

#include <map>
#include <memory>
#include <vector>

namespace VCL {

class Scene
{
public:

  Color ambient_light_;
  std::vector<std::unique_ptr<Object>> objs_;
  std::map<std::string, std::unique_ptr<Material>> mats_;
  std::vector<std::unique_ptr<Light>> lights_;

public:

  Scene() = default;
  virtual ~Scene() = default;

  Object *Intersect(const Ray &ray, Vec3 &pos) const;
};

}
