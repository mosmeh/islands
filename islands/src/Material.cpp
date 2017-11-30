#include "Material.h"

namespace islands {

Material::Material() : opacity_(Opacity::InheritModel) {}

void Material::setVertexShader(std::shared_ptr<Shader> shader) {
	vertex_ = shader;
}

void Material::setGeometryShader(std::shared_ptr<Shader> shader) {
	geometry_ = shader;
}

void Material::setFragmentShader(std::shared_ptr<Shader> shader) {
	fragment_ = shader;
}

void Material::setUniformProvider(const UniformProvider& provider) {
	uniformProvider_ = provider;
}

const Material::UniformProvider& Material::getUniformProvider() const {
	return uniformProvider_;
}

std::shared_ptr<Program> Material::getProgram(bool skinning) const {
	std::shared_ptr<Shader> vertex = vertex_;
	if (!vertex) {
		vertex = Shader::createOrGet(
			skinning ? "skinning.vert" : "default.vert", Shader::Type::Vertex);
	}

	std::shared_ptr<Shader> fragment = fragment_;
	if (!fragment) {
		fragment = Shader::createOrGet(
			texture_ ? "texture.frag" : "default.frag", Shader::Type::Fragment);
	}

	if (geometry_) {
		return Program::createOrGet(
			vertex->getName() + "//" + geometry_->getName() + "//" + fragment->getName(),
			Program::ShaderList{vertex, geometry_, fragment});
	} else {
		return Program::createOrGet(
			vertex->getName() + "//" + fragment->getName(),
			Program::ShaderList{vertex, fragment});
	}
}

void Material::setTexture(std::shared_ptr<Texture2D> texture) {
	texture_ = texture;
}

std::shared_ptr<Texture2D> Material::getTexture() const {
	return texture_;
}

void Material::setOpacity(Opacity opacity) {
	opacity_ = opacity;
}

Material::Opacity Material::getOpacity() const {
	return opacity_;
}

}
