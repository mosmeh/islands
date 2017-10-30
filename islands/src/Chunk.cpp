#include "System.h"
#include "Chunk.h"
#include "ResourceSystem.h"
#include "PhysicalBody.h"
#include "Physics.h"
#include "Player.h"
#include "Enemy.h"

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

std::shared_ptr<Entity> Chunk::createEntity(const std::string& name) {
	const auto entity = std::make_shared<Entity>(name, *this);
	entities_.emplace_back(entity);
	return entity;
}

const std::vector<std::shared_ptr<Entity>>& Chunk::getEntities() const {
	return entities_;
}

std::shared_ptr<Entity> Chunk::getEntityByName(const std::string& name) const {
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
	
	cleanEntities();
	for (size_t i = 0; i < entities_.size(); ++i) {
		entities_.at(i)->update();
	}
	cleanEntities();

	aabb_.min = glm::vec3(INFINITY);
	aabb_.max = glm::vec3(-INFINITY);
	for (const auto entity : entities_) {
		if ((entity->getSelfMask() & Entity::Mask::StaticObject) &&
			entity->hasComponent<MeshCollider>()) {
			for (const auto collider : entity->getComponents<Collider>()) {
				aabb_.min = glm::min(aabb_.min, collider->getGlobalAABB().min);
				aabb_.max = glm::max(aabb_.max, collider->getGlobalAABB().max);
			}
		}
	}
	aabb_.min.z = -INFINITY; // FIXME
	aabb_.max.z = INFINITY;

	physics::update(*this);
}

void Chunk::draw() {
	upload();

	for (const auto entity : entities_) {
		entity->draw();
	}
}

const geometry::AABB& Chunk::getGlobalAABB() const {
	return aabb_;
}

void Chunk::loadImpl() {
	picojson::value json;
	{
		std::ifstream ifs(LEVEL_DIR + sys::getFilePathSeperator() + filename_);
		ifs >> json;
	}

	createEntity("Player")->createComponent<Player>();

	for (const auto& ent : json.get("entities").get<picojson::object>()) {
		const auto entity = createEntity(ent.first);
		const auto& prop = ent.second.get<picojson::object>();
		if (prop.find("position") != prop.end()) {
			entity->setPosition(toVec3(prop.at("position")));
		}
		if (prop.find("quaternion") != prop.end()) {
			entity->setQuaternion(toQuat(prop.at("quaternion")));
		}
		if (prop.find("scale") != prop.end()) {
			entity->setScale(toVec3(prop.at("scale")));
		}
		entity->setSelfMask(Entity::Mask::StaticObject);
		entity->setFilterMask(Entity::Mask::DynamicObject);

		std::shared_ptr<Model> model(nullptr);
		if (prop.find("model") != prop.end()) {
			const auto& modelProp = prop.at("model").get<picojson::object>();

			const auto& meshName = modelProp.at("mesh").get<std::string>();
			model = ResourceSystem::getInstance().createOrGet<Model>(meshName, meshName);
			const auto drawer = entity->createComponent<ModelDrawer>(model);

			if (modelProp.find("visible") != modelProp.end()) {
				drawer->setVisible(modelProp.at("visible").get<bool>());
			}
			if (modelProp.find("lightmap") != modelProp.end()) {
				const auto& lightmapName = modelProp.at("lightmap").get<std::string>();
				drawer->setLightmapTexture(ResourceSystem::getInstance().createOrGet<Texture2D>(
					lightmapName, lightmapName));
			}
		}

		if (prop.find("collision") != prop.end()) {
			if (prop.at("collision").is<std::string>()) {
				const auto& type = prop.at("collision").get<std::string>();
				if (type == "sphere") {
					assert(model);
					entity->createComponent<SphereCollider>(model);
				} else {
					throw std::exception("not implemented");
				}
			} else {
				const auto& collisionProp = prop.at("collision").get<picojson::object>();
				const auto& type = collisionProp.at("type").get<std::string>();
				const auto& meshName = collisionProp.at("mesh_name").get<std::string>();
				const auto collisionMesh = ResourceSystem::getInstance().createOrGet<Model>(
					meshName, meshName);
				if (type == "mesh") {
					entity->createComponent<MeshCollider>(collisionMesh);
				} else if (type == "floor") {
					entity->createComponent<FloorCollider>(collisionMesh);
				} else {
					throw std::exception("not implemented");
				}
			}
		}

		if (prop.find("enemy") != prop.end()) {
			const auto& enemyProp = prop.at("enemy").get<picojson::object>();
			
			const auto& type = enemyProp.at("type").get<std::string>();
			if (type == "slime") {
				entity->createComponent<Slime>();
			} else {
				throw std::exception("not implemented");
			}
		}
	}
}

void Chunk::cleanEntities() {
	entities_.erase(std::remove_if(entities_.begin(), entities_.end(), [](std::shared_ptr<Entity> e) {
		return e->isDestroyed();
	}), entities_.end());
}

}