#include "Material.h"

namespace islands {

Material::Material() : opaqueness_(Opaqueness::InheritModel) {}

void Material::setVertexShader(std::shared_ptr<Shader> shader) {
	vertex_ = shader;
}

void Material::setGeometryShader(std::shared_ptr<Shader> shader) {
	geometry_ = shader;
}

void Material::setFragmentShader(std::shared_ptr<Shader> shader) {
	fragment_ = shader;
}

void Material::setUpdateUniformCallback(const UpdateUniformCallback& callback) {
	updateUniformCallback_ = callback;
}

const Material::UpdateUniformCallback& Material::getUpdateUniformCallback() const {
	return updateUniformCallback_;
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

void Material::setOpaqueness(Opaqueness opaqueness) {
	opaqueness_ = opaqueness;
}

Material::Opaqueness Material::getOpaqueness() const {
	return opaqueness_;
}

}
