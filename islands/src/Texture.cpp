#pragma warning(push)
#pragma warning(disable: 4244)
#pragma warning(disable: 4456)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(pop)

#include "Texture.h"
#include "Log.h"

namespace islands {

Texture2D::Texture2D(const std::string& name, const std::string& filename) :
	Resource(name),
	filename_(filename),
	data_(nullptr) {}

Texture2D::~Texture2D() {
	if (isUploaded()) {
		glDeleteTextures(1, &id_);
	}
	stbi_image_free(data_);
}

void Texture2D::bind(unsigned int textureUnit) {
	assert(textureUnit < 32);
	upload();

	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, id_);
}

void Texture2D::loadImpl() {
	int numComponents;
	stbi_set_flip_vertically_on_load(true);
	data_ = stbi_load(filename_.c_str(), &width_, &height_, &numComponents, STBI_rgb_alpha);
	if (!data_) {
		SLOG << "stbi: " << stbi_failure_reason() << std::endl;
		throw;
	}
	assert(numComponents == 4);
}

void Texture2D::uploadImpl() {
	glGenTextures(1, &id_);
	glBindTexture(GL_TEXTURE_2D, id_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_);

	stbi_image_free(data_);
	data_ = nullptr;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

}