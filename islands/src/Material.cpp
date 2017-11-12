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

	const auto numDiffuseTextures = material->GetTextureCount(aiTextureType_DIFFUSE);
	aiString path;
	for (unsigned int i = 0; i < numDiffuseTextures; ++i) {
		material->GetTexture(aiTextureType_DIFFUSE, 0, &path);

		diffuseTextures_.emplace_back(
			ResourceSystem::getInstance().createOrGet<Texture2D>(path.C_Str(), path.C_Str()));
	}
}

bool Material::isLoaded() const {
	for (const auto texture : diffuseTextures_) {
		if (!texture->isLoaded()) {
			return false;
		}
	}
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
	for (size_t i = 0; i < diffuseTextures_.size(); ++i) {
		diffuseTextures_.at(i)->bind(i + 1);
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
