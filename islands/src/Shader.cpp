#include "Shader.h"
#include "ResourceSystem.h"
#include "Log.h"

namespace islands {

Shader::Shader(const std::string& name, const std::string& filename, GLenum type) :
	Resource(name),
	filename_(filename),
	type_(type) {

	assert(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER);
}

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
	std::ifstream ifs(filename_.c_str(), std::ios::in);
	std::ostringstream ss;
	ss << ifs.rdbuf();
	source_ = ss.str();
}

void Shader::uploadImpl() {
	const auto s = source_.c_str();
	GLint length = source_.size();

	id_ = glCreateShader(type_);
	glShaderSource(id_, 1, &s, &length);
	source_.clear();
	glCompileShader(id_);

	GLint infoLogLength;
	glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 0) {
		const auto infoLog = std::make_unique<GLchar[]>(infoLogLength);
		glGetShaderInfoLog(id_, infoLogLength, nullptr, infoLog.get());

		SLOG << "Shader: " << infoLog.get() << std::endl;
		throw;
	}
}

Program::Program(const std::string& name, const std::string& vertex, const std::string& fragment) :
	Resource(name),
	vertex_(ResourceSystem::getInstance().createOrGet<Shader>(vertex, vertex.c_str(), GL_VERTEX_SHADER)),
	fragment_(ResourceSystem::getInstance().createOrGet<Shader>(fragment, fragment.c_str(), GL_FRAGMENT_SHADER)) {}

Program::~Program() {
	if (isUploaded()) {
		glDeleteProgram(id_);
	}
}

bool Program::isLoaded() const {
	return Resource::isLoaded() && vertex_->isLoaded() && fragment_->isLoaded();
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

void Program::loadImpl() {
	vertex_->loadAsync();
	fragment_->loadAsync();
}

void Program::uploadImpl() {
	id_ = glCreateProgram();
	glAttachShader(id_, vertex_->getId());
	glAttachShader(id_, fragment_->getId());
	glLinkProgram(id_);

	GLint infoLogLength;
	glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 0) {
		const auto infoLog = std::make_unique<GLchar[]>(infoLogLength);
		glGetProgramInfoLog(id_, infoLogLength, nullptr, infoLog.get());

		SLOG << "Program: " << infoLog.get() << std::endl;
		throw;
	}

	glDetachShader(id_, vertex_->getId());
	glDetachShader(id_, fragment_->getId());
}

}
