#include "gles_device.hpp"

#include <algorithm>
#include <cstdint>
#include <stdexcept>

namespace emgui {
namespace detail {

void GlesDeviceFont::LoadDefaultFontTexImage() {
  uint8_t *pixels = nullptr;
  int width = 0, height = 0;
  ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  glActiveTexture(GL_TEXTURE0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
      GL_UNSIGNED_BYTE, pixels);
  ImGui::GetIO().Fonts->ClearInputData();
  ImGui::GetIO().Fonts->ClearTexData();
}

GLuint GlesDeviceShader::CreateShader(GLenum type, char const* source) const {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  GLint compile_status = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
  if (compile_status == GL_FALSE) {
    char info_log[256];
    glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
    glDeleteShader(shader);
    std::stringstream os;
    os << "glCompileShader failed with error : " << info_log;
    throw std::invalid_argument(os.str());
  }
  return shader;
}

void GlesDeviceVertexShader::Enable() {
  SetupOrthographicProjectionMatrix();
  glVertexAttribPointer(position_loc_.value(), 2, GL_FLOAT, GL_FALSE,
      sizeof(ImDrawVert), reinterpret_cast<GLvoid const*>(offsetof(ImDrawVert, pos)));
  glVertexAttribPointer(texture_coord_loc_.value(), 2, GL_FLOAT, GL_FALSE,
      sizeof(ImDrawVert), reinterpret_cast<GLvoid const*>(offsetof(ImDrawVert, uv)));
  glVertexAttribPointer(texture_color_loc_.value(), 4, GL_UNSIGNED_BYTE, GL_TRUE,
      sizeof(ImDrawVert), reinterpret_cast<GLvoid const*>(offsetof(ImDrawVert, col)));
  glEnableVertexAttribArray(position_loc_.value());
  glEnableVertexAttribArray(texture_coord_loc_.value());
  glEnableVertexAttribArray(texture_color_loc_.value());
}

void GlesDeviceVertexShader::SetupOrthographicProjectionMatrix() {
  float width = std::max(ImGui::GetIO().DisplaySize.x, 1.0f);
  float height = std::max(ImGui::GetIO().DisplaySize.y, 1.0f);
  const float orth_proj_mat[4][4] = {
    { 2.0f / width, 0.0f,            0.0f, 0.0f},
    { 0.0f,         2.0f / -height,  0.0f, 0.0f},
    { 0.0f,         0.0f,           -1.0f, 0.0f},
    {-1.0f,         1.0f,            0.0f, 1.0f}
  };
  glUniformMatrix4fv(proj_mat_loc_.value(), 1, GL_FALSE, &orth_proj_mat[0][0]);
}

} // namespace detail

void GlesDevice::DrawLists(ImDrawData& draw_data) {
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  draw_data.ScaleClipRects(ImGui::GetIO().DisplayFramebufferScale);
  program_.DrawLists(draw_data);
  glDisable(GL_SCISSOR_TEST);
}

} // namespace emgui
