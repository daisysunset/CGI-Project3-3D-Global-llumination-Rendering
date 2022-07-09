#include "image.h"

namespace VCL {
Image::Image(int width, int height, int channels) {
  width_ = width;
  height_ = height;
  channels_ = channels;
  int num_elems = width * height * channels;
  buffer_ = new unsigned char[num_elems];
}
};