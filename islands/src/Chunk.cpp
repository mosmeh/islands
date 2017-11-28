#include "Chunk.h"
#include "System.h"
#include "Camera.h"
#include "PhysicalBody.h"
#include "Physics.h"
#include "Player.h"
#include "Enemy.h"
#include "SpecialObjects.h"
#include "AssetArchive.h"

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
	filename_(filename),
	cameraOffset_(15.f) {}

std::shared_ptr<Entity> Chunk::createEntity(const std::string& name) {
	const auto entity = std::make_shared<Entity>(name, *this);
	entities_.emplace_back(entity);
	return entity;
}

const std::list<std::shared_ptr<Entity>>& Chunk::getEntities() const {
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
	for (const auto e : entities_) {
		e->update();
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

	Camera::getInstance().setOffset(cameraOffset_);

	for (const auto entity : entities_) {
		entity->drawOpaque();
	}
	for (const auto entity : entities_) {
		entity->drawTransparent();
	}
}

const geometry::AABB& Chunk::getGlobalAABB() const {
	return aabb_;
}

std::shared_ptr<Sound> Chunk::getBGM() const {
	return bgm_;
}

void Chunk::loadImpl() {
	picojson::value json;
	{
#ifdef ENABLE_ASSET_ARCHIVE
		const auto filePath = LEVEL_DIR + '/' + filename_;
		picojson::parse(json, AssetArchive::getInstance().readTextFile(filePath));
#else
		const auto filePath = LEVEL_DIR + sys::getFilePathSeparator() + filename_;
		std::ifstream ifs(filePath);
		ifs >> json;
#endif
	}

	createEntity("Player")->createComponent<Player>();

	if (json.contains("camera")) {
		const auto& cameraProp = json.get("camera").get<picojson::object>();
		if (cameraProp.find("offset") != cameraProp.end()) {
			cameraOffset_ = static_cast<float>(cameraProp.at("offset").get<double>());
		}
	}

	bgm_ = Sound::createOrGet(
		json.contains("bgm") ? json.get("bgm").get<std::string>() : "bgm.ogg");

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
		entity->setFilterMask(Entity::Mask::DynamicObject);
		entity->setSelfMask(Entity::Mask::StageObject);

		std::shared_ptr<Model> model(nullptr);
		if (prop.find("model") != prop.end()) {
			const auto& modelProp = prop.at("model").get<picojson::object>();

			model = Model::createOrGet(modelProp.at("mesh").get<std::string>());
			const auto drawer = entity->createComponent<ModelDrawer>(model);

			if (modelProp.find("visible") != modelProp.end()) {
				drawer->setVisible(modelProp.at("visible").get<bool>());
			}
			if (modelProp.find("lightmap") != modelProp.end()) {
				drawer->setTexture(Texture2D::createOrGet(
					modelProp.at("lightmap").get<std::string>()));
			}
			if (modelProp.find("vertex") != modelProp.end()) {
				const auto& name = modelProp.at("vertex").get<std::string>();
				drawer->setVertexShader(Shader::createOrGet(name, Shader::Type::Vertex));
			}
			if (modelProp.find("geometry") != modelProp.end()) {
				const auto& name = modelProp.at("geometry").get<std::string>();
				drawer->setGeometryShader(Shader::createOrGet(name, Shader::Type::Geometry));
			}
			if (modelProp.find("fragment") != modelProp.end()) {
				const auto& name = modelProp.at("fragment").get<std::string>();
				drawer->setFragmentShader(Shader::createOrGet(name, Shader::Type::Fragment));
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
				const auto collisionMesh = Model::createOrGet(meshName);
				if (type == "mesh") {
					entity->createComponent<MeshCollider>(collisionMesh);
				}  else if (type == "wall") {
					entity->createComponent<MeshCollider>(collisionMesh);
					entity->setSelfMask(Entity::Mask::CollisionWall);
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
				entity->createComponent<enemy::Slime>();
			} else if (type == "big_slime") {
				entity->createComponent<enemy::BigSlime>();
			} else if (type == "rabbit") {
				entity->createComponent<enemy::Rabbit>();
			} else if (type == "crab") {
				entity->createComponent<enemy::Crab>();
			} else if (type == "dragon") {
				entity->createComponent<enemy::Dragon>();
			} else {
				throw std::exception("not implemented");
			}
		}

		if (prop.find("special") != prop.end()) {
			const auto& specialProp = prop.at("special").get<picojson::object>();
			
			const auto& type = specialProp.at("type").get<std::string>();
			if (type == "totem_poll") {
				entity->createComponent<specialobj::TotemPoll>();
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
