#pragma once

#include "Resource.h"

namespace islands {

class Texture2D : public Resource {
public:
	Texture2D(const std::string& name, const std::string& filename);

	Texture2D(const Texture2D&) = delete;
	Texture2D& operator=(const Texture2D&) = delete;
	Texture2D(Texture2D&&) = default;

	virtual ~Texture2D();

	void bind(unsigned int textureUnit);

private:
	const std::string filename_;
	int width_, height_;
	unsigned char* data_;
	GLuint id_;

	void loadImpl() override;
	void uploadImpl() override;
};

}