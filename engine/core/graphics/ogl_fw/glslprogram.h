#ifndef GLSLPROGRAM_H
#define GLSLPROGRAM_H

#include <GL/glew.h>

#include <glm/glm.hpp>

#include <string>
#include <map>
#include <stdexcept>

struct GLSLProgramException : public std::runtime_error {
    GLSLProgramException(const std::string& msg) :
        std::runtime_error(msg) {}
};

namespace GLSLUtils {
    bool file_exists(const std::string& file_name);
    std::string get_file_extension(const char* file_name);
}

namespace GLSLShader {
    enum GLSLShaderType
    {
        VERTEX = GL_VERTEX_SHADER,
        FRAGMENT = GL_FRAGMENT_SHADER,
        GEOMETRY = GL_GEOMETRY_SHADER,
        TESS_CONTROL = GL_TESS_CONTROL_SHADER,
        TESS_EVALUATION = GL_TESS_EVALUATION_SHADER,
        COMPUTE = GL_COMPUTE_SHADER
    };
}

struct glsl_program_t
{
    glsl_program_t();
    ~glsl_program_t();
    glsl_program_t(const glsl_program_t&) = delete;
    glsl_program_t& operator=(const glsl_program_t&) = delete; 
public:
    void compile_shader(const char* file_name);
    void compile_shader(const char* file_name, const GLSLShader::GLSLShaderType type);
    void compile_shader(const std::string& shader_source, const GLSLShader::GLSLShaderType type, const char* file_name = NULL);
public:
    void link(void);
    void validate(void) const;
    void use(void) const;
    GLint get_handle(void) const;
    bool is_linked(void) const;
public:
    inline void bind_attrib_loc(const GLuint location, const char* name) const; 
    inline void bind_frag_data_loc(const GLuint location, const char* name) const;
public:
    inline void set_uniform(const char* name, const float x, const float y, const float z);
    inline void set_uniform(const char* name, const glm::vec2& v);
    inline void set_uniform(const char* name, const glm::vec3& v);
    inline void set_uniform(const char* name, const glm::vec4& v);
    inline void set_uniform(const char* name, const glm::mat4& m);
    inline void set_uniform(const char* name, const glm::mat3& m);
    inline void set_uniform(const char* name, const float val);
    inline void set_uniform(const char* name, const int val);
    inline void set_uniform(const char* name, const bool val);
    inline void set_uniform(const char* name, const GLuint val);
public:
    void find_uniform_locations(void);
    void print_active_uniforms(void) const;
    void print_active_uniform_blocks(void) const;
    void print_active_attribs(void) const;
    const char* get_attrib_type_string_form(const GLenum type) const; 
private:
    inline GLint get_uniform_location(const char* name);
    void detach_delete_shader_objects(void);
private:
    GLuint handle;
    bool linked;
    std::map<std::string, int> uniform_locations; 
};
#endif