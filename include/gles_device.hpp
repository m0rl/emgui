#ifndef EMGUI_INCLUDE_GLES_DEVICE_HPP_
#define EMGUI_INCLUDE_GLES_DEVICE_HPP_

#include <optional>
#include <sstream>
#include <type_traits>

#include <SDL_opengl.h>

#include "imgui.h"

namespace emgui {
namespace detail {

class GlesDeviceBuffer {
 public:
  explicit GlesDeviceBuffer(GLenum target) : target_(target) {
    GLuint buffer_name = 0;
    glGenBuffers(1, &buffer_name);
    buffer_ = buffer_name;
  }

  GlesDeviceBuffer(GlesDeviceBuffer&) = delete;
  GlesDeviceBuffer& operator=(GlesDeviceBuffer&) = delete;

  GlesDeviceBuffer(GlesDeviceBuffer&& other) noexcept {
    std::swap(target_, other.target_);
    std::swap(buffer_, other.buffer_);
  }

  ~GlesDeviceBuffer() {
    if (buffer_.has_value())
      glDeleteBuffers(1, &buffer_.value());
  }

  void LoadData(GLvoid const* data, GLsizeiptr size) {
    glBindBuffer(target_.value(), buffer_.value());
    glBufferData(target_.value(), size, data, GL_STREAM_DRAW);
  }

  void Bind() {
    glBindBuffer(target_.value(), buffer_.value());
  }

 private:
  std::optional<GLenum> target_;
  std::optional<GLuint> buffer_;
};

class GlesDeviceFont {
 public:
  GlesDeviceFont() {
    GLuint texture_name = 0;
    glGenTextures(1, &texture_name);
    glBindTexture(GL_TEXTURE_2D, texture_name);
    font_texture_ = texture_name;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    LoadDefaultFontTexImage();
    ImGui::GetIO().Fonts->TexID = reinterpret_cast<void *>(font_texture_.value());
  }

  GlesDeviceFont(GlesDeviceFont&) = delete;
  GlesDeviceFont& operator=(GlesDeviceFont&) = delete;

  GlesDeviceFont(GlesDeviceFont&& other) noexcept {
    std::swap(font_texture_, other.font_texture_);
  }

  ~GlesDeviceFont() {
    if (font_texture_.has_value())
      glDeleteTextures(1, &font_texture_.value());
  }

 private:
  void LoadDefaultFontTexImage();

  std::optional<GLuint> font_texture_;
};

class GlesDeviceShader {
 public:
  GlesDeviceShader(GLuint program, GLenum type, const char* source)
      : shader_(CreateShader(type, source)), program_attached_(program) {
    glAttachShader(program_attached_.value(), shader_.value());
  }

  GlesDeviceShader(GlesDeviceShader const&) = delete;
  GlesDeviceShader& operator=(GlesDeviceShader const&) = delete;

  GlesDeviceShader(GlesDeviceShader&& other) noexcept {
    std::swap(shader_, other.shader_);
    std::swap(program_attached_, other.program_attached_);
  }

  virtual ~GlesDeviceShader() {
    if (program_attached_.has_value() && shader_.has_value()) {
      glDetachShader(program_attached_.value(), shader_.value());
      glDeleteShader(shader_.value());
    }
  }

  virtual void LoadAttributesLocation() {}
  virtual void Enable() {}
  virtual void Disable() {}

 protected:
  template <typename... Args>
  GLint GetUniformLocation(Args&&... args) const {
    return glGetUniformLocation(program_attached_.value(), std::forward<Args>(args)...);
  }

  template <typename... Args>
  GLint GetAttribLocation(Args&&... args) const {
    return glGetAttribLocation(program_attached_.value(), std::forward<Args>(args)...);
  }

 private:
  GLuint CreateShader(GLenum type, const char* source) const;

  std::optional<GLuint> shader_;
  std::optional<GLuint> program_attached_;
};

class GlesDeviceVertexShader final : public GlesDeviceShader {
 public:
  explicit GlesDeviceVertexShader(GLuint program)
      : GlesDeviceShader(program, GL_VERTEX_SHADER, kShaderSource) {}

  GlesDeviceVertexShader(GlesDeviceVertexShader&& other) noexcept
      : GlesDeviceShader(std::move(other)) {
    std::swap(proj_mat_loc_, other.proj_mat_loc_);
    std::swap(position_loc_, other.position_loc_);
    std::swap(texture_coord_loc_, other.texture_coord_loc_);
    std::swap(texture_color_loc_, other.texture_color_loc_);
  }

  void LoadAttributesLocation() final {
    proj_mat_loc_ = GetUniformLocation("proj_mat");
    position_loc_ = GetAttribLocation("position");
    texture_coord_loc_ = GetAttribLocation("frag_texture_coord");
    texture_color_loc_ = GetAttribLocation("frag_texture_color");
  }

  void Enable() final;

  void Disable() final {
    glDisableVertexAttribArray(position_loc_.value());
    glDisableVertexAttribArray(texture_coord_loc_.value());
    glDisableVertexAttribArray(texture_color_loc_.value());
  }

 private:
  static constexpr char const* kShaderSource =
      "uniform mat4 proj_mat;\n"
      "attribute vec2 position;\n"
      "attribute vec2 frag_texture_coord;\n"
      "attribute vec4 frag_texture_color;\n"
      "varying vec2 texture_coord;\n"
      "varying vec4 texture_color;\n"
      "void main()\n"
      "{\n"
      "	texture_coord = frag_texture_coord;\n"
      "	texture_color = frag_texture_color;\n"
      "	gl_Position = proj_mat * vec4(position.xy, 0, 1);\n"
      "}\n";

  void SetupOrthographicProjectionMatrix();

  std::optional<GLint> proj_mat_loc_;
  std::optional<GLint> position_loc_;
  std::optional<GLint> texture_coord_loc_;
  std::optional<GLint> texture_color_loc_;
};

class GlesDeviceFragmentShader final : public GlesDeviceShader {
 public:
  explicit GlesDeviceFragmentShader(GLuint program)
      : GlesDeviceShader(program, GL_FRAGMENT_SHADER, kShaderSource) {}

  GlesDeviceFragmentShader(GlesDeviceFragmentShader&& other) noexcept
      : GlesDeviceShader(std::move(other)) {
    std::swap(texture_loc_, other.texture_loc_);
  }

  void LoadAttributesLocation() final {
    texture_loc_ = GetUniformLocation("texture");
  }

  void Enable() final {
    glUniform1i(texture_loc_.value(), 0);
  }

 private:
  static constexpr char const* kShaderSource =
      "precision mediump float;\n"
      "uniform sampler2D texture;\n"
      "varying vec2 texture_coord;\n"
      "varying vec4 texture_color;\n"
      "void main()\n"
      "{\n"
      "	gl_FragColor = texture_color * texture2D(texture, texture_coord);\n"
      "}\n";

  std::optional<GLint> texture_loc_;
};

template <typename... Shaders>
class GlesDeviceProgram {
 public:
  GlesDeviceProgram()
      : program_(glCreateProgram()), shaders_(Shaders(program_.value())...) {
    LinkProgram(program_.value());
    ForeachShader([](auto& shader) { shader.LoadAttributesLocation(); });
  }

  GlesDeviceProgram(GlesDeviceProgram&) = delete;
  GlesDeviceProgram& operator=(GlesDeviceProgram&) = delete;

  GlesDeviceProgram(GlesDeviceProgram&& other) noexcept {
    std::swap(program_, other.program_);
  }

  ~GlesDeviceProgram() {
    if (program_.has_value())
      glDeleteProgram(program_.value());
  }

  void DrawLists(ImDrawData const& draw_data) {
    ScopedProgramLoader program_loader(*this);
    for (int i = 0; i < draw_data.CmdListsCount; ++i) {
      ImDrawList const* cmd_list = draw_data.CmdLists[i];
      array_buffer_.LoadData(&cmd_list->VtxBuffer.front(),
          cmd_list->VtxBuffer.size() * sizeof(ImDrawVert));
      element_array_buffer_.LoadData(&cmd_list->IdxBuffer.front(),
          cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx));
      DrawElements(cmd_list->CmdBuffer);
    }
  }

 private:
  struct ScopedProgramLoader {
    ScopedProgramLoader(GlesDeviceProgram& program) : hosted_program_(program) {
      glUseProgram(hosted_program_.program_.value());
      hosted_program_.array_buffer_.Bind();
      hosted_program_.ForeachShader([](auto& shader) { shader.Enable(); });
    }

    ~ScopedProgramLoader() {
      hosted_program_.ForeachShader([](auto& shader) { shader.Disable(); });
    }

    GlesDeviceProgram& hosted_program_;
  };

  template <typename F, size_t... ShaderIndexes>
  void ForeachShaderImpl(F&& f, std::index_sequence<ShaderIndexes...>) {
    (std::forward<F>(f)(std::get<ShaderIndexes>(shaders_)), ...);
  }

  template <typename F>
  void ForeachShader(F&& f) {
    ForeachShaderImpl(std::forward<F>(f), std::index_sequence_for<Shaders...>{});
  }

  void DrawElements(ImVector<ImDrawCmd> const& cmd_buffer) {
    static_assert(std::is_same_v<unsigned short, ImDrawIdx>,
        "glDrawElements expects indices of type GL_UNSIGNED_SHORT");
    std::uintptr_t idx_buffer_offset = 0;
    for (auto cmd = cmd_buffer.begin(); cmd != cmd_buffer.end(); ++cmd) {
      glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLuint>(cmd->TextureId));
      GLsizei width = cmd->ClipRect.z - cmd->ClipRect.x;
      GLsizei height = cmd->ClipRect.w - cmd->ClipRect.y;
      glScissor(cmd->ClipRect.x, cmd->ClipRect.y, width, height);
      glDrawElements(GL_TRIANGLES, cmd->ElemCount, GL_UNSIGNED_SHORT,
          reinterpret_cast<GLvoid const*>(idx_buffer_offset));
      idx_buffer_offset += cmd->ElemCount * sizeof(ImDrawIdx);
    }
  }

  void LinkProgram(GLuint program) {
    glLinkProgram(program);
    GLint link_status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (link_status == GL_FALSE) {
      char info_log[256];
      glGetProgramInfoLog(program, sizeof(info_log), NULL, info_log);
      std::stringstream os;
      os << "glLinkProgram failed with error : " << info_log;
      throw std::invalid_argument(os.str());
    }
  }

  std::optional<GLuint> program_;
  std::tuple<Shaders...> shaders_;
  GlesDeviceBuffer array_buffer_{GL_ARRAY_BUFFER};
  GlesDeviceBuffer element_array_buffer_{GL_ELEMENT_ARRAY_BUFFER};
};

} // namespace detail

class GlesDevice {
 public:
  GlesDevice() = default;

  GlesDevice(GlesDevice const&) = delete;
  GlesDevice& operator=(GlesDevice const&) = delete;

  void DrawLists(ImDrawData& draw_data);

 private:
  detail::GlesDeviceProgram<
      detail::GlesDeviceVertexShader, detail::GlesDeviceFragmentShader> program_;
  detail::GlesDeviceFont font_;
};

} // namespace emgui

#endif // EMGUI_INCLUDE_GLES_DEVICE_HPP_
