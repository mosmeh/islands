#include "Material.h"
#include "ResourceSystem.h"

namespace islands {

Material::Material(const std::string& name, const aiMaterial* material) :
	Resource(name),
	program_(ResourceSystem::getInstance().createOrGet<Program>(
		"DefaultProgram", "default.vert", "default.frag")),
	diffuseColor_(1.f, 0, 1.f, 1.f) {

	aiColor3D diffuse;
	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS) {
		diffuseColor_.r = diffuse.r;
		diffuseColor_.g = diffuse.g;
		diffuseColor_.b = diffuse.b;
	} else {
		diffuseColor_.rgb = glm::vec3(1.f, 0, 1.f);
	}

	ai_real opacity;
	if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
		diffuseColor_.a = opacity;
	} else {
		diffuseColor_.a = 1.f;
	}

}

bool Material::isLoaded() const {
	if (lightmap_ && !lightmap_->isLoaded()) {
		return false;
	}
	return true;
}

void Material::use() const {
	program_->use();
	program_->setUniform("diffuse", diffuseColor_);
	if (lightmap_) {
		lightmap_->bind(0);
		program_->setUniform("tex", static_cast<GLuint>(0));
	}
}

const glm::vec4& Material::getDiffuseColor() const {
	return diffuseColor_;
}

void Material::setLightmapTexture(std::shared_ptr<Texture2D> texture) {
	lightmap_ = texture;
	program_ = ResourceSystem::getInstance().createOrGet<Program>(
		"TextureProgram", "default.vert", "texture.frag");
}

void Material::setProgram(std::shared_ptr<Program> program) {
	program_ = program;
}

std::shared_ptr<Program> Material::getProgram() const {
	return program_;
}

}
