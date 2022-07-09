#pragma once

#include "common/mathtype.h"

namespace VCL {

class Light {
 public:
  Vec3 position;
  Color intensity;
  Light(const Vec3 &position, const Color &intensity)
      : position(position), intensity(intensity) {}
};

} // namespace VCL
