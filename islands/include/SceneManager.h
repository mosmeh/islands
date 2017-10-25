#pragma once

#include "Chunk.h"
#include "Geometry.h"

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

class SceneManager {
public:
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
	virtual ~SceneManager();

	static SceneManager& getInstance();

	void update();
	void draw();

	void jumpTo(const glm::ivec3& destination);

private:
	const std::array<glm::ivec3, 6> NEIGHBOR_OFFSETS;

	std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>> chunks_;
	glm::ivec3 currentCoord_;
	std::shared_ptr<Chunk> currentChunk_;
	std::shared_ptr<Program> fullScreenProgram_;
	GLuint frameBuffer_, fbTexture_, renderBuffer_;
	bool transitioning_;
	double transitionStartedAt_;

	SceneManager();
};

}