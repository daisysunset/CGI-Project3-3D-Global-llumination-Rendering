#pragma once

#include "graphics/material.h"

namespace VCL {

class Object
{
public:

  const Material *mat_;

public:

  Object(const Material *const mat) : mat_(mat) { };

  virtual ~Object() = default;

  const Material *Mat() const { return mat_; }

  virtual real Intersect(const Ray &ray) const = 0;

  virtual Vec3 ClosestNormal(const Vec3 &pos) const = 0;
};

class Plane : public Object
{
protected:

  const Vec3 pos_;
  const Vec3 n_;

public:

  Plane(const Material *const mat, const Vec3 &pos, const Vec3 &dir) :
    Object(mat),
    pos_(pos),
    n_(dir.normalized())
  { }

  virtual ~Plane() = default;

  virtual real Intersect(const Ray &ray) const override
  {
    real dist = std::numeric_limits<real>::infinity();
    real tmp = ray.dir_.dot(n_);
    if(tmp > - EPS_) return dist;
    real t =  (pos_ - ray.ori_).dot(n_) / tmp;
    if (t < 0 ) return dist;
    else return t;
  }

  virtual Vec3 ClosestNormal(const Vec3 &position) const { return n_; }
};

class Sphere : public Object
{
protected:

  const Vec3 cen_;
  const real rad_;

public:

  Sphere(const Material *const mat, const Vec3 &cen, const real rad) :
    Object(mat),
    cen_(cen),
    rad_(rad)
  { }

  virtual ~Sphere() = default;

  virtual real Intersect(const Ray &ray) const override
  {
    real dist = std::numeric_limits<real>::infinity();
    real A = ray.dir_.dot(ray.dir_);
    real B = 2 * ray.dir_.dot(ray.ori_ - cen_);
    real C = (ray.ori_ - cen_).dot(ray.ori_ - cen_) - rad_ * rad_;
    real discriminate = B * B - 4 * A * C;
    if (discriminate < 0)
        return dist;
    real t0 = ( - B + sqrtf(discriminate)) / (2 * A) ;
    real t1 = ( - B - sqrtf(discriminate)) / (2 * A) ;
    if(t1 >= 0) return t1;
    if (t0 >= 0) return t0;//从里面射出去??
    else return dist;
  }

  virtual Vec3 ClosestNormal(const Vec3 &pos) const { return (pos - cen_).normalized(); }
};

class CapeOutside: public Object{
protected:

  Vec3 v_[7];
  const real rad_;
  const int idx[6][3] = {{0,1,2},{0,2,3},{0,3,4},{0,4,5},{0,5,6},{0,6,1}};
  Vec3 n_[6];

public:

  CapeOutside(const Material *const mat, const Vec3 v0,real rad):
  Object(mat),
  rad_(rad)
  {
    // other six nodes
    real h = -rad/2.0;
    v_[0] = v0;
    v_[1] = v0 + Vec3(rad_, h, 0);
    v_[2] = v0 + Vec3(rad_/2.0, h, -rad_/2.0*std::sqrt(3));
    v_[3] = v0 + Vec3(-rad_/2.0, h, -rad_/2.0*std::sqrt(3));
    v_[4] = v0 + Vec3(-rad_, h, 0);
    v_[5] = v0 + Vec3(-rad_/2.0, h, rad_/2.0*std::sqrt(3));
    v_[6] = v0 + Vec3(rad_/2.0, h, rad_/2.0*std::sqrt(3));
    // calculate normal
    for(int i = 0; i < 6; ++i){
      Vec3 a = v_[idx[i][1]] - v_[idx[i][0]];
      Vec3 b = v_[idx[i][2]] - v_[idx[i][0]];
      n_[i] =  (a.cross(b)).normalized();//dev
    }
  }

  virtual ~CapeOutside() = default;

  bool SameSide(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& p) const {
    Vec3 ab = b - a;
    Vec3 ac = c - a;
    Vec3 ap = p - a;
    return (ab.cross(ac)).dot(ab.cross(ap)) >= 0;
  }

  bool PointInTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& p) const{
    return SameSide(a,b,c,p) && SameSide(b,c,a,p) && SameSide(c,a,b,p);
  }

  virtual real Intersect(const Ray &ray) const override
  {
    real dist = std::numeric_limits<real>::infinity();
    for (int i = 0; i < 6; ++i){
      //assume it is a plane and find a intersect and distance
      real cos_theta = ray.dir_.dot(n_[i]);
      if(cos_theta > EPS_) continue;
      real t =  (v_[0] - ray.ori_).dot(n_[i]) / cos_theta;
      // find if the intersect is in the triangle
      if(t > 0 && t < dist){
        Vec3 pos = ray.ori_ + ray.dir_ * t;
        if (PointInTriangle(v_[idx[i][0]],v_[idx[i][1]],v_[idx[i][2]],pos))
          dist = t;
      }
    }
    return dist;
  }

  virtual Vec3 ClosestNormal(const Vec3 &pos) const{
    for (int i = 0; i < 6; ++i){
      if (PointInTriangle(v_[idx[i][0]],v_[idx[i][1]],v_[idx[i][2]],pos))
        return n_[i];
    }
  }
};

class CapeInside: public Object{
protected:

  Vec3 v_[7];
  const real rad_;
  const int idx[6][3] = {{0,1,2},{0,2,3},{0,3,4},{0,4,5},{0,5,6},{0,6,1}};
  Vec3 n_[6];

public:

  CapeInside(const Material *const mat, const Vec3 v0,real rad):
  Object(mat),
  rad_(rad)
  {
    // other six nodes
    real h = -rad/2.0;
    v_[0] = v0;
    v_[1] = v0 + Vec3(rad_, h, 0);
    v_[2] = v0 + Vec3(rad_/2.0, h, -rad_/2.0*std::sqrt(3));
    v_[3] = v0 + Vec3(-rad_/2.0, h, -rad_/2.0*std::sqrt(3));
    v_[4] = v0 + Vec3(-rad_, h, 0);
    v_[5] = v0 + Vec3(-rad_/2.0, h, rad_/2.0*std::sqrt(3));
    v_[6] = v0 + Vec3(rad_/2.0, h, rad_/2.0*std::sqrt(3));
    // calculate normal
    for(int i = 0; i < 6; ++i){
      Vec3 a = v_[idx[i][1]] - v_[idx[i][0]];
      Vec3 b = v_[idx[i][2]] - v_[idx[i][0]];
      n_[i] =  -(a.cross(b)).normalized();//dev
    }
  }

  virtual ~CapeInside() = default;

  bool SameSide(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& p) const {
    Vec3 ab = b - a;
    Vec3 ac = c - a;
    Vec3 ap = p - a;
    return (ab.cross(ac)).dot(ab.cross(ap)) >= 0;
  }

  bool PointInTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& p) const{
    return SameSide(a,b,c,p) && SameSide(b,c,a,p) && SameSide(c,a,b,p);
  }

  virtual real Intersect(const Ray &ray) const override
  {
    real dist = std::numeric_limits<real>::infinity();
    for (int i = 0; i < 6; ++i){
      //assume it is a plane and find a intersect and distance
      real cos_theta = ray.dir_.dot(n_[i]);
      if(cos_theta > EPS_) continue;
      real t =  (v_[0] - ray.ori_).dot(n_[i]) / cos_theta;
      // find if the intersect is in the triangle
      if(t > 0 && t < dist){
        Vec3 pos = ray.ori_ + ray.dir_ * t;
        if (PointInTriangle(v_[idx[i][0]],v_[idx[i][1]],v_[idx[i][2]],pos))
          dist = t;
      }
    }
    return dist;
  }

  virtual Vec3 ClosestNormal(const Vec3 &pos) const{
    for (int i = 0; i < 6; ++i){
      if (PointInTriangle(v_[idx[i][0]],v_[idx[i][1]],v_[idx[i][2]],pos))
        return n_[i];
    }
  }
};

class Cube : public Object
{
protected:

  const Vec3 cen_;
  const real l_;
  const real h_;
  const real w_;

  Vec3 v_[6];
  Vec3 n_[6] = {Vec3(1, 0, 0), Vec3(-1, 0, 0), Vec3(0, 1, 0), Vec3(0, -1, 0), Vec3(0, 0, 1), Vec3(0, 0, -1)};

public:

  Cube(const Material *const mat, const Vec3 &cen, const real l, const real h, const real w) :
    Object(mat),
    cen_(cen),
    l_(l),
    h_(h),
    w_(w)
  {
    v_[0] = cen_ + Vec3(l_/2.0, 0, 0);
    v_[1] = cen_ - Vec3(l_/2.0, 0, 0);
    v_[2] = cen_ + Vec3(0, h_/2.0, 0);
    v_[3] = cen_ - Vec3(0, h_/2.0, 0);
    v_[4] = cen_ + Vec3(0, 0, w_/2.0);
    v_[5] = cen_ - Vec3(0, 0, w_/2.0);
  }

  virtual ~Cube() = default;

  virtual real Intersect(const Ray &ray) const override
  {
    real dist = std::numeric_limits<real>::infinity();
    for (int i = 0; i < 6; ++i){
      real cos_theta = ray.dir_.dot(n_[i]);
      if(cos_theta >= EPS_) continue;
      real t = (v_[i] - ray.ori_).dot(n_[i]) / cos_theta;

      if (t > 0 && t < dist){
        Vec3 pos = ray.ori_ + ray.dir_ * t;
        Vec3 dis = pos - cen_;
        if ((abs(dis[0]) < l_/2.0 + EPS_)  && (abs(dis[1]) < h_/2.0 + EPS_) && (abs(dis[2]) < w_/2.0 + EPS_)) 
          dist = t;
      }
    }
    return dist;
  }

  virtual Vec3 ClosestNormal(const Vec3 &pos) const { 
    Vec3 dis = pos - cen_;
    if (abs(dis[0] - l_/2.0) < EPS_) return (n_[0]).normalized();
    if (abs(dis[0] + l_/2.0) < EPS_) return (n_[1]).normalized();
    if (abs(dis[1] - h_/2.0) < EPS_) return (n_[2]).normalized();
    if (abs(dis[1] + h_/2.0) < EPS_) return (n_[3]).normalized();
    if (abs(dis[2] - w_/2.0) < EPS_) return (n_[4]).normalized();
    if (abs(dis[2] + w_/2.0) < EPS_) return (n_[5]).normalized();
  }
};

}

