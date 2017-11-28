#pragma once

#include "Resource.h"

namespace islands {

class Shader : public SharedResource<Shader> {
public:
	enum class Type {
		Vertex,
		Geometry,
		Fragment
	};

	Shader(const std::string& name, const std::string& filename, const Type& type);

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader(Shader&&) = default;

	virtual ~Shader();

	GLuint getId();

private:
	const std::string filename_;
	const Type type_;
	GLuint id_;
	std::string source_;

	void loadImpl() override;
	void uploadImpl() override;

	static GLenum toGLenum(const Type& type);
};

class Program : public SharedResource<Program> {
public:
	Program(const std::string& name, const std::string& vertex, const std::string& fragment);
	Program(const std::string& name, std::shared_ptr<Shader> vertex, std::shared_ptr<Shader> fragment);
	Program(const std::string& name,
		const std::string& vertex,
		const std::string& geometry,
		const std::string& fragment);
	Program(const std::string& name,
		std::shared_ptr<Shader> vertex,
		std::shared_ptr<Shader> geometry,
		std::shared_ptr<Shader> fragment);

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
	const std::shared_ptr<Shader> vertex_, geometry_, fragment_;
	GLuint id_;

	void uploadImpl() override;
};

}
