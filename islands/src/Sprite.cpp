#include "Sprite.h"

namespace islands {

Sprite::Sprite(std::shared_ptr<Texture2D> texture) :
	spriteProgram_(Program::createOrGet("SpriteProgram",
		Program::ShaderList{
			Shader::createOrGet("sprite.vert", Shader::Type::Vertex),
			Shader::createOrGet("sprite.geom", Shader::Type::Geometry),
			Shader::createOrGet("sprite.frag", Shader::Type::Fragment)})),
	texture_(texture),
	pos_(0.f),
	size_(1.f),
	alpha_(1.f) {}

void Sprite::setPosition(const glm::vec2& pos) {
	pos_ = pos;
}

void Sprite::setSize(const glm::vec2& size) {
	size_ = size;
}

void Sprite::setAlpha(float alpha) {
	alpha_ = alpha;
}

void Sprite::draw() {
	vertexArray_.bind();
	spriteProgram_->use();
	spriteProgram_->setUniform("pos", pos_);
	spriteProgram_->setUniform("size", size_);
	spriteProgram_->setUniform("tex", static_cast<GLuint>(0));
	spriteProgram_->setUniform("alpha", alpha_);
	texture_->bind(0);

	glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
	glDrawArrays(GL_POINTS, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

}
