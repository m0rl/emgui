#ifndef EMGUI_INCLUDE_WINDOW_MANAGER_HPP_
#define EMGUI_INCLUDE_WINDOW_MANAGER_HPP_

#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <SDL.h>

#include "gles_device.hpp"

namespace emgui {
namespace detail {

class SDLGLContextWindow {
 public:
  SDLGLContextWindow(std::string_view title) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    CreateGlContextWindow(title);
  }

  SDLGLContextWindow(SDLGLContextWindow&) = delete;
  SDLGLContextWindow& operator=(SDLGLContextWindow&) = delete;

  void ResizeContextWindow(int width, int height) {
    SDL_SetWindowSize(glcontext_window_, width, height);
  }

  void SwapContextWindowBuffers() {
    SDL_GL_SwapWindow(glcontext_window_);
  }

  std::pair<int, int> ContextWindowSize() const {
    int width = 0, height = 0;
    SDL_GetWindowSize(glcontext_window_, &width, &height);
    return {width, height};
  }

  uint32_t ContextWindowFlags() const {
    return SDL_GetWindowFlags(glcontext_window_);
  }

 protected:
  ~SDLGLContextWindow() {
    SDL_GL_DeleteContext(glcontext_);
    SDL_DestroyWindow(glcontext_window_);
    SDL_Quit();
  }

 private:
  void CreateGlContextWindow(std::string_view title) {
    glcontext_window_ = SDL_CreateWindow(title.data(), SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    glcontext_ = SDL_GL_CreateContext(glcontext_window_);
  }

  SDL_Window *glcontext_window_;
  SDL_GLContext glcontext_;
};

} // namespace detail

class Window {
 public:
  virtual void Draw() = 0;
  virtual ~Window() = default;
};

class WindowManager : private detail::SDLGLContextWindow {
 public:
  explicit WindowManager(std::string_view title)
      : detail::SDLGLContextWindow(title) {
    SetupImguiKeyMap(ImGui::GetIO());
    SetupImguiClipboardHandlers(ImGui::GetIO());
    SetupCanvasFullscreenMode();
  }

  WindowManager(WindowManager const&) = delete;
  WindowManager& operator=(WindowManager const&) = delete;

  ~WindowManager() {
    emscripten_exit_soft_fullscreen();
    ImGui::Shutdown();
  }

  void Run() {
    emscripten_set_main_loop_arg(EmscriptenEventLoopProxy, this, 0, 1);
  }

  void RegisterWindow(std::unique_ptr<Window> window) {
    windows_.push_back(std::move(window));
  }

 private:
  static void EmscriptenEventLoopProxy(void *arg) {
    WindowManager *wm = static_cast<WindowManager*>(arg);
    wm->ProcessEvents();
  }

  static int EmscriptenOnCanvasResizedProxy(int, void const*, void *arg) {
    WindowManager *wm = static_cast<WindowManager*>(arg);
    wm->OnCanvasResized();
    return 0;
  }

  static const char *ImguiGetClipboardTextHandler(void *) {
    return SDL_GetClipboardText();
  }

  static void ImguiSetClipboardTextHandler(void *, char const* text) {
    SDL_SetClipboardText(text);
  }

  void SetupImguiKeyMap(ImGuiIO& io);
  void PassSDLEventsToImguiIO(ImGuiIO& io);

  void ProcessEvents() {
    PassSDLEventsToImguiIO(ImGui::GetIO());
    UpdateImguiFrameConfig(ImGui::GetIO());
    ImGui::NewFrame();
    for (auto& window : windows_)
      window->Draw();
    FillBackgroundWithColor(kBackgroundColor);
    ImGui::Render();
    render_device_.DrawLists(*ImGui::GetDrawData());
    SwapContextWindowBuffers();
  }

  void SetupCanvasFullscreenMode() {
    EmscriptenFullscreenStrategy strategy;
    strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
    strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
    strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
    strategy.canvasResizedCallback =
        &WindowManager::EmscriptenOnCanvasResizedProxy;
    strategy.canvasResizedCallbackUserData = this;
    emscripten_enter_soft_fullscreen(kCanvasElementName, &strategy);
  }

  void SetupImguiClipboardHandlers(ImGuiIO& io) {
    io.SetClipboardTextFn = ImguiSetClipboardTextHandler;
    io.GetClipboardTextFn = ImguiGetClipboardTextHandler;
  }

  void FillBackgroundWithColor(ImVec4 const& color) {
    glViewport(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  void UpdateFrameDisplaySize(ImGuiIO& io) {
    int width = 0, height = 0;
    std::tie(width, height) = ContextWindowSize();
    io.DisplaySize = ImVec2(width, height);
  }

  void UpdateFrameDeltaTime(ImGuiIO& io) {
    io.DeltaTime = 1.0f / 60.0f;
  }

  void UpdateFrameMouseState(ImGuiIO& io) {
    int cursorx = 0, cursory = 0;
    uint32_t mouse_state = SDL_GetMouseState(&cursorx, &cursory);
    if (ContextWindowFlags() & SDL_WINDOW_MOUSE_FOCUS)
      io.MousePos = ImVec2(cursorx, cursory);
    io.MouseDown[0] = ((mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0);
    io.MouseDown[1] = ((mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0);
    io.MouseDown[2] = ((mouse_state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0);
  }

  void UpdateImguiFrameConfig(ImGuiIO& io) {
    UpdateFrameDisplaySize(io);
    UpdateFrameDeltaTime(io);
    UpdateFrameMouseState(io);
  }

  void OnCanvasResized() {
    int width = 0, height = 0;
    emscripten_get_canvas_element_size(kCanvasElementName, &width, &height);
    ResizeContextWindow(width, height);
  }

  const ImVec4 kBackgroundColor = ImColor(50, 50, 50);
  const char *kCanvasElementName = "emgui_canvas_element";

  GlesDevice render_device_;
  std::vector<std::unique_ptr<Window>> windows_;
};

} // namespace emgui

#endif // EMGUI_INCLUDE_WINDOW_MANAGER_HPP_
