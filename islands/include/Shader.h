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

	Shader(const std::string& filename, const Type& type);

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader(Shader&&) = default;

	virtual ~Shader();

	GLuint getId();

private:
	const Type type_;
	GLuint id_;
	std::string source_;

	void loadImpl() override;
	void uploadImpl() override;

	static GLenum toGLenum(const Type& type);
};

class Program : public SharedResource<Program> {
public:
	using ShaderList = std::initializer_list<std::shared_ptr<Shader>>;

	Program(const std::string& name, ShaderList shaders);

	Program(const Program&) = delete;
	Program& operator=(const Program&) = delete;
	Program(Program&&) = default;

	virtual ~Program();

	bool isLoaded() const override;

	void use();

	template <typename T>
	bool setUniform(const char* name, T&& value) const {
		assert(isUploaded());
		const auto location = glGetUniformLocation(id_, name);
		if (location == -1) {
			return false;
		}

		setUniformImpl(location, std::forward<T>(value));
		return true;
	}

private:
	const std::vector<std::shared_ptr<Shader>> shaders_;
	GLuint id_;

	void uploadImpl() override;

	void setUniformImpl(GLint location, GLuint value) const;
	void setUniformImpl(GLint location, glm::float32 value) const;
	void setUniformImpl(GLint location, const glm::fvec2& value) const;
	void setUniformImpl(GLint location, const glm::fvec3& value) const;
	void setUniformImpl(GLint location, const glm::fvec4& value) const;
	void setUniformImpl(GLint location, const glm::mat4& value) const;
};

}
