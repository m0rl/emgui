#include "imgui.h"

#include "window_manager.hpp"

namespace {

class ExampleWindow : public emgui::Window {
 public:
  void Draw() override {
    ImGui::SetNextWindowPos({150, 50}, ImGuiSetCond_Once);
    ImGui::SetNextWindowSize({600, 800}, ImGuiSetCond_Once);
    ImGui::Begin("Example Window");
    ImGui::End();
  }
};

} // namespace <anonymous> 

int main()
{
  emgui::WindowManager window_manager("Emgui Example");
  window_manager.RegisterWindow(std::make_unique<ExampleWindow>());
  window_manager.Run();
}
