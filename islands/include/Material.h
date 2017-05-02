#pragma once

#include "Resource.h"
#include "Texture.h"
#include "Shader.h"

namespace islands {

class Material : public Resource {
public:
	Material(const std::string& name, const aiMaterial* material);

	Material(const Material&) = delete;
	Material& operator=(const Material&) = delete;
	Material(Material&&) = default;

	virtual ~Material() = default;

	bool isLoaded() const override;
	void use() const;

	void setLightmapTexture(std::shared_ptr<Texture2D> texture);

	void setProgram(std::shared_ptr<Program> program);
	std::shared_ptr<Program> getProgram() const;

private:
	glm::vec4 diffuseColor_;
	std::vector<std::shared_ptr<Texture2D>> diffuseTextures_;
	std::shared_ptr<Program> program_;
	std::shared_ptr<Texture2D> lightmap_;

	void loadImpl() override;
};

}