#pragma once

#include <string>
#include "graphics/image.h"
#include "graphics/framebuffer.h"

namespace VCL {
class VWindow {
 public:
  bool should_close_= false;
  Image* surface_ = nullptr;
  // keys
  // buttoms
  // callbacks
  virtual void Init(const std::string& title, int& width, int& height, void* renderer) = 0;
  virtual void Destroy() = 0;
  virtual void DrawBuffer(Framebuffer* buffer) = 0;
};

void InitPlatform();
void DestroyPlatform();
VWindow* CreateVWindow(const std::string& title, int& width, int& height, void* renderer);

void PollInputEvents();
}  // namespace VCL