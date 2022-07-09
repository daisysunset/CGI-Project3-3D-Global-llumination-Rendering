#pragma once

#include <vector>

#include "common/mathtype.h"

namespace VCL {
class Camera {
 public:
  float z_near_;
  float z_far_;
  float fovy_;
  float aspect_;

  Vec3f pos_;
  Vec3f up_;
  Vec3f right_;
  Vec3f lookat_;

  Vec3f target_;
  float radius_;
  float phi_;
  float theta_;
  Vec3f saved_target_;
  float saved_radius_;
  float saved_phi_;
  float saved_theta_;

  bool proj_dirty_ = false;
  bool view_dirty_ = false;
  Mat4f proj_;
  Mat4f view_;

  Mat4f proj_view_;

  void InitData(const float aspect, const float fovy, const float z_near,
                const float z_far, const float radius, const float phi,
                const float theta, const Vec3f& target);
  void Rotate(const float dx, const float dy);
  void Translate(const float dx, const float dy);
  void Scale(const float dy);
  void ResetAspect(const float aspect);
  void UpdateData();

  void LookAt(const Vec3f& pos, const Vec3f& target);
  void SetPerspective(const float fovy, const float z_near, const float z_far);
  void SetSpherical(const float radius, const float phi, const float theta,
                    const Vec3f& target);

  static Vec3f SphericalToCartesian(const float radius, const float phi,
                                    const float theta) {
    return Vec3f(std::sin(theta) * std::sin(phi), std::cos(theta),
                 std::sin(theta) * std::cos(phi)) *
           radius;
  }

  Ray GenerateRay(const real sx, const real sy); // sx, sy in [0, 1]
};
};  // namespace VCL