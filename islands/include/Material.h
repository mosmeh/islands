#pragma once

#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"

namespace islands {

class Material {
public:
	using UniformProvider = std::function<void(std::shared_ptr<Program>)>;
	enum class Opacity {
		Opaque,
		Transparent,
		InheritModel
	};

	Material();
	virtual ~Material() = default;

	void setVertexShader(std::shared_ptr<Shader> shader);
	void setGeometryShader(std::shared_ptr<Shader> shader);
	void setFragmentShader(std::shared_ptr<Shader> shader);
	std::shared_ptr<Program> getProgram(bool skinning) const;

	void setUniformProvider(const UniformProvider& provider);
	const UniformProvider& getUniformProvider() const;

	void setTexture(std::shared_ptr<Texture2D> texture);
	std::shared_ptr<Texture2D> getTexture() const;

	void setOpacity(Opacity opacity);
	Opacity getOpacity() const;

private:
	std::shared_ptr<Shader> vertex_, geometry_, fragment_;
	UniformProvider uniformProvider_;
	std::shared_ptr<Texture2D> texture_;
	Opacity opacity_;
};

}
