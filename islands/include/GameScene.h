#pragma once

#include "Chunk.h"
#include "HealthIndicator.h"
#include "Scene.h"

namespace islands {
namespace detail {

std::uint64_t fold(std::int64_t x);
std::uint64_t cantorPair(std::uint64_t a, std::uint64_t b);

}
}

namespace std {

template <>
struct hash<glm::ivec3> {
	std::uint64_t operator()(const glm::ivec3& v) const {
		using namespace islands::detail;
		return cantorPair(cantorPair(fold(v.x), fold(v.y)), fold(v.z));
	}
};

}

namespace islands {

class GameScene : public Scene {
public:
	GameScene(const std::string& levelFilename);
	virtual ~GameScene() = default;

	void update();
	void draw();
	void onLeave();

private:
	std::shared_ptr<Program> backgroundProgram_;
	HealthIndicator healthIndicator_;

	std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>> chunks_;
	glm::ivec3 currentCoord_;
	std::shared_ptr<Chunk> currentChunk_;
	std::shared_ptr<Entity> playerEntity_;

	std::shared_ptr<Sound> currentBGM_;
	std::shared_ptr<Sound::Instance> currentBGMInstance_;

	void jumpTo(const glm::ivec3& destination);
};

}
