#pragma once

#include "common/mathtype.h"

namespace VCL {
class Framebuffer {
 public:
  int width_;
  int height_;
  unsigned char* color_ = nullptr;
  float* depth_ = nullptr;

  Framebuffer(){};
  Framebuffer(int width, int height);
  ~Framebuffer() {
    if (color_) delete[] color_;
    if (depth_) delete[] depth_;
  }
  void Clear();
};
};  // namespace VCL