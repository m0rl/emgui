#include "window_manager.hpp"

namespace emgui {

void WindowManager::SetupImguiKeyMap(ImGuiIO& io) {
  io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;
  io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
  io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
  io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
  io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
  io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
  io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
  io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
  io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
  io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
  io.KeyMap[ImGuiKey_A] = SDLK_a;
  io.KeyMap[ImGuiKey_C] = SDLK_c;
  io.KeyMap[ImGuiKey_V] = SDLK_v;
  io.KeyMap[ImGuiKey_X] = SDLK_x;
  io.KeyMap[ImGuiKey_Y] = SDLK_y;
  io.KeyMap[ImGuiKey_Z] = SDLK_z;
}

void WindowManager::PassSDLEventsToImguiIO(ImGuiIO& io) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_MOUSEWHEEL:
      if (event.wheel.y > 0)
        io.MouseWheel = 1;
      else if (event.wheel.y < 0)
        io.MouseWheel = -1;
      break;
    case SDL_MOUSEBUTTONDOWN:
      if (event.button.button == SDL_BUTTON_LEFT)
        io.MouseDown[0] = true;
      else if (event.button.button == SDL_BUTTON_RIGHT)
        io.MouseDown[1] = true;
      else if (event.button.button == SDL_BUTTON_MIDDLE)
        io.MouseDown[2] = true;
      break;
    case SDL_MOUSEBUTTONUP:
      if (event.button.button == SDL_BUTTON_LEFT)
        io.MouseDown[0] = false;
      else if (event.button.button == SDL_BUTTON_RIGHT)
        io.MouseDown[1] = false;
      else if (event.button.button == SDL_BUTTON_MIDDLE)
        io.MouseDown[2] = false;
      break;
    case SDL_TEXTINPUT:
      io.AddInputCharactersUTF8(event.text.text);
      break;
    case SDL_KEYDOWN:
      [[fallthrough]];
    case SDL_KEYUP:
      int key = event.key.keysym.sym & ~SDLK_SCANCODE_MASK;
      io.KeysDown[key] = (event.type == SDL_KEYDOWN);
      io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
      io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
      io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
      break;
    }
  }
}

} // namespace emgui
