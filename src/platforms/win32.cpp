#include <direct.h>
#include <spdlog/spdlog.h>
#include <windows.h>

#include "graphics/platform.h"
#include "renderer/renderer.h"

namespace VCL {
static int g_initialized = 0;

#ifdef UNICODE
static const wchar_t* const WINDOW_CLASS_NAME = L"Class";
static const wchar_t* const WINDOW_ENTRY_NAME = L"Entry";
#else
static const char* const WINDOW_CLASS_NAME = "Class";
static const char* const WINDOW_ENTRY_NAME = "Entry";
#endif

class WinWindow : public VWindow {
 public:
  HWND handle_;
  HDC memory_dc_;
  virtual void Init(const std::string& title, int& width, int& height,
                    void* renderer);
  virtual void Destroy();
  virtual void DrawBuffer(Framebuffer* buffer);
};

static LRESULT CALLBACK ProcessMessage(HWND hWnd, UINT uMsg, WPARAM wParam,
                                       LPARAM lParam) {
  Renderer* renderer = (Renderer*)GetProp(hWnd, WINDOW_ENTRY_NAME);
  if (renderer == NULL) {
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
  } else if (uMsg == WM_CLOSE) {
    renderer->window_->should_close_ = true;
    return 0;
  } /* else if (uMsg == WM_LBUTTONDOWN) {
    POINT point;
    GetCursorPos(&point);
    ScreenToClient(reinterpret_cast<WinWindow*>(renderer->window_)->handle_,
                   &point);
    renderer->last_mouse_pos_ = Vec2f(float(point.x), float(point.y));
    renderer->button_pressed_[size_t(BUTTON::Left)] = true;
  } else if (uMsg == WM_LBUTTONUP) {
    renderer->button_pressed_[size_t(BUTTON::Left)] = false;
  } else if (uMsg == WM_RBUTTONDOWN) {
    POINT point;
    GetCursorPos(&point);
    ScreenToClient(reinterpret_cast<WinWindow*>(renderer->window_)->handle_,
                   &point);
    renderer->last_mouse_pos_ = Vec2f(float(point.x), float(point.y));
    renderer->button_pressed_[size_t(BUTTON::Right)] = true;
  } else if (uMsg == WM_RBUTTONUP) {
    renderer->button_pressed_[size_t(BUTTON::Right)] = false;
  } else if (uMsg == WM_MOUSEMOVE) {
    POINT point;
    GetCursorPos(&point);
    ScreenToClient(reinterpret_cast<WinWindow*>(renderer->window_)->handle_,
                   &point);
    if (renderer->button_pressed_[size_t(BUTTON::Left)]) {
      renderer->camera_->Rotate(float(point.x) - renderer->last_mouse_pos_.x(),
                                float(point.y) - renderer->last_mouse_pos_.y());
    }
    if (renderer->button_pressed_[size_t(BUTTON::Right)]) {
      renderer->camera_->Translate(
          float(point.x) - renderer->last_mouse_pos_.x(),
          float(point.y) - renderer->last_mouse_pos_.y());
    }
    renderer->last_mouse_pos_ = Vec2f(float(point.x), float(point.y));
  } else if (uMsg == WM_MOUSEWHEEL) {
    float offset = GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
    renderer->camera_->Scale(offset);
  }*/ else {
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }
  return 0;
}

void InitPlatform() {
  assert(g_initialized == 0);
  ATOM class_atom;
  WNDCLASS window_class;
  window_class.style = CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc = ProcessMessage;
  window_class.cbClsExtra = 0;
  window_class.cbWndExtra = 0;
  window_class.hInstance = GetModuleHandle(NULL);
  window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
  window_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  window_class.lpszMenuName = NULL;
  window_class.lpszClassName = WINDOW_CLASS_NAME;
  class_atom = RegisterClass(&window_class);
  assert(class_atom != 0);
  (void)(class_atom);
  g_initialized = 1;
}

void DestroyPlatform() {
  assert(g_initialized == 1);
  UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(NULL));
  g_initialized = 0;
}

void PollInputEvents() {
  MSG message;
  while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
}

VWindow* CreateVWindow(const std::string& title, int& width, int& height,
                       void* renderer) {
  WinWindow* window = new WinWindow;
  window->Init(title, width, height, renderer);
  SetProp(window->handle_, WINDOW_ENTRY_NAME, renderer);
  ShowWindow(window->handle_, SW_SHOW);
  return window;
};

void WinWindow::Init(const std::string& title, int& width, int& height,
                     void* renderer) {
  assert(g_initialized && width > 0 && height > 0);
  DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
  RECT rect;
#ifdef UNICODE
  wchar_t t_title[256];
  mbstowcs(t_title, title.c_str(), 256);
#else
  const char* t_title = title.c_str();
#endif
  rect.left = 0;
  rect.top = 0;
  rect.right = width;
  rect.bottom = height;
  AdjustWindowRect(&rect, style, 0);
  int twidth = rect.right - rect.left;
  int theight = rect.bottom - rect.top;

  handle_ = CreateWindow(WINDOW_CLASS_NAME, t_title, style, CW_USEDEFAULT,
                         CW_USEDEFAULT, twidth, theight, NULL, NULL,
                         GetModuleHandle(NULL), NULL);
  if (handle_ == NULL) {
    spdlog::error("window creation failed");
    exit(-1);
  }

  surface_ = new Image(width, height, 4);
  delete[] surface_->buffer_;
  surface_->buffer_ = nullptr;

  BITMAPINFOHEADER bi_header;
  HBITMAP dib_bitmap;
  HBITMAP old_bitmap;
  HDC window_dc;
  window_dc = GetDC(handle_);
  memory_dc_ = CreateCompatibleDC(window_dc);
  ReleaseDC(handle_, window_dc);
  memset(&bi_header, 0, sizeof(BITMAPINFOHEADER));
  bi_header.biSize = sizeof(BITMAPINFOHEADER);
  bi_header.biWidth = width;
  bi_header.biHeight = -height; /* top-down */
  bi_header.biPlanes = 1;
  bi_header.biBitCount = 32;
  bi_header.biCompression = BI_RGB;
  dib_bitmap =
      CreateDIBSection(memory_dc_, (BITMAPINFO*)&bi_header, DIB_RGB_COLORS,
                       (void**)&surface_->buffer_, NULL, 0);
  assert(dib_bitmap != NULL);
  old_bitmap = (HBITMAP)SelectObject(memory_dc_, dib_bitmap);
  DeleteObject(old_bitmap);
};

void WinWindow::Destroy() {
  ShowWindow(handle_, SW_HIDE);
  RemoveProp(handle_, WINDOW_ENTRY_NAME);
  DeleteDC(memory_dc_);
  DestroyWindow(handle_);
}

void WinWindow::DrawBuffer(Framebuffer* buffer) {
  assert(surface_->width_ == buffer->width_ &&
         surface_->height_ == buffer->height_);
  const int width = surface_->width_;
  const int height = surface_->height_;
  for (int r = 0; r < height; ++r) {
    for (int c = 0; c < width; ++c) {
      int flipped_r = height - 1 - r;
      int src_index = (r * width + c) * 4;
      int dst_index = (flipped_r * width + c) * 4;
      unsigned char* src_pixel = &buffer->color_[src_index];
      unsigned char* dst_pixel = &surface_->buffer_[dst_index];
      dst_pixel[0] = src_pixel[2];  // b
      dst_pixel[1] = src_pixel[1];  // g
      dst_pixel[2] = src_pixel[0];  // r
    }
  }

  HDC window_dc = GetDC(handle_);
  BitBlt(window_dc, 0, 0, width, height, memory_dc_, 0, 0, SRCCOPY);
  ReleaseDC(handle_, window_dc);
}
};  // namespace VCL