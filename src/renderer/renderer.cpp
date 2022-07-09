#include "renderer.h"

#include "common/helperfunc.h"
#include "graphics/globillum.h"
#include <iostream>

namespace VCL {
void Renderer::Init(const std::string& title, int width, int height,const bool MonteCarlo) {
  width_ = width;
  height_ = height;
  MonteCarlo_ = MonteCarlo;
  InitPlatform();
  window_ = CreateVWindow(title, width_, height_, this);
  framebuffer_ = new Framebuffer(width_, height_);
  
  camera_ = new Camera;
  const float c_y = 1.5;
  const float c_z = 1.5 + 1.5 * std::sqrt(2);
  camera_->InitData((float)width_ / height_, 0.25f * PI_, 1.0f, 1000.0f, c_z,
                    0.0f, 0.5f * PI_, Vec3f(0, c_y, 0));

  auto &mats = scene_.mats_;
	auto &objs = scene_.objs_;
  auto &lights = scene_.lights_;
	// Initialize materials.
	mats["ceiling"] = std::make_unique<Material>(Color(280, 10, 10)/255.0);
	mats["floor"] = std::make_unique<Material>(Color(0,255,127)/255.0);
	mats["front"] = std::make_unique<Material>(Color(0.3, 0.8, 0.8));
  mats["end"] = std::make_unique<Material>(Color(0.8, 0.8, 0.3));
	mats["side"] = std::make_unique<Material>(Color(0, 0.1, 1));//blue

  if (MonteCarlo_){
    mats["mirror"] = std::make_unique<Material>(Color(37.2f, 24.4f, 13.2f) / 255, Color(.6f, .6f, .6f), -1);
    mats["yellow_light"] = std::make_unique<Material>(Color(1, 1, 0.5), true);
  }
  else {
    mats["mirror"] = std::make_unique<Material>(Color(0, 0, 0) , Color(1.6f, 1.6f, 1.6f), 30);
    mats["yellow_light"] = std::make_unique<Material>(Color(10, 10, 5), true);
  }
	mats["light"] = std::make_unique<Material>(Color(20, 20, 20), true);
  mats["small_light"] = std::make_unique<Material>(Color(3, 3, 3), true);
 

  mats["metal"] = std::make_unique<Material>(Color(0, 0, 0), Color(.8f, .8f, .8f), 30);
  if (MonteCarlo_)
    mats["lampo"] = std::make_unique<Material>(Color(0.8, 0.8, 0));
  else
    mats["lampo"] = std::make_unique<Material>(Color(1.8, 1.8, 0));
  mats["lampi"] = std::make_unique<Material>(Color(0.1, 0.1, 0));
  mats["stick"] = std::make_unique<Material>(Color(1.8, 1.8, 0.1));
  mats["cube"] = std::make_unique<Material>(Color(0, 0, 0.5),Color(.01f, .01f, .01f), 0);

	// Set boundaries.
	objs.emplace_back(std::make_unique<Plane>(
		mats["ceiling"].get(),
		Vec3(0, 3, 0), Vec3(0, -1, 0)));
	objs.emplace_back(std::make_unique<Plane>(
		mats["floor"].get(),
		Vec3(0, 0, 0), Vec3(0, 1, 0)));
	objs.emplace_back(std::make_unique<Plane>(
		mats["front"].get(),
		Vec3(0, 0, -4), Vec3(0, 0, 1)));
  objs.emplace_back(std::make_unique<Plane>(
		mats["end"].get(),
		Vec3(0, 0, 0), Vec3(0, 0, -1)));
	objs.emplace_back(std::make_unique<Plane>(//left
		mats["side"].get(),
		Vec3(-2, 0, 0), Vec3(1, 0, 0)));
	objs.emplace_back(std::make_unique<Plane>(//right
		mats["mirror"].get(),
		Vec3(2, 0, 0), Vec3(-1, 0, 0)));
 
	// Set the light.
  //---random---
  Vec3 dotlight1 = Vec3(0.8+0.2*rand_01() ,1.6+0.2*rand_01(),-4);
  Vec3 dotlight2 = Vec3(-dotlight1[0], dotlight1[1],dotlight1[2]);
  //---random---
	const real dl = real(2) / 3;
	const real rl = 10;
	const real hl = std::sqrt(rl * rl - dl * dl);
  const real dl2 = 0.02;
	const real rl2 = 0.1;
	const real hl2 = std::sqrt(rl2 * rl2 - dl2 * dl2);

  objs.emplace_back(std::make_unique<Sphere>( mats["light"].get(),Vec3(0, 3 + hl, -2), rl));
  lights.emplace_back(std::make_unique<Light>(Vec3(0, 3, -2),Color(1, 1, 1) * 2.0));
  
  objs.emplace_back(std::make_unique<Sphere>( mats["small_light"].get(), dotlight1 + Vec3( 0, 0, - hl2), rl2));
  lights.emplace_back(std::make_unique<Light>(dotlight1, Color(1, 1, 1) * 2.0));
  
  objs.emplace_back(std::make_unique<Sphere>( mats["small_light"].get(), dotlight2 + Vec3( 0, 0, - hl2), rl2));
  lights.emplace_back(std::make_unique<Light>(dotlight2, Color(1, 1, 1) * 2.0));
  
  // Set internal objects.
  //---random---
  real tmpz = rand_01();
  real ball_rad = 0.4 + 0.2 * rand_01();
  Vec3 ball ;
  Vec3 cube_;

  if(tmpz < 0.33){
    ball = Vec3(0, ball_rad, -3);
    cube_ = Vec3(-1, real(0.8), -1);
  }
  else if (tmpz < 0.67){
    ball = Vec3(0, ball_rad, -1.2);
    cube_ = Vec3(-0.2, real(0.8), -3);
  }
  else{
    ball = Vec3(1.35, ball_rad, -2.5);
    cube_ = Vec3(-1, real(0.8), -2);
  }
  //std::cout<<tmpz<<std::endl;
  //std::cout<<ball_rad<<std::endl;
  //---random---

  objs.emplace_back(std::make_unique<Cube>(mats["cube"].get(), cube_, real(.6),real(1.6),real(.8)));
  objs.emplace_back(std::make_unique<Sphere>( mats["metal"].get(), ball, ball_rad));

  // lamp position
  //---random---
  Vec3 lamp_o = Vec3(1.35+0.05*rand_01(), real(1.5), -1.2-0.1*rand_01());
  //---random---
  Vec3f lamp_p = lamp_o + Vec3(0, -0.25, 0);
  Vec3f lamp_c = Vec3(lamp_o[0], 0.58,lamp_o[2] );
  Vec3f lamp_d = Vec3(lamp_o[0], 0.025,lamp_o[2] );

  objs.emplace_back(std::make_unique<CapeOutside>(mats["lampo"].get(),lamp_o, real(.5)));
  objs.emplace_back(std::make_unique<CapeInside>(mats["lampi"].get(), lamp_o+Vec3(0, -0.01, 0), real(.5)));

  objs.emplace_back(std::make_unique<Sphere>( mats["yellow_light"].get(), lamp_p , real(.15)));
  lights.emplace_back(std::make_unique<Light>( lamp_p  , Color(1, 1, 0) * 0.35));

  objs.emplace_back(std::make_unique<Cube>( mats["stick"].get(), lamp_c , real(0.05),real(1.06),real(0.05)));
  objs.emplace_back(std::make_unique<Cube>( mats["stick"].get(), lamp_d , real(0.4),real(0.05),real(0.4)));
  
  scene_.ambient_light_ = Color(0.05, 0.05, 0.05);
}

void Renderer::Progress(int &x, int &y, Color **buffer, int **cnt) {
  const real dx = real(1) / width_;
	const real dy = real(1) / height_;

  const real lx = dx * x;
  const real ly = dy * y;

  const real sx = lx + rand01() * dx;
  const real sy = ly + rand01() * dy;

  if (!MonteCarlo_) {
    buffer[y][x] += (GlobIllum::RayTrace(scene_, camera_->GenerateRay(sx, sy)) - buffer[y][x]) / (++cnt[y][x]);    
  }
  else {
    buffer[y][x] += (GlobIllum::PathTrace(scene_, camera_->GenerateRay(sx, sy)) - buffer[y][x]) / (++cnt[y][x]);
  }

  int idx = (y * width_ + x) * 4;
  for (int i = 0; i < 3; i++) framebuffer_->color_[idx + i] = std::round(std::pow(std::clamp(buffer[y][x][i], 0.0f, 1.0f), 1 / 2.2f) * 255);

  x++;
  if (x == width_) {
    x = 0;
    y++;
    if (y == height_) y = 0;
  }
}

void Renderer::MainLoop() {

  int x;
  int y;

  Color **buffer = new Color *[height_];
  int **cnt = new int *[height_];

  for (y = 0; y < height_; y++) {
    buffer[y] = new Color[width_];
    cnt[y] = new int[width_];
    for (x = 0; x < width_; x++) {
      buffer[y][x] = Color::Zero();
      cnt[y][x] = 0;
    }
  }

  x = y = 0;
  int idx = 0;
  const int buffer_size = height_ * width_;
  int patch_size = 50000;
  patch_size = patch_size > buffer_size ? buffer_size : patch_size;
  while (!window_->should_close_) {
    PollInputEvents();

    // parallel version
    # pragma omp parallel for
    for (int i = 0; i < patch_size; ++i) {
      int p = (idx + i) % buffer_size;
      int px = p % width_;
      int py = p / width_;
      Progress(px, py, buffer, cnt);
    }
    idx = (idx + patch_size) % buffer_size;

    window_->DrawBuffer(framebuffer_);
  }

  for (y = 0; y < height_; y++) {
    delete[] buffer[y];
    delete[] cnt[y];
  }
  delete[] buffer;
  delete[] cnt;
}

void Renderer::Destroy() {
  if (camera_) delete camera_;
  if (framebuffer_) delete framebuffer_;
  window_->Destroy();
  if (window_) delete window_;
  DestroyPlatform();
}
};  // namespace VCL