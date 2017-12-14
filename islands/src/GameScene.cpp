#include "GameScene.h"
#include "PhysicalBody.h"
#include "Health.h"
#include "Scene.h"
#include "AssetArchive.h"
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

GameScene::GameScene(const std::string& levelFilename) :
	backgroundProgram_(Program::createOrGet("BackgroundProgram",
		Program::ShaderList{
			Shader::createOrGet("full_screen.vert", Shader::Type::Vertex),
			Shader::createOrGet("background.frag", Shader::Type::Fragment)})) {

	picojson::value json;
	{
#ifdef ENABLE_ASSET_ARCHIVE
		const auto filePath = LEVEL_DIR + '/' + levelFilename;
		picojson::parse(json, AssetArchive::getInstance().readTextFile(filePath));
#else
		const auto filePath = LEVEL_DIR + sys::getFilePathSeparator() + levelFilename;
		std::ifstream ifs(filePath);
		ifs >> json;
#endif
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
		chunks_.emplace(coord, chunk);
	}

	jumpTo(glm::ivec3(0));
	playerEntity_->setPosition({0.f, 0.f, 1.f});
}

void GameScene::update() {
	static const std::array<glm::ivec3, 6> NEIGHBOR_OFFSETS{
		glm::ivec3(-1, 0, 0), glm::ivec3(1, 0, 0),
		glm::ivec3(0, -1, 0), glm::ivec3(0, 1, 0),
		glm::ivec3(0, 0, -1), glm::ivec3(0, 0, 1)
	};

	currentChunk_->update();

	const auto& playerAABB = playerEntity_->getFirstComponent<Collider>()->getGlobalAABB();
	const auto playerVelocity = glm::normalize(playerEntity_->getFirstComponent<PhysicalBody>()->getVelocity());
	for (const auto& offset : NEIGHBOR_OFFSETS) {
		const auto destCoord = currentCoord_ + offset;
		if (chunks_.find(destCoord) != chunks_.end()) {
			const auto& destAABB = chunks_.at(destCoord)->getGlobalAABB();
			if (glm::dot(playerVelocity, glm::vec3(offset)) > 0.f &&
				geometry::intersect(destAABB, playerAABB)) {

				jumpTo(destCoord);
				SceneManager::getInstance().fadeInOut();
			}
		}
	}
}

void GameScene::draw() {
	if (currentChunk_) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		backgroundProgram_->use();
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glEnable(GL_DEPTH_TEST);

		currentChunk_->draw();

		healthIndicator_.draw(playerEntity_->getFirstComponent<Health>());
	}
}

void GameScene::onLeave() {
	if (currentBGMInstance_) {
		currentBGMInstance_->stop();
	}
}

void GameScene::jumpTo(const glm::ivec3& dest) {
	SLOG << "Jump to " << dest << std::endl;
	assert(chunks_.find(dest) != chunks_.end());

	const auto nextChunk = chunks_.at(dest);
	if (currentChunk_) {
		const auto nextPlayer = nextChunk->getEntityByName("Player");

		static constexpr float JUMP_STEP = 2.f;
		nextPlayer->setPosition(playerEntity_->getPosition()
			+ JUMP_STEP * glm::vec3(dest - currentCoord_));
		nextPlayer->setQuaternion(playerEntity_->getQuaternion());

		{
			const auto current = playerEntity_->getFirstComponent<Health>();
			const auto next = nextPlayer->getFirstComponent<Health>();
			next->set(current->get());
			next->setMaxHealth(current->getMaxHealth());
		}

		playerEntity_ = nextPlayer;

		if (nextChunk->getBGM() != currentBGM_) {
			currentBGMInstance_->stop();
			currentBGMInstance_ = nextChunk->getBGM()->createInstance();
			currentBGMInstance_->play(true);
		}
		currentChunk_ = nextChunk;
	} else {
		currentChunk_ = nextChunk;
		currentCoord_ = dest;

		playerEntity_ = currentChunk_->getEntityByName("Player");
		currentBGM_ = currentChunk_->getBGM();
		currentBGMInstance_ = currentBGM_->createInstance();
		currentBGMInstance_->play(true);
	}

	currentCoord_ = dest;
	currentBGM_ = currentChunk_->getBGM();
}

}
