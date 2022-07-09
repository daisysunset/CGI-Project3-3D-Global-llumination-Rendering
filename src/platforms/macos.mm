#include <Cocoa/Cocoa.h>
#include <mach-o/dyld.h>
#include <spdlog/spdlog.h>
#include <unistd.h>

#include "graphics/platform.h"
#include "renderer/renderer.h"

namespace VCL {
class MacWindow : public VWindow {
 public:
  NSWindow *handle_;
  virtual void Init(const std::string& title, int& width, int& height, void* renderer);
  virtual void Destroy();
  virtual void DrawBuffer(Framebuffer* buffer);
};
};

@interface WindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation WindowDelegate {
  VCL::MacWindow* window_;
}

- (instancetype)initWithWindow:(VCL::MacWindow*)window {
    self = [super init];
    if (self != nil) {
      window_ = window;
    }
    return self;
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
    (void)(sender);
    window_->should_close_ = 1;
    return NO;
}
@end

@interface ContentView : NSView
@end

@implementation ContentView {
  VCL::MacWindow* window_;
  VCL::Renderer* renderer_;
}

- (instancetype)initWithWindow:(VCL::MacWindow*)window pRenderer:(void*)renderer {
    self = [super init];
    if (self != nil) {
      window_ = window;
      renderer_ = (VCL::Renderer*)renderer;
    }
    CGRect rect = CGRectMake(0, 0, self.frame.size.width, self.frame.size.height);
    NSTrackingAreaOptions options = NSTrackingActiveInKeyWindow
                                   | NSTrackingMouseMoved
                                   | NSTrackingInVisibleRect;
    NSTrackingArea* area = [[NSTrackingArea alloc] initWithRect:rect options:options owner:self userInfo:nil];
    [self addTrackingArea:area];
    return self;
}

- (BOOL)acceptsFirstResponder {
  return YES;
}

- (void)mouseDown:(NSEvent*)event {
  NSPoint point = [window_->handle_ mouseLocationOutsideOfEventStream];
  NSRect rect = [[window_->handle_ contentView] frame];
  float xpos = (float)point.x;
  float ypos = (float)(rect.size.height - 1 - point.y);
  renderer_->last_mouse_pos_ = VCL::Vec2f(xpos, ypos);
  renderer_->button_pressed_[size_t(VCL::BUTTON::Left)] = true;
}

- (void)mouseUp:(NSEvent*)event {
  renderer_->button_pressed_[size_t(VCL::BUTTON::Left)] = false;
}

- (void)rightMouseDown:(NSEvent*)event {
  NSPoint point = [window_->handle_ mouseLocationOutsideOfEventStream];
  NSRect rect = [[window_->handle_ contentView] frame];
  float xpos = (float)point.x;
  float ypos = (float)(rect.size.height - 1 - point.y);
  renderer_->last_mouse_pos_ = VCL::Vec2f(xpos, ypos);
  renderer_->button_pressed_[size_t(VCL::BUTTON::Right)] = true;
}

- (void)rightMouseUp:(NSEvent*)event {
  renderer_->button_pressed_[size_t(VCL::BUTTON::Right)] = false;
}

- (void)mouseMoved:(NSEvent*)event {
  NSPoint point = [window_->handle_ mouseLocationOutsideOfEventStream];
  NSRect rect = [[window_->handle_ contentView] frame];
  float xpos = (float)point.x;
  float ypos = (float)(rect.size.height - 1 - point.y);
  renderer_->last_mouse_pos_ = VCL::Vec2f(xpos, ypos);
}

- (void)mouseDragged:(NSEvent*)event {
  NSPoint point = [window_->handle_ mouseLocationOutsideOfEventStream];
  NSRect rect = [[window_->handle_ contentView] frame];
  float xpos = (float)point.x;
  float ypos = (float)(rect.size.height - 1 - point.y);
  renderer_->camera_->Rotate(xpos - renderer_->last_mouse_pos_.x(),
                             ypos - renderer_->last_mouse_pos_.y());
  renderer_->last_mouse_pos_ = VCL::Vec2f(xpos, ypos);
}

- (void)rightMouseDragged:(NSEvent*)event {
  NSPoint point = [window_->handle_ mouseLocationOutsideOfEventStream];
  NSRect rect = [[window_->handle_ contentView] frame];
  float xpos = (float)point.x;
  float ypos = (float)(rect.size.height - 1 - point.y);
  renderer_->camera_->Translate(xpos - renderer_->last_mouse_pos_.x(),
                                ypos - renderer_->last_mouse_pos_.y());
  renderer_->last_mouse_pos_ = VCL::Vec2f(xpos, ypos);
}

- (void)scrollWheel:(NSEvent*)event {
  float offset = (float)[event scrollingDeltaY];
  if ([event hasPreciseScrollingDeltas]) {
    offset *= 0.1f;
  }
  renderer_->camera_->Scale(offset);
}

- (void)drawRect:(NSRect)dirtyRect {
    VCL::Image* surface = window_->surface_;
    NSBitmapImageRep *rep = [[[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:&(surface->buffer_)
                          pixelsWide:surface->width_
                          pixelsHigh:surface->height_
                       bitsPerSample:8
                     samplesPerPixel:3
                            hasAlpha:NO
                            isPlanar:NO
                      colorSpaceName:NSCalibratedRGBColorSpace
                         bytesPerRow:surface->width_ * 4
                        bitsPerPixel:32] autorelease];
    NSImage *nsimage = [[[NSImage alloc] init] autorelease];
    [nsimage addRepresentation:rep];
    [nsimage drawInRect:dirtyRect];
}

@end

namespace VCL {
static NSAutoreleasePool *g_autoreleasepool = NULL;

static void CreateMenubar(void) {
    NSMenu *menu_bar, *app_menu;
    NSMenuItem *app_menu_item, *quit_menu_item;
    NSString *app_name, *quit_title;

    menu_bar = [[[NSMenu alloc] init] autorelease];
    [NSApp setMainMenu:menu_bar];

    app_menu_item = [[[NSMenuItem alloc] init] autorelease];
    [menu_bar addItem:app_menu_item];

    app_menu = [[[NSMenu alloc] init] autorelease];
    [app_menu_item setSubmenu:app_menu];

    app_name = [[NSProcessInfo processInfo] processName];
    quit_title = [@"Quit " stringByAppendingString:app_name];
    quit_menu_item = [[[NSMenuItem alloc] initWithTitle:quit_title
                                                 action:@selector(terminate:)
                                          keyEquivalent:@"q"] autorelease];
    [app_menu addItem:quit_menu_item];
}

void InitPlatform() {
  if (NSApp == nil) {
    g_autoreleasepool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    CreateMenubar();
    [NSApp finishLaunching];
  }
};

void DestroyPlatform() {
  assert(g_autoreleasepool != NULL);
  [g_autoreleasepool drain];
  g_autoreleasepool = [[NSAutoreleasePool alloc] init];
}

void MacWindow::Init(const std::string& title, int& width, int& height, void* renderer) {
  NSRect rect;
  NSUInteger mask;
  WindowDelegate *delegate;
  ContentView *view;

  rect = NSMakeRect(0, 0, width, height);
  mask = NSWindowStyleMaskTitled
      | NSWindowStyleMaskClosable
      | NSWindowStyleMaskMiniaturizable;
  handle_ = [[NSWindow alloc] initWithContentRect:rect
                                         styleMask:mask
                                           backing:NSBackingStoreBuffered
                                             defer:NO];
  assert(handle_ != nil);
  [handle_ setTitle:[NSString stringWithUTF8String:title.c_str()]];
  [handle_ setColorSpace:[NSColorSpace genericRGBColorSpace]];

  delegate = [[WindowDelegate alloc] initWithWindow:this];
  assert(delegate != nil);
  [handle_ setDelegate:delegate];

  view = [[[ContentView alloc] initWithWindow:this pRenderer:renderer] autorelease];
  assert(view != nil);

  [handle_ setContentView:view];
  [handle_ makeFirstResponder:view];

  surface_ = new Image(width, height, 4);
  [handle_ makeKeyAndOrderFront:nil];
}

VWindow* CreateVWindow(const std::string& title, int& width, int& height,
                       void* renderer) {
  assert(NSApp && width > 0 && height > 0);
  MacWindow* window = new MacWindow;
  window->Init(title, width, height, renderer);
  return window;
}

void MacWindow::Destroy() {
  [handle_ orderOut:nil];

  [[handle_ delegate] release];
  [handle_ close];

  [g_autoreleasepool drain];
  g_autoreleasepool = [[NSAutoreleasePool alloc] init];
}

void MacWindow::DrawBuffer(Framebuffer* buffer) {
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
      dst_pixel[0] = src_pixel[0];  // b
      dst_pixel[1] = src_pixel[1];  // g
      dst_pixel[2] = src_pixel[2];  // r
    }
  }
  [[handle_ contentView] setNeedsDisplay:YES];
}

void PollInputEvents(){
  while(true) {
    NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:[NSDate distantPast]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
    if(event == nil) break;
    [NSApp sendEvent:event];
  }
  [g_autoreleasepool drain];
  g_autoreleasepool = [[NSAutoreleasePool alloc] init];
};
};