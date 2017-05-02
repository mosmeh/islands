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
		float(obj.at("w").get<double>()),
		float(obj.at("x").get<double>()),
		float(obj.at("y").get<double>()),
		float(obj.at("z").get<double>())
	};
}

namespace islands {

Chunk::Chunk(const std::string& name, const std::string& filename) :
	Resource(name),
	filename_(filename) {

	picojson::value json;
	{
		std::ifstream ifs(filename);
		ifs >> json;
	}

	const auto& env = json.get("environment").get<picojson::object>();
	UNUSED(env);

	for (const auto& ent : json.get("entities").get<picojson::object>()) {
		const auto entity = std::make_shared<Entity>(ent.first, this);
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
			drawer->setLightmapTexture(ResourceSystem::getInstance().createOrGet<Texture2D>(
				entity->getName() + "-baked", entity->getName() + "-baked.png"));

			entity->attachComponent(drawer);

			//if (modelProp.find("collision") != modelProp.end()) {
			{
				//const auto& type = modelProp.at("collision").get<std::string>();

				if (entity->getName() == "Player") {
					const auto collider = std::make_shared<SphereCollider>(model, 1.f);
					physicsSystem_.registerCollider(collider);
					entity->attachComponent(collider);

					const auto body = std::make_shared<PhysicalBody>(1.f);
					body->setCollider(collider);
					physicsSystem_.registerBody(body);
					entity->attachComponent(body);
				} else if (entity->getName() == "Plane.001") {
					const auto collider = std::make_shared<MeshCollider>(model);
					physicsSystem_.registerCollider(collider);
					entity->attachComponent(collider);
				} else {
					//const auto collider = std::make_shared<MeshCollider>(model);
					const auto collider = std::make_shared<SphereCollider>(model, 0.f);
					physicsSystem_.registerCollider(collider);
					entity->attachComponent(collider);
				}
			}
		}

		entities_.push_back(entity);
	}
}

bool Chunk::isLoaded() const {
	for (const auto entity : entities_) {
		if (!entity->isLoaded()) {
			return false;
		}
	}
	return true;
}

std::shared_ptr<Entity> Chunk::getEntity(const std::string& name) {
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
	for (const auto entity : entities_) {
		entity->update();
	}
	physicsSystem_.update();
}

void Chunk::draw() {
	for (const auto entity : entities_) {
		entity->draw();
	}
}

void Chunk::loadImpl() {
	for (const auto entity : entities_) {
		entity->loadAsync();
	}
}

}