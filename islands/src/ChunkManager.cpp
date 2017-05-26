#include "ChunkManager.h"
#include "PlayerController.h"
#include "ResourceSystem.h"

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

ChunkManager::ChunkManager() : player_(std::make_shared<Entity>("Player")) {
	player_->setPosition({0, 0, 10.f});
	player_->setScale({0.00485f, 0.00485f, 0.006525f});

	player_->attachComponent(std::make_shared<PlayerController>());

	constexpr auto meshName = "character6a.fbx";
	const auto model = ResourceSystem::getInstance().createOrGet<Model>(meshName, meshName);
	const auto drawer = std::make_shared<ModelDrawer>(model);
	player_->attachComponent(drawer);

	const auto collider = std::make_shared<SphereCollider>(model, 1.f);
	player_->attachComponent(collider);

	const auto body = std::make_shared<PhysicalBody>(1.f);
	body->setCollider(collider);
	player_->attachComponent(body);

	chunks_.emplace(glm::uvec3(0), std::make_shared<Chunk>("forest1.json", "forest1.json"));
	jumpTo(glm::uvec3(0));
}

ChunkManager& ChunkManager::getInstance() {
	static ChunkManager instance;
	return instance;
}

void ChunkManager::update() {
	player_->update();
	currentChunk_->update();
}

void ChunkManager::draw() {
	player_->draw();
	currentChunk_->draw();
}

void ChunkManager::jumpTo(const glm::ivec3& dest) {
	assert(chunks_.find(dest) != chunks_.end());
	currentCoord_ = dest;
	currentChunk_ = chunks_.at(dest);
	player_->setChunk(currentChunk_.get());
	currentChunk_->getPhysicsSystem().registerBody(player_->getComponent<PhysicalBody>());
	currentChunk_->getPhysicsSystem().registerCollider(player_->getComponent<Collider>());
}

}