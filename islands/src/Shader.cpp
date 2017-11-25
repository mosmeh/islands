#include "Shader.h"
#include "AssetArchive.h"
#include "Log.h"

namespace islands {

Shader::Shader(const std::string& name, const std::string& filename, GLenum type) :
	SharedResource(name),
	id_(0),
	filename_(filename),
	type_(type) {

	assert(type == GL_VERTEX_SHADER
		|| type == GL_GEOMETRY_SHADER
		|| type == GL_FRAGMENT_SHADER);
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
	static const std::string SHADER_DIR = "shader";
#ifdef ENABLE_ASSET_ARCHIVE
	source_ = AssetArchive::getInstance().readTextFile(SHADER_DIR + '/' + filename_);
#else
	std::ifstream ifs(SHADER_DIR + sys::getFilePathSeparator() + filename_, std::ios::in);
	std::ostringstream ss;
	ss << ifs.rdbuf();
	source_ = ss.str();
#endif
}

void Shader::uploadImpl() {
	const auto str = source_.c_str();
	const auto length = static_cast<GLint>(source_.size());

	id_ = glCreateShader(type_);
	glShaderSource(id_, 1, &str, &length);
	source_.clear();
	glCompileShader(id_);

	GLint infoLogLength;
	glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 0) {
		const auto infoLog = std::make_unique<GLchar[]>(infoLogLength);
		glGetShaderInfoLog(id_, infoLogLength, nullptr, infoLog.get());

		SLOG << "Shader: " << infoLog.get() << std::endl;
	}

	GLint compileStatus;
	glGetShaderiv(id_, GL_COMPILE_STATUS, &compileStatus);
	assert(compileStatus == GL_TRUE);
}

Program::Program(const std::string& name, const std::string& vertex, const std::string& fragment) :
	SharedResource(name),
	id_(0),
	vertex_(Shader::createOrGet(vertex, vertex, GL_VERTEX_SHADER)),
	geometry_(nullptr),
	fragment_(Shader::createOrGet(fragment, fragment, GL_FRAGMENT_SHADER)) {}

Program::Program(const std::string& name, std::shared_ptr<Shader> vertex, std::shared_ptr<Shader> fragment) :
	SharedResource(name),
	id_(0),
	vertex_(vertex),
	geometry_(nullptr),
	fragment_(fragment) {}

Program::Program(
	const std::string& name,
	const std::string& vertex,
	const std::string& geometry,
	const std::string& fragment) :
	SharedResource(name),
	id_(0),
	vertex_(Shader::createOrGet(vertex, vertex, GL_VERTEX_SHADER)),
	geometry_(Shader::createOrGet(geometry, geometry, GL_GEOMETRY_SHADER)),
	fragment_(Shader::createOrGet(fragment, fragment, GL_FRAGMENT_SHADER)) {}

Program::Program(const std::string& name, std::shared_ptr<Shader> vertex, std::shared_ptr<Shader> geometry, std::shared_ptr<Shader> fragment) :
	SharedResource(name),
	id_(0),
	vertex_(vertex),
	geometry_(geometry),
	fragment_(fragment) {}

Program::~Program() {
	if (isUploaded()) {
		glDeleteProgram(id_);
	}
}

bool Program::isLoaded() const {
	if (geometry_ && !geometry_->isLoaded()) {
		return false;
	}
	return vertex_->isLoaded() && fragment_->isLoaded();
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
	glAttachShader(id_, vertex_->getId());
	if (geometry_) {
		glAttachShader(id_, geometry_->getId());
	}
	glAttachShader(id_, fragment_->getId());
	glLinkProgram(id_);
	glDetachShader(id_, vertex_->getId());
	if (geometry_) {
		glDetachShader(id_, geometry_->getId());
	}
	glDetachShader(id_, fragment_->getId());

	GLint infoLogLength;
	glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 0) {
		const auto infoLog = std::make_unique<GLchar[]>(infoLogLength);
		glGetProgramInfoLog(id_, infoLogLength, nullptr, infoLog.get());

		SLOG << "Program: " << infoLog.get() << std::endl;
	}

	GLint linkStatus;
	glGetProgramiv(id_, GL_LINK_STATUS, &linkStatus);
	assert(linkStatus == GL_TRUE);
}

}
