#include "Material.h"
#include "ResourceSystem.h"

namespace islands {

Material::Material(const std::string& name, const aiMaterial* material) :
	Resource(name),
	program_(ResourceSystem::getInstance().createOrGet<Program>(
		"DefaultProgram", "default.vert", "default.frag")) {

	aiColor4D diffuse;
	aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
	diffuseColor_ = {diffuse.r, diffuse.g, diffuse.b, diffuse.a};
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
	program_ = 	ResourceSystem::getInstance().createOrGet<Program>(
		"TextureProgram", "default.vert", "texture.frag");
}

void Material::setProgram(std::shared_ptr<Program> program) {
	program_ = program;
}

std::shared_ptr<Program> Material::getProgram() const {
	return program_;
}

}
