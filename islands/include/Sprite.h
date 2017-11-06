#pragma once

#include "Texture.h"
#include "Shader.h"
#include "GLObjects.h"

namespace islands {

class Sprite {
public:
	Sprite(std::shared_ptr<Texture2D> texture);
	virtual ~Sprite() = default;

	void setPosition(const glm::vec2& pos);
	void setSize(const glm::vec2& size);
	void setAlpha(float alpha);

	void draw();

private:
	const std::shared_ptr<Program> spriteProgram_;
	VertexArray vertexArray_;
	std::shared_ptr<Texture2D> texture_;
	glm::vec2 pos_, size_;
	float alpha_;
};

}