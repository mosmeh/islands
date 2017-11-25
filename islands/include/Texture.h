#pragma once

#include "Resource.h"

namespace islands {

class Texture2D : public Resource<Texture2D> {
public:
	Texture2D(const std::string& name, const std::string& filename);

	Texture2D(const Texture2D&) = delete;
	Texture2D& operator=(const Texture2D&) = delete;
	Texture2D(Texture2D&&) = default;

	virtual ~Texture2D();

	void bind(unsigned int textureUnit);

private:
	const std::string filename_;
	int width_, height_, channels_;
	unsigned char* data_;
	GLuint id_;

	void loadImpl() override;
	void uploadImpl() override;
};

class RenderTexture {
public:
	RenderTexture();
	RenderTexture(const RenderTexture&) = delete;
	RenderTexture& operator=(const RenderTexture&) = delete;
	RenderTexture(RenderTexture&&) = default;
	virtual ~RenderTexture();

	GLuint getId() const;
	void setSize(const glm::uvec2& size);
	void bind(unsigned int textureUnit);

private:
	GLuint id_;
};

}
