#include "Chunk.h"
#include "ResourceSystem.h"
#include "PhysicalBody.h"
#include "picojson/picojson.h"

glm::vec3 toVec3(const picojson::value& v) {
	const auto& obj = v.get<picojson::object>();
	return{
		obj.at("x").get<double>(),
		obj.at("y").get<double>(),
		obj.at("z").get<double>()
	};
}

glm::quat toQuat(const picojson::value& v) {
	const auto& obj = v.get<picojson::object>();
	return{
		static_cast<float>(obj.at("w").get<double>()),
		static_cast<float>(obj.at("x").get<double>()),
		static_cast<float>(obj.at("y").get<double>()),
		static_cast<float>(obj.at("z").get<double>())
	};
}

namespace islands {

Chunk::Chunk(const std::string& name, const std::string& filename) :
	Resource(name),
	filename_(filename) {}

bool Chunk::isLoaded() const {
	for (const auto entity : entities_) {
		if (!entity->isLoaded()) {
			return false;
		}
	}
	return true;
}

void Chunk::addEntity(std::shared_ptr<Entity> entity) {
	entity->setChunk(this);
	entitiesToBeAdded_.emplace_back(entity);
}

std::shared_ptr<Entity> Chunk::getEntity(const std::string& name) const {
	assert(isLoaded());

	const auto iter = std::find_if(entities_.begin(), entities_.end(),
		[&name](std::shared_ptr<Entity> e) {
		return e->getName() == name;
	});
	if (iter != entities_.end()) {
		return *iter;
	} else {
		throw std::invalid_argument("not found");
	}
}

void Chunk::update() {
	load();
	for (const auto entity : entities_) {
		entity->update();
	}

	std::copy(entitiesToBeAdded_.begin(), entitiesToBeAdded_.end(), std::back_inserter(entities_));
	entitiesToBeAdded_.clear();
	entitiesToBeAdded_.shrink_to_fit();

	physicsSystem_.update();
}

void Chunk::draw() {
	upload();

	for (const auto entity : entities_) {
		entity->draw();
	}
}

PhysicsSystem& Chunk::getPhysicsSystem() {
	return physicsSystem_;
}

void Chunk::loadImpl() {
	picojson::value json;
	{
		std::ifstream ifs(filename_);
		ifs >> json;
	}

	const auto& env = json.get("environment").get<picojson::object>();
	UNUSED(env);

	for (const auto& ent : json.get("entities").get<picojson::object>()) {
		const auto entity = std::make_shared<Entity>(ent.first);
		const auto& properties = ent.second.get<picojson::object>();
		entity->setPosition(toVec3(properties.at("position")));
		entity->setQuaternion(toQuat(properties.at("quaternion")));
		entity->setScale(toVec3(properties.at("scale")));

		if (properties.find("model") != properties.end()) {
			const auto& modelProp = properties.at("model").get<picojson::object>();

			const auto& meshName = modelProp.at("mesh").get<std::string>();
			const auto model = ResourceSystem::getInstance().createOrGet<Model>(meshName, meshName);
			const auto drawer = std::make_shared<ModelDrawer>(model);

			if (modelProp.find("visible") != modelProp.end()) {
				drawer->setVisible(modelProp.at("visible").get<bool>());
			}
			if (modelProp.find("cast_shadow") != modelProp.end()) {
				drawer->setCastShadow(modelProp.at("cast_shadow").get<bool>());
			}
			if (modelProp.find("receive_shadow") != modelProp.end()) {
				drawer->setReceiveShadow(modelProp.at("receive_shadow").get<bool>());
			}
			if (modelProp.find("lightmap") != modelProp.end()) {
				const auto& lightmapName = modelProp.at("lightmap").get<std::string>();
				drawer->setLightmapTexture(ResourceSystem::getInstance().createOrGet<Texture2D>(
					lightmapName, lightmapName));
			}

			entity->attachComponent(drawer);

			//if (modelProp.find("collision") != modelProp.end()) {
			{
				//const auto& type = modelProp.at("collision").get<std::string>();

				if (entity->getName() == "Plane.003") {
					const auto collider = std::make_shared<PlaneCollider>(model, glm::vec3(0.f, 0.f, 1.f));
					collider->setOffset(2.2f);
					physicsSystem_.registerCollider(collider);
					entity->attachComponent(collider);
				} else {
					const auto collider = std::make_shared<SphereCollider>(model);
					physicsSystem_.registerCollider(collider);
					entity->attachComponent(collider);
				}
			}
		}

		addEntity(entity);
	}
}

}