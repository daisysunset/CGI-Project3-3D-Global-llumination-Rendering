#include "globillum.h"

#include "common/helperfunc.h"
#include "light.h"

#include <iostream>

namespace VCL::GlobIllum {

Vec3 AxisAngle(const Vec3 &w, const real cos2theta, const real phi)
{
	const real cos_theta = std::sqrt(cos2theta);
	const real sin_theta = std::sqrt(1 - cos2theta);
	const Vec3 u = (std::abs(w[0]) > real(.1) ? Vec3(0, 1, 0) : Vec3(1, 0, 0)).cross(w).normalized();
	const Vec3 v = w.cross(u);
	return (u * std::cos(phi) * sin_theta + v * std::sin(phi) * sin_theta + w * cos_theta).normalized();
}

Vec3 Sample(const Material *const mat, const Vec3 &n, const Vec3 &wi, Color &weight)
{
  const real R = mat->k_d_.mean() / (mat->k_d_.mean() + mat->k_s_.mean());
  const real r0 = rand01();
  if (r0 < R) { // sample diffuse ray
    weight = mat->k_d_.any() ? mat->k_d_ / R : Color(0, 0, 0);
    return AxisAngle(n, rand01(), rand01() * 2 * PI_);
  }
  else { // sample specular ray
    if (mat->alpha_ >= 0) {
      const Vec3 d = AxisAngle(n * 2 * n.dot(wi) - wi, std::pow(rand01(), real(2) / (mat->alpha_ + 2)), rand01() * 2 * PI_);
      weight = n.dot(d) <= 0 || !mat->k_s_.any() ? Color(0, 0, 0) : mat->k_s_ / (1 - R);
      return d;
    }
    else { // for ideal mirrors
      weight = mat->k_s_.any() ? mat->k_s_ / (1 - R) : Color(0, 0, 0);
      return n * 2 * n.dot(wi) - wi;
    }
  }
}

Color RayTrace(const Scene &scene, Ray ray)// eye-ray
{
  Color color(0, 0, 0);
  Color weight(1, 1, 1);
  std::vector<Light> lights;

  for (int depth = 0; depth < 10; depth++) {
    lights.clear();//光线
    Vec3 pos;
    const Object *obj = scene.Intersect(ray, pos);// eye-ray，交点，物体
    if (!obj) return color;
    auto mat = obj->Mat();//物体材质
    const Vec3 n = obj->ClosestNormal(pos);//物体法向

    // Lights
    for (const auto& tlight : scene.lights_) {// 场景中的光源，有两个
      Vec3 test_pos;
      const Ray test_ray(pos + 0.01 * (tlight->position - pos), (tlight->position - pos).normalized());// shadow ray
      const Object * test_obj = scene.Intersect(test_ray, test_pos);
      if (test_obj && test_obj->Mat()->emissive_) {//打到光球上，获得光线
        lights.push_back(*tlight);
      }
    }

    // Phong shading
    Color result(0, 0, 0);
    for(std::vector<Light>::iterator it = lights.begin(); it != lights.end(); ++it){//不在阴影里
      Vec3 light = (it->position - pos).normalized();
      Vec3 reflected_light =  2 * n * n.dot(light) - light;
      Color l = it->intensity / (it->position - pos).dot(it->position - pos);
      result += mat->k_d_ * l * (light.dot(n) > 0? light.dot(n):0); // diffuse
      result += mat->k_s_ * l * std::pow (reflected_light.dot(ray.dir_),mat->alpha_) ; // specular
    }
    result += scene.ambient_light_ * mat->k_d_;// ambient - 无论是否在阴影里

    // accumulate color
    Color R = mat->k_s_ * 0.5;
    color += weight * (Color(1, 1, 1) - R) * result;
    weight *= R;
    
    // generate new ray
    // reflected_ray  specularly reflective
    ray.dir_ = ray.dir_ - 2 * ray.dir_.dot(n) * n;
    ray.ori_ = pos + 0.00001 * ray.dir_;
  }

  return color;
}

Color PathTrace(const Scene &scene, Ray ray)
{
  Color color(1, 1, 1);
  
  for (int depth = 0; depth < 5; depth++) {
    Vec3 pos;
    const Object *obj = scene.Intersect(ray, pos);
    if (!obj) {
      return Color(0,0,0);
    }
    if (obj->Mat()->emissive_) {
      return color * obj->Mat()->k_d_;
    }

    Color  weight(1,1,1);
    ray.dir_ = Sample(obj->Mat(), obj->ClosestNormal(pos), -ray.dir_, weight);
    ray.ori_ = pos + 0.01 * ray.dir_;

    if (!weight.any()) return weight;
    else {
      color *= weight;
    }
  }

  return Color(0,0,0);
}

}