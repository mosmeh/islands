#include "Effect.h"
#include "Camera.h"

namespace islands {
namespace effect {

Damage::Damage(double duration) : duration_(duration) {}

void Damage::start() {
	drawer_ = getEntity().getFirstComponent<ModelDrawer>();

	const auto material = std::make_shared<Material>();
	material->setFragmentShader(Shader::createOrGet("damage.frag", Shader::Type::Fragment));
	material->setUpdateUniformCallback([this](std::shared_ptr<Program> program) {
		program->use();
		program->setUniform("MVP", getEntity().calculateMVPMatrix());
		program->setUniform("time", static_cast<glm::float32>(glfwGetTime() - startedAt_));
	});
	drawer_->pushMaterial(material);

	startedAt_ = glfwGetTime();
}

void Damage::update() {
	if (glfwGetTime() - startedAt_ > duration_) {
		drawer_->popMaterial();
		destroy();
	}
}

Scatter::Scatter(const FinishCallback& callback) : callback_(callback) {}

void Scatter::start() {
	drawer_ = getEntity().getFirstComponent<ModelDrawer>();

	const auto material = std::make_shared<Material>();
	material->setOpaqueness(Material::Opaqueness::Transparent);
	material->setVertexShader(Shader::createOrGet("scatter.vert", Shader::Type::Vertex));
	material->setGeometryShader(Shader::createOrGet("scatter.geom", Shader::Type::Geometry));
	material->setFragmentShader(Shader::createOrGet("scatter.frag", Shader::Type::Fragment));
	material->setUpdateUniformCallback([this](std::shared_ptr<Program> program) {
		program->use();
		program->setUniform("M", getEntity().getModelMatrix());
		program->setUniform("MV", Camera::getInstance().getViewMatrix() * getEntity().getModelMatrix());
		program->setUniform("VP", Camera::getInstance().getViewProjectionMatrix());
		program->setUniform("time", static_cast<glm::float32>(2.0 * (glfwGetTime() - startedAt_)));
	});
	drawer_->pushMaterial(material);

	startedAt_ = glfwGetTime();
}

void Scatter::update() {
	if (glfwGetTime() - startedAt_ > 1.0) {
		drawer_->popMaterial();
		callback_();
		destroy();
	}
}

void Sea::start() {
	drawer_ = getEntity().getFirstComponent<ModelDrawer>();

	const auto material = std::make_shared<Material>();
	material->setVertexShader(Shader::createOrGet("sea.vert", Shader::Type::Vertex));
	material->setUpdateUniformCallback([this](std::shared_ptr<Program> program) {
		program->use();
		program->setUniform("M", getEntity().getModelMatrix());
		program->setUniform("VP", Camera::getInstance().getViewProjectionMatrix());
		program->setUniform("time", static_cast<glm::float32>(glfwGetTime()));
	});
	drawer_->pushMaterial(material);
}

void Sea::update() {}

void SwimRing::start() {
	initPos_ = getEntity().getPosition();
	startedAt_ = glfwGetTime();
}

void SwimRing::update() {
	getEntity().setPosition(initPos_ + glm::vec3(0, 0, 0.3f * std::sin(glfwGetTime() - startedAt_)));
}

void Fish::start() {
	initPos_ = getEntity().getPosition();
	startedAt_ = glfwGetTime();
}

void Fish::update() {
	const auto delta = 0.5 * (glfwGetTime() - startedAt_);
	getEntity().setQuaternion(geometry::directionToQuaternion({0, std::cos(delta), 0}, {1.f, 0, 0}));
	getEntity().setPosition(initPos_ + glm::vec3(0, 4.f * std::sin(delta), 0));
}

}
}
