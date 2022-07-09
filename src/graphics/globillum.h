#include "graphics/scene.h"

namespace VCL::GlobIllum {

Color RayTrace(const Scene &scene, Ray ray);
Color PathTrace(const Scene &scene, Ray ray);

}
