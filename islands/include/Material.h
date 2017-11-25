#pragma once

#include "Shader.h"

namespace islands {

struct Material {
	Material(const aiMaterial* material);
	virtual ~Material() = default;

	const std::string& getName() const;
	const glm::vec4& getDiffuse() const;
	void apply(std::shared_ptr<Program> program) const;

private:
	std::string name_;
	glm::vec4 diffuse_;
};

}
