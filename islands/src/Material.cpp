#include "Material.h"

namespace islands {

Material::Material(const aiMaterial* material) : diffuse_(1.f, 0, 1.f, 1.f) {
	aiString aName;
	material->Get(AI_MATKEY_NAME, aName);
	name_ = aName.C_Str();

	aiColor3D aDiffuse;
	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, aDiffuse) == AI_SUCCESS) {
		diffuse_.rgb = {aDiffuse.r, aDiffuse.g, aDiffuse.b};
	}

	ai_real opacity;
	if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
		diffuse_.a = opacity;
	}
}

const std::string& Material::getName() const {
	return name_;
}

const glm::vec4& Material::getDiffuse() const {
	return diffuse_;
}

void Material::apply(std::shared_ptr<Program> program) const {
	program->use();
	program->setUniform("diffuse", diffuse_);
}

}
