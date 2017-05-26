#pragma once

#include "Chunk.h"

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

class ChunkManager {
public:
	ChunkManager(const ChunkManager&) = delete;
	ChunkManager& operator=(const ChunkManager&) = delete;
	virtual ~ChunkManager() = default;

	static ChunkManager& getInstance();

	void update();
	void draw();

	void jumpTo(const glm::ivec3& destination);

private:
	std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>> chunks_;
	glm::uvec3 currentCoord_;
	std::shared_ptr<Chunk> currentChunk_;
	std::shared_ptr<Entity> player_;

	ChunkManager();
};

}