#include "framebuffer.h"

#include "common/helperfunc.h"

namespace VCL {
Framebuffer::Framebuffer(int width, int height) {
  width_ = width;
  height_ = height;
  int size = width * height;
  color_ = new unsigned char[size * 4];
  depth_ = new float[size];
  Clear();
}

void Framebuffer::Clear() {
  int size = width_ * height_;
  Vec4f clear_color = Vec4f(0.0, 0.0, 0.0, 1.0);
  for (int i = 0; i < size; ++i) {
    color_[4 * i] = FloatToUChar(clear_color[0]);
    color_[4 * i + 1] = FloatToUChar(clear_color[1]);
    color_[4 * i + 2] = FloatToUChar(clear_color[2]);
    color_[4 * i + 3] = FloatToUChar(clear_color[3]);
  }
  for (int i = 0; i < size; ++i) {
    depth_[i] = 1.0;
  }
}
};  // namespace VCL