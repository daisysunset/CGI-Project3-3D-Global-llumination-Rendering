#pragma once

namespace VCL {
class Image {
 public:
  int width_;
  int height_;
  int channels_;
  unsigned char* buffer_ = nullptr;

  Image(){};
  Image(int width, int height, int channels);
  ~Image() {
    if (buffer_) delete[] buffer_;
  }
};
}  // namespace VCL