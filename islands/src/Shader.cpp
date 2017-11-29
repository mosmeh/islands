#include "Shader.h"
#include "AssetArchive.h"
#include "Log.h"

namespace islands {

Shader::Shader(const std::string& filename, const Type& type) :
	SharedResource(filename),
	id_(0),
	type_(type) {}

Shader::~Shader() {
	if (isUploaded()) {
		glDeleteShader(id_);
	}
}

GLuint Shader::getId() {
	upload();
	return id_;
}

void Shader::loadImpl() {
	static const std::string SHADER_DIR = "shader";
#ifdef ENABLE_ASSET_ARCHIVE
	source_ = AssetArchive::getInstance().readTextFile(SHADER_DIR + '/' + getName());
#else
	std::ifstream ifs(SHADER_DIR + sys::getFilePathSeparator() + getName(), std::ios::in);
	std::ostringstream ss;
	ss << ifs.rdbuf();
	source_ = ss.str();
#endif
}

void Shader::uploadImpl() {
	const auto str = source_.c_str();
	const auto length = static_cast<GLint>(source_.size());

	id_ = glCreateShader(toGLenum(type_));
	glShaderSource(id_, 1, &str, &length);
	source_.clear();
	glCompileShader(id_);

	GLint infoLogLength;
	glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 0) {
		std::vector<GLchar> infoLog(infoLogLength);
		glGetShaderInfoLog(id_, infoLogLength, nullptr, infoLog.data());

		SLOG << "Shader: " << infoLog.data() << std::endl;
	}

	GLint compileStatus;
	glGetShaderiv(id_, GL_COMPILE_STATUS, &compileStatus);
	assert(compileStatus == GL_TRUE);
}

GLenum Shader::toGLenum(const Type& type) {
	switch (type) {
	case Type::Vertex:
		return GL_VERTEX_SHADER;
	case Type::Geometry:
		return GL_GEOMETRY_SHADER;
	case Type::Fragment:
		return GL_FRAGMENT_SHADER;
	default:
		throw std::exception("unreachable");
	}
}


Program::Program(const std::string& name, ShaderList shaders) :
	SharedResource(name),
	id_(0),
	shaders_(shaders) {}

Program::~Program() {
	if (isUploaded()) {
		glDeleteProgram(id_);
	}
}

bool Program::isLoaded() const {
	for (const auto shader : shaders_) {
		if (!shader->isLoaded()) {
			return false;
		}
	}
	return true;
}

void Program::use() {
	upload();
	glUseProgram(id_);
}

void Program::setUniform(const char* name, GLuint value) const {
	assert(isUploaded());
	glUniform1i(glGetUniformLocation(id_, name), value);
}

void Program::setUniform(const char* name, glm::float32 value) const {
	assert(isUploaded());
	glUniform1f(glGetUniformLocation(id_, name), value);
}

void Program::setUniform(const char* name, const glm::fvec2& value) const {
	assert(isUploaded());
	glUniform2fv(glGetUniformLocation(id_, name), 1, glm::value_ptr(value));
}

void Program::setUniform(const char* name, const glm::fvec3& value) const {
	assert(isUploaded());
	glUniform3fv(glGetUniformLocation(id_, name), 1, glm::value_ptr(value));
}

void Program::setUniform(const char* name, const glm::fvec4& value) const {
	assert(isUploaded());
	glUniform4fv(glGetUniformLocation(id_, name), 1, glm::value_ptr(value));
}

void Program::setUniform(const char* name, const glm::mat4& value, bool transpose) const {
	assert(isUploaded());
	glUniformMatrix4fv(glGetUniformLocation(id_, name), 1, transpose, glm::value_ptr(value));
}

void Program::uploadImpl() {
	id_ = glCreateProgram();
	for (const auto shader : shaders_) {
		glAttachShader(id_, shader->getId());
	}
	glLinkProgram(id_);
	for (const auto shader : shaders_) {
		glDetachShader(id_, shader->getId());
	}

	GLint infoLogLength;
	glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 0) {
		std::vector<GLchar> infoLog(infoLogLength);
		glGetProgramInfoLog(id_, infoLogLength, nullptr, infoLog.data());

		SLOG << "Program: " << infoLog.data() << std::endl;
	}

	GLint linkStatus;
	glGetProgramiv(id_, GL_LINK_STATUS, &linkStatus);
	assert(linkStatus == GL_TRUE);
}

}
