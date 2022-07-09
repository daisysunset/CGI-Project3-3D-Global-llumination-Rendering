#pragma once

#include "common/mathtype.h"

namespace VCL {

class Material
{
public:

  Color k_d_; // diffuse reflection constant
  Color k_s_ = Color::Zero(); // specular reflection constant
  real alpha_ = 0; // shininess constant
  bool emissive_;


public:

  Material(const Color &k_d, const bool emissive = false) :
    k_d_(k_d),
    emissive_(emissive)
  { }

  Material(const Color &k_d, const Color &k_s, const real alpha) :
    k_d_(k_d),
    k_s_(k_s),
    emissive_(false),
    alpha_(alpha)
  { }

  virtual ~Material() = default;
};

}
