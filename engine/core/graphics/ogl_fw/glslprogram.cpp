#include "glslprogram.h"

#include <stdio.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <vector>

namespace GLSLShaderInfo {
	std::map<std::string, GLSLShader::GLSLShaderType> extensions = {
		{".vs",   GLSLShader::GLSLShaderType::VERTEX},
		{".vert", GLSLShader::GLSLShaderType::VERTEX},
		{"_vert.glsl", GLSLShader::GLSLShaderType::VERTEX},
		{".vert.glsl", GLSLShader::GLSLShaderType::VERTEX },
		{".gs",   GLSLShader::GLSLShaderType::GEOMETRY},
		{".geom", GLSLShader::GLSLShaderType::GEOMETRY},
		{ ".geom.glsl", GLSLShader::GLSLShaderType::GEOMETRY },
		{".tcs",  GLSLShader::GLSLShaderType::TESS_CONTROL},
		{ ".tcs.glsl",  GLSLShader::GLSLShaderType::TESS_CONTROL },
		{ ".tes",  GLSLShader::GLSLShaderType::TESS_EVALUATION },
		{".tes.glsl",  GLSLShader::GLSLShaderType::TESS_EVALUATION},
		{".fs",   GLSLShader::GLSLShaderType::FRAGMENT},
		{".frag", GLSLShader::GLSLShaderType::FRAGMENT},
		{"_frag.glsl", GLSLShader::GLSLShaderType::FRAGMENT},
		{".frag.glsl", GLSLShader::GLSLShaderType::FRAGMENT},
		{".cs",   GLSLShader::GLSLShaderType::COMPUTE},
		{ ".cs.glsl",   GLSLShader::GLSLShaderType::COMPUTE }
	};
}

glsl_program_t::glsl_program_t() : handle(0), linked(false)
{
}

glsl_program_t::~glsl_program_t()
{
	if (handle == 0) return;
	else {
		detach_delete_shader_objects();
	}
	glDeleteProgram(handle);
}

void glsl_program_t::compile_shader(const char* file_name)
{
	const std::string ext = GLSLUtils::get_file_extension(file_name);
	GLSLShader::GLSLShaderType type = GLSLShader::GLSLShaderType::VERTEX;
	const auto it = GLSLShaderInfo::extensions.find(ext);
	if (it != GLSLShaderInfo::extensions.end())
	{
		type = it->second;
	}
	else
	{
		std::string msg = "Unrecognized extension: " + ext;
		throw GLSLProgramException(msg);
	}
	compile_shader(file_name, type);
}

void glsl_program_t::compile_shader(const char* file_name, const GLSLShader::GLSLShaderType type)
{
	if (!GLSLUtils::file_exists(file_name))
	{
		std::string msg = std::string("Shader: ") + file_name + " not found.";
		throw GLSLProgramException(msg);
	}

	if (handle <= 0)
	{
		handle = glCreateProgram(); if (handle == 0) 
		{
			throw GLSLProgramException("Unable to create shader program.");
		}
	}

	std::ifstream in_file(file_name, std::ios::in);
	if (!in_file)
	{
		std::string msg = std::string("Unable to open: ") + file_name;
		throw GLSLProgramException(msg);
	}

	// Get file contents
	std::stringstream code = {};
	code << in_file.rdbuf();
	in_file.close();

	compile_shader(code.str(), type, file_name);
}

void glsl_program_t::compile_shader(const std::string& shader_source, const GLSLShader::GLSLShaderType type, const char* file_name)
{
	if (handle <= 0)
	{
		handle = glCreateProgram(); if (handle <= 0)
		{
			throw GLSLProgramException("Unable to create shader program.");
		}
	}

	GLuint shader_handle = glCreateShader(type);

	const char* c_code = shader_source.c_str();
	glShaderSource(shader_handle, 1, &c_code, NULL);

	// Compile the shader
	glCompileShader(shader_handle);

	GLint result = {};
	glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &result);
	if (result == 0) // 0 == GL_FALSE
	{
		// Compilation failed, get log
		std::string msg = {};
		if (file_name != NULL)
		{
			msg = std::string(file_name) + ": shader compilation failed.\n";
		}
		else
		{
			msg = "Shader compilation failed.\n";
		}

		int length = 0;
		glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			std::string log(length, ' ');
			int written = 0;
			glGetShaderInfoLog(shader_handle, length, &written, &log[0]);
			msg += log;
		}
		throw GLSLProgramException(msg);
	}
	else
	{
		glAttachShader(handle, shader_handle);
	}
}

void glsl_program_t::link(void)
{
	if (linked) return;
	if (handle <= 0) throw GLSLProgramException("Program has not been compiled.");

	glLinkProgram(handle);

	GLint link_status = {};
	std::string err_str = {};
	glGetProgramiv(handle, GL_LINK_STATUS, &link_status);

	if (link_status == 0)
	{
		int length = 0;
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);
		err_str += "Program link failed:\n";
		if (length > 0) {
			std::string log(length, ' ');
			int written = 0;
			glGetProgramInfoLog(handle, length, &written, &log[0]);
			err_str += log;
		}
	}
	else {
		find_uniform_locations();
		linked = true;
	}

	detach_delete_shader_objects();

	if (link_status == 0) throw GLSLProgramException(err_str);
}

void glsl_program_t::validate(void) const
{
	if (is_linked() == false) throw GLSLProgramException("Program is not linked");

	GLint status = {};
	glValidateProgram(handle);
	glGetProgramiv(handle, GL_VALIDATE_STATUS, &status);

	if (status == 0)
	{
		// Store log and return false
		int length = 0;
		std::string log_str = {};

		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);

		if (length > 0) {
			char* c_log = new char[length];
			int written = 0;
			glGetProgramInfoLog(handle, length, &written, c_log);
			log_str = c_log;
			delete[] c_log;
		}

		throw GLSLProgramException(std::string("Program failed to validate\n") + log_str);
	}
}

void glsl_program_t::use(void) const
{
	if (handle <= 0 || (linked == false)) throw GLSLProgramException("Shader has not been linked");
	glUseProgram(handle);
}

inline GLint glsl_program_t::get_handle(void) const
{
	return handle;
}

inline bool glsl_program_t::is_linked(void) const
{
	return linked;
}

inline void glsl_program_t::bind_attrib_loc(const GLuint location, const char* name) const
{
	glBindAttribLocation(handle, location, name);
}

inline void glsl_program_t::bind_frag_data_loc(const GLuint location, const char* name) const
{
	glBindFragDataLocation(handle, location, name);
}

inline void glsl_program_t::set_uniform(const char* name, const float x, const float y, const float z)
{
	const GLint loc = get_uniform_location(name);
	glUniform3f(loc, x, y, z);
}

void glsl_program_t::set_uniform(const char* name, const glm::vec2& v)
{
	const GLint loc = get_uniform_location(name);
	glUniform2f(loc, v.x, v.y);
}

void glsl_program_t::set_uniform(const char* name, const glm::vec3& v)
{
	this->set_uniform(name, v.x, v.y, v.z);
}

void glsl_program_t::set_uniform(const char* name, const glm::vec4& v)
{
	const GLint loc = get_uniform_location(name);
	glUniform2f(loc, v.x, v.y);
}

void glsl_program_t::set_uniform(const char* name, const glm::mat4& m)
{
	const GLint loc = get_uniform_location(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
}

void glsl_program_t::set_uniform(const char* name, const glm::mat3& m)
{
	const GLint loc = get_uniform_location(name);
	glUniformMatrix3fv(loc, 1, GL_FALSE, &m[0][0]);
}

void glsl_program_t::set_uniform(const char* name, const float val)
{
	const GLint loc = get_uniform_location(name);
	glUniform1f(loc, val);
}

void glsl_program_t::set_uniform(const char* name, const int val)
{
	const GLint loc = get_uniform_location(name);
	glUniform1i(loc, val);
}

void glsl_program_t::set_uniform(const char* name, const bool val)
{
	const int loc = get_uniform_location(name);
	glUniform1i(loc, val);
}

void glsl_program_t::set_uniform(const char* name, const GLuint val)
{
	const GLint loc = get_uniform_location(name);
	glUniform1ui(loc, val);
}

void glsl_program_t::find_uniform_locations(void)
{
	uniform_locations.clear();

	GLint num_uniforms = 0;

#ifdef WIN32
	// For OpenGL 4.3 and above, use glGetProgramResource
	glGetProgramInterfaceiv(handle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &num_uniforms);
	GLenum properties[4] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX };
	for (GLint i = 0; i < num_uniforms; ++i)
	{
		GLint results[4] = {};
		glGetProgramResourceiv(handle, GL_UNIFORM, i, 4, properties, 4, NULL, results);

		if (results[3] != -1) continue; // Skip uniforms in blocks
		GLint name_buff_size = results[0] + 1;
		char* name = new char[name_buff_size];
		glGetProgramResourceName(handle, GL_UNIFORM, i, name_buff_size, NULL, name);
		uniform_locations[name] = results[2];
		delete[] name;
	}
#endif 
}

void glsl_program_t::print_active_uniforms(void) const
{
#ifdef WIN32
	// For OpenGL 4.3 and above, use glGetProgramResource
	GLint num_uniforms = 0;
	glGetProgramInterfaceiv(handle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &num_uniforms);

	GLenum properties[4] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX };

	printf("Active uniforms:\n");
	for (int i = 0; i < num_uniforms; ++i) {
		GLint results[4] = {};
		glGetProgramResourceiv(handle, GL_UNIFORM, i, 4, properties, 4, NULL, results);

		if (results[3] != -1) continue;  // Skip uniforms in blocks
		GLint nameBufSize = results[0] + 1;
		char* name = new char[nameBufSize];
		glGetProgramResourceName(handle, GL_UNIFORM, i, nameBufSize, NULL, name);
		printf("%-5d %s (%s)\n", results[2], name, get_attrib_type_string_form(results[1]));
		delete[] name;
	}
#endif
}

void glsl_program_t::print_active_uniform_blocks(void) const
{
#ifdef WIN32
	GLint num_blocks = 0;

	glGetProgramInterfaceiv(handle, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &num_blocks);
	GLenum block_properties[] = { GL_NUM_ACTIVE_VARIABLES, GL_NAME_LENGTH };
	GLenum block_index[] = { GL_ACTIVE_VARIABLES };
	GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_BLOCK_INDEX };

	for (int block = 0; block < num_blocks; ++block) {
		GLint block_info[2];
		glGetProgramResourceiv(handle, GL_UNIFORM_BLOCK, block, 2, block_properties, 2, NULL, block_info);
		GLint num_uniforms = block_info[0];

		char* block_name = new char[block_info[1] + 1];
		glGetProgramResourceName(handle, GL_UNIFORM_BLOCK, block, block_info[1] + 1, NULL, block_name);
		printf("Uniform block \"%s\":\n", block_name);
		delete[] block_name;

		GLint* uniform_indexes = new GLint[num_uniforms];
		glGetProgramResourceiv(handle, GL_UNIFORM_BLOCK, block, 1, block_index, num_uniforms, NULL, uniform_indexes);

		for (int unif = 0; unif < num_uniforms; ++unif) {
			GLint uni_index = uniform_indexes[unif];
			GLint results[3];
			glGetProgramResourceiv(handle, GL_UNIFORM, uni_index, 3, properties, 3, NULL, results);

			GLint num_buff_size = results[0] + 1;
			char* name = new char[num_buff_size];
			glGetProgramResourceName(handle, GL_UNIFORM, uni_index, num_buff_size, NULL, name);
			printf("    %s (%s)\n", name, get_attrib_type_string_form(results[1]));
			delete[] name;
		}

		delete[] uniform_indexes;
	}
#endif
}

void glsl_program_t::print_active_attribs(void) const
{
#ifdef WIN32    // >= OpenGL 4.3, use glGetProgramResource
	GLint num_attribs;
	glGetProgramInterfaceiv(handle, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &num_attribs);

	GLenum properties[4] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION };

	printf("Active attributes:\n");
	for (int i = 0; i < num_attribs; ++i) {
		GLint results[3];
		glGetProgramResourceiv(handle, GL_PROGRAM_INPUT, i, 3, properties, 3, NULL, results);

		GLint nameBufSize = results[0] + 1;
		char* name = new char[nameBufSize];
		glGetProgramResourceName(handle, GL_PROGRAM_INPUT, i, nameBufSize, NULL, name);
		printf("%-5d %s (%s)\n", results[2], name, get_attrib_type_string_form(results[1]));
		delete[] name;
	}
#endif
}

const char* glsl_program_t::get_attrib_type_string_form(const GLenum type) const
{
	// There are many more types than are covered here, but
    // these are the most common in these examples.
	switch (type) {
	case GL_FLOAT:
		return "float";
	case GL_FLOAT_VEC2:
		return "vec2";
	case GL_FLOAT_VEC3:
		return "vec3";
	case GL_FLOAT_VEC4:
		return "vec4";
	case GL_DOUBLE:
		return "double";
	case GL_INT:
		return "int";
	case GL_UNSIGNED_INT:
		return "unsigned int";
	case GL_BOOL:
		return "bool";
	case GL_FLOAT_MAT2:
		return "mat2";
	case GL_FLOAT_MAT3:
		return "mat3";
	case GL_FLOAT_MAT4:
		return "mat4";
	default:
		return "?";
	}
}

inline GLint glsl_program_t::get_uniform_location(const char* name)
{
	return uniform_locations[name];
}

void glsl_program_t::detach_delete_shader_objects(void)
{
	GLint num_shaders = 0;
	glGetProgramiv(handle, GL_ATTACHED_SHADERS, &num_shaders); 
	std::vector<GLuint> shader_names(num_shaders);
	glGetAttachedShaders(handle, num_shaders, NULL, shader_names.data());
	for (GLuint& shader : shader_names)
	{
		glDetachShader(handle, shader);
		glDeleteShader(shader);
	}
}

bool GLSLUtils::file_exists(const std::string& file_name)
{
	struct stat info = {};
	int ret = -1;

	ret = stat(file_name.c_str(), &info);
	return (0 == ret);
}

std::string GLSLUtils::get_file_extension(const char* file_name)
{
	std::string name_str(file_name);

	size_t dot_loc = name_str.find_last_of('.');
	if (dot_loc != std::string::npos) {
		std::string ext = name_str.substr(dot_loc);
		if (ext == ".glsl") {

			size_t loc = name_str.find_last_of('.', dot_loc - 1);
			if (loc == std::string::npos) {
				loc = name_str.find_last_of('_', dot_loc - 1);
			}
			if (loc != std::string::npos) {
				return name_str.substr(loc);
			}
		}
		else {
			return ext;
		}
	}
	return "";
}
