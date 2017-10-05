#pragma once

#include "Resource.h"

namespace islands {

class Shader : public Resource {
public:
	Shader(const std::string& name, const std::string& filename, GLenum type);

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader(Shader&&) = default;

	virtual ~Shader();

	GLuint getId();

private:
	const std::string filename_;
	const GLenum type_;
	GLuint id_;
	std::string source_;

	void loadImpl() override;
	void uploadImpl() override;
};

class Program : public Resource {
public:
	Program(const std::string& name, const std::string& vertex, const std::string& fragment);

	Program(const Program&) = delete;
	Program& operator=(const Program&) = delete;
	Program(Program&&) = default;

	virtual ~Program();

	bool isLoaded() const override;
	void use();
	void setUniform(const char* name, GLuint value) const;
	void setUniform(const char* name, glm::float32 value) const;
	void setUniform(const char* name, const glm::fvec2& value) const;
	void setUniform(const char* name, const glm::fvec3& value) const;
	void setUniform(const char* name, const glm::fvec4& value) const;
	void setUniform(const char* name, const glm::mat4& value, bool transpose = false) const;

private:
	const std::shared_ptr<Shader> vertex_, fragment_;
	GLuint id_;

	void uploadImpl() override;
};

}
