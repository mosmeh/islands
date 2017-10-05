#include "ChunkManager.h"
#include "PlayerController.h"
#include "ResourceSystem.h"
#include "Log.h"

namespace islands {

namespace detail {

std::uint64_t fold(std::int64_t x) {
	// http://mathworld.wolfram.com/FoldingFunction.html
	if (x == 0) {
		return 0;
	} else if (x > 0) {
		return 2 * static_cast<std::uint64_t>(x);
	} else {
		return 2 * static_cast<std::uint64_t>(-x) - 1;
	}
}

std::uint64_t cantorPair(std::uint64_t a, std::uint64_t b) {
	// http://mathworld.wolfram.com/PairingFunction.html
	const auto x = a + b;
	const auto y = x + 1;
	if (x % 2 == 0) {
		return (x / 2) * y + b;
	} else {
		return x * (y / 2) + b;
	}
}

}

ChunkManager::ChunkManager() :
	NEIGHBOR_OFFSETS{
		glm::ivec3(-1, 0, 0), glm::ivec3(1, 0, 0),
		glm::ivec3(0, -1, 0), glm::ivec3(0, 1, 0),
		glm::ivec3(0, 0, -1), glm::ivec3(0, 0, 1)
	},
	player_(std::make_shared<Entity>("Player")) {

	picojson::value json;
	{
		std::ifstream ifs("levels.json");
		ifs >> json;
	}
	for (const auto& item : json.get<picojson::array>()) {
		const auto& obj = item.get<picojson::object>();

		const auto& coordArray = obj.at("coord").get<picojson::array>();
		assert(coordArray.size() == 3);
		const glm::ivec3 coord(
			coordArray.at(0).get<double>(),
			coordArray.at(1).get<double>(),
			coordArray.at(2).get<double>()
		);

		const auto& filename = obj.at("filename").get<std::string>();

		const auto chunk = std::make_shared<Chunk>(filename, filename);
		chunk->update();
		chunk->update();
		chunks_.emplace(coord, chunk);
	}

	player_->setPosition({0.f, 0.f, 2.f});
	player_->setScale({0.00485f, 0.00485f, 0.006525f});

	player_->attachComponent(std::make_shared<PlayerController>());

	constexpr auto meshName = "character6a.fbx";
	const auto model = ResourceSystem::getInstance().createOrGet<Model>(meshName, meshName);
	player_->attachComponent(std::make_shared<ModelDrawer>(model));

	const auto collider = std::make_shared<SphereCollider>(model);
	player_->attachComponent(collider);

	const auto body = std::make_shared<PhysicalBody>();
	body->setCollider(collider);
	player_->attachComponent(body);

	jumpTo(glm::ivec3(0));
}
}

ChunkManager& ChunkManager::getInstance() {
	static ChunkManager instance;
	return instance;
}

void ChunkManager::update() {
	player_->update();
	currentChunk_->update();

	const auto& playerAABB = player_->getFirstComponent<Collider>()->getGlobalAABB();
	const auto& playerVelocity = player_->getFirstComponent<PhysicalBody>()->getVelocity();
	for (const auto& offset : NEIGHBOR_OFFSETS) {
		const auto destCoord = currentCoord_ + offset;
		if (chunks_.find(destCoord) != chunks_.end()) {
			if (glm::dot(playerVelocity, glm::vec3(offset)) > 0 &&
				geometry::intersect(chunks_.at(destCoord)->getGlobalAABB(), playerAABB)) {

				jumpTo(destCoord);
			}
		}
	}
}

void ChunkManager::draw() {
	player_->draw();
	currentChunk_->draw();
}

void ChunkManager::jumpTo(const glm::ivec3& dest) {
	SLOG << "Jump to " << dest << std::endl;
	assert(chunks_.find(dest) != chunks_.end());

	transitioning_ = true;
	transitionStartedAt_ = glfwGetTime();

	currentCoord_ = dest;
	currentChunk_ = chunks_.at(dest);

	player_->setChunk(currentChunk_.get());
	player_->update();
	player_->getFirstComponent<PlayerController>()->updateParentChunk();
}

}