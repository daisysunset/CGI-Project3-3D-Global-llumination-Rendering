#include <iostream>
#include "renderer/renderer.h"
#include <spdlog/spdlog.h>

using namespace VCL;

int main() {
  spdlog::set_pattern("[%^%l%$] %v");
#ifdef NDEBUG
  spdlog::set_level(spdlog::level::info);
#else
  spdlog::set_level(spdlog::level::debug);
#endif
  Renderer renderer;
  // switch between ray-tracing and path-tracing
  const bool MonteCarlo = false;
  
  renderer.Init("Visual Computing", 800, 600,MonteCarlo);
  renderer.MainLoop();
  renderer.Destroy();
  return 0;
}