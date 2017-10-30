#include "SceneManager.h"
#include "ResourceSystem.h"
#include "PhysicalBody.h"
#include "Window.h"
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

SceneManager::SceneManager() :
	NEIGHBOR_OFFSETS{
		glm::ivec3(-1, 0, 0), glm::ivec3(1, 0, 0),
		glm::ivec3(0, -1, 0), glm::ivec3(0, 1, 0),
		glm::ivec3(0, 0, -1), glm::ivec3(0, 0, 1)
	},
	fullScreenProgram_(ResourceSystem::getInstance().createOrGet<Program>(
		"blackOut", "full_screen.vert", "black_out.frag")),
	transitioning_(false) {

	fullScreenProgram_->use();
	fullScreenProgram_->setUniform("tex", static_cast<GLuint>(0));

	glGenFramebuffers(1, &frameBuffer_);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_);

	glGenTextures(1, &fbTexture_);
	glBindTexture(GL_TEXTURE_2D, fbTexture_);
	const auto& size = Window::getInstance().getFramebufferSize();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTexture_, 0);

	glGenRenderbuffers(1, &renderBuffer_);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer_);

	Window::getInstance().registerFramebufferResizeCallback([&](int width, int height) {
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_);

		glBindTexture(GL_TEXTURE_2D, fbTexture_);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer_);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	});

	picojson::value json;
	{
		std::ifstream ifs(LEVEL_DIR + sys::getFilePathSeperator() + "levels.json");
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

	jumpTo(glm::ivec3(0));

	currentChunk_->getEntityByName("Player")->setPosition({0.f, 0.f, 2.f});
}

SceneManager::~SceneManager() {
	glDeleteFramebuffers(1, &frameBuffer_);
	glDeleteTextures(1, &fbTexture_);
	glDeleteRenderbuffers(1, &renderBuffer_);
}

SceneManager& SceneManager::getInstance() {
	static SceneManager instance;
	return instance;
}

void SceneManager::update() {
	currentChunk_->update();

	if (!transitioning_) {
		const auto playerEntity = currentChunk_->getEntityByName("Player");
		const auto& playerAABB = playerEntity->getFirstComponent<Collider>()->getGlobalAABB();
		const auto playerVelocity = glm::normalize(playerEntity->getFirstComponent<PhysicalBody>()->getVelocity());
		for (const auto& offset : NEIGHBOR_OFFSETS) {
			const auto destCoord = currentCoord_ + offset;
			if (chunks_.find(destCoord) != chunks_.end()) {
				const auto& destAABB = chunks_.at(destCoord)->getGlobalAABB();
				if (glm::dot(playerVelocity, glm::vec3(offset)) > 0.f &&
					geometry::intersect(destAABB, playerAABB)) {

					jumpTo(destCoord);
				}
			}
		}
	}

	if (transitioning_ && glfwGetTime() - transitionStartedAt_ > 1.0) {
		transitioning_ = false;
	}
}

void SceneManager::draw() {
	if (!transitioning_ || glfwGetTime() - transitionStartedAt_ > 0.5) {
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		currentChunk_->draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	fullScreenProgram_->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbTexture_);
	fullScreenProgram_->setUniform("progress", static_cast<float>(glfwGetTime() - transitionStartedAt_));
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void SceneManager::jumpTo(const glm::ivec3& dest) {
	SLOG << "Jump to " << dest << std::endl;
	assert(chunks_.find(dest) != chunks_.end());

	transitioning_ = true;
	transitionStartedAt_ = glfwGetTime();

	if (currentChunk_) {
		const auto currentPlayer = currentChunk_->getEntityByName("Player");
		const auto nextPlayer = chunks_.at(dest)->getEntityByName("Player");

		static constexpr float JUMP_STEP = 2.f;
		nextPlayer->setPosition(currentPlayer->getPosition()
			+ JUMP_STEP * glm::vec3(dest - currentCoord_));
		nextPlayer->setQuaternion(currentPlayer->getQuaternion());
	}

	currentCoord_ = dest;
	currentChunk_ = chunks_.at(dest);
}

}