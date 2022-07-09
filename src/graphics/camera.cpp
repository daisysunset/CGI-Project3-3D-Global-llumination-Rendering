#include "camera.h"

#include <cmath>

namespace VCL {

void Camera::InitData(const float aspect, const float fovy, const float z_near,
                      const float z_far, const float radius, const float phi,
                      const float theta, const Vec3f& target) {
  aspect_ = aspect;
  saved_radius_ = radius;
  saved_phi_ = phi;
  saved_theta_ = theta;
  saved_target_ = target;
  SetPerspective(fovy, z_near, z_far);
  SetSpherical(radius, phi, theta, target);
  UpdateData();
}

void Camera::Rotate(const float dx, const float dy) {
  static constexpr float rotate_ratio = 0.25f * PI_ / 180.0f;
  phi_ -= rotate_ratio * dx;
  theta_ -= rotate_ratio * dy;
  phi_ = std::fmod(phi_, 2.0f * PI_);
  if (phi_ < 0.0f) phi_ += 2.0f * PI_;
  theta_ = std::clamp(theta_, 0.1f, PI_ - 0.1f);
  LookAt(SphericalToCartesian(radius_, phi_, theta_), target_);
}

void Camera::Translate(const float dx, const float dy) {
  static constexpr float trans_ratio = 0.001f;
  target_ += trans_ratio * radius_ * (dy * up_ - dx * right_);
  LookAt(SphericalToCartesian(radius_, phi_, theta_), target_);
}

void Camera::Scale(const float dy) {
  static constexpr float scale_ratio = 0.05f;
  radius_ /= std::exp(scale_ratio * dy);
  radius_ = std::clamp(radius_, 0.1f, 150.0f);
  LookAt(SphericalToCartesian(radius_, phi_, theta_), target_);
}

void Camera::ResetAspect(const float aspect) {
  aspect_ = aspect;
  proj_dirty_ = true;
}

void Camera::UpdateData() {
  if (proj_dirty_) {
    float y_scale = 1.0f / std::tan(fovy_ / 2);
    float x_scale = y_scale / aspect_;
    // opengl style: z \in [-1, 1]
    proj_ << x_scale, 0, 0, 0, 0, y_scale, 0, 0, 0, 0,
        -(z_far_ + z_near_) / (z_far_ - z_near_),
        -2 * z_near_ * z_far_ / (z_far_ - z_near_), 0, 0, -1, 0;
  }
  if (view_dirty_) {
    view_ << right_.x(), right_.y(), right_.z(), -right_.dot(pos_), up_.x(),
        up_.y(), up_.z(), -up_.dot(pos_), -lookat_.x(), -lookat_.y(),
        -lookat_.z(), lookat_.dot(pos_), 0, 0, 0, 1;
  }
  if (proj_dirty_ || view_dirty_) {
    proj_view_ = proj_ * view_;
  }
  proj_dirty_ = false;
  view_dirty_ = false;
}

void Camera::LookAt(const Vec3f& pos, const Vec3f& target) {
  pos_ = pos;
  target_ = target;
  lookat_ = (target - pos).normalized();
  right_ = lookat_.cross(Vec3f::Unit(1)).normalized();
  up_ = right_.cross(lookat_).normalized();

  view_dirty_ = true;
}

void Camera::SetPerspective(const float fovy, const float z_near,
                            const float z_far) {
  fovy_ = fovy;
  z_near_ = z_near;
  z_far_ = z_far;
  proj_dirty_ = true;
}

void Camera::SetSpherical(const float radius, const float phi,
                          const float theta, const Vec3f& target) {
  radius_ = radius;
  phi_ = phi;
  theta_ = theta;
  LookAt(SphericalToCartesian(radius, phi, theta) + target, target);
}

Ray Camera::GenerateRay(const real sx, const real sy) {
  const real tx = sx * 2 - 1;
  const real ty = sy * 2 - 1;
  const real dy = tan(fovy_ / 2);
  const real dx = dy * aspect_;
  return Ray(pos_, lookat_ + ty * dy * up_ + tx * dx * right_);
}
};  // namespace VCL