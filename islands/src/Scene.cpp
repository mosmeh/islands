#include "Scene.h"
#include "Input.h"
#include "Window.h"
#include "Sound.h"
#include "GameScene.h"

namespace islands {

SceneManager::SceneManager() :
	current_(nullptr),
	blackOutProgram_(Program::createOrGet("BlackOutProgram", 
		Program::ShaderList{
			Shader::createOrGet("full_screen.vert", Shader::Type::Vertex),
			Shader::createOrGet("black_out.frag", Shader::Type::Fragment)})),
	transition_{TransitionState::None, -HUGE_VAL} {

	blackOutProgram_->use();
	blackOutProgram_->setUniform("tex", static_cast<GLuint>(0));

	glGenFramebuffers(1, &frameBuffer_);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_);

	const auto& size = Window::getInstance().getFramebufferSize();
	renderTexture_.setSize(size);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture_.getId(), 0);

	glGenRenderbuffers(1, &renderBuffer_);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer_);

	Window::getInstance().registerFramebufferResizeCallback([&](int width, int height) {
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_);

		renderTexture_.setSize({width, height});

		glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer_);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	});
}

SceneManager::~SceneManager() {
	glDeleteFramebuffers(1, &frameBuffer_);
	glDeleteRenderbuffers(1, &renderBuffer_);
}

SceneManager& SceneManager::getInstance() {
	static SceneManager instance;
	return instance;
}

void SceneManager::update() {
	if (current_) {
		const auto progress = transition_.getProgress();
		switch (transition_.status) {
		case TransitionState::None:
			current_->update();
			break;
		case TransitionState::FadeIn:
			current_->update();
			if (progress >= 1.0) {
				transition_.status = TransitionState::None;
			}
			break;
		case TransitionState::FadeOut:
			if (progress > 0.5) {
				transition_.status = TransitionState::FadeIn;
			}
			break;
		}
	}
}

void SceneManager::draw() {
	if (current_) {
		if (transition_.status != TransitionState::FadeOut) {
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_);
			current_->draw();
		}

		blackOutProgram_->use();
		const auto progress = (transition_.status == TransitionState::None) ?
			1.f : static_cast<glm::float32>(transition_.getProgress());
		blackOutProgram_->setUniform("progress", progress);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		blackOutProgram_->use();
		renderTexture_.bind(0);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glEnable(GL_DEPTH_TEST);
	}
}

void SceneManager::fadeInOut() {
	transition_.status = TransitionState::FadeOut;
	transition_.startedAt = glfwGetTime();
}

std::shared_ptr<Scene> SceneManager::getPreviousScene() const {
	assert(prev_);
	return prev_;
}

double SceneManager::Transition::getProgress() {
	return (glfwGetTime() - startedAt) / 0.5;
}

TitleScene::TitleScene() :
	titleProgram_(Program::createOrGet("TitleProgram",
		Program::ShaderList{
			Shader::createOrGet("sprite.vert", Shader::Type::Vertex),
			Shader::createOrGet("sprite.geom", Shader::Type::Geometry),
			Shader::createOrGet("title.frag", Shader::Type::Fragment)})),
	titleTexture_(Texture2D::createOrGet("title.png")),
	selectedItem_(0),
	repeated_(false) {}

void TitleScene::update() {
	if (Input::getInstance().anyButtonExceptArrowPressed()) {
		if (selectedItem_ == 0) {
			SceneManager::getInstance().changeScene<LevelSelectionScene>();
		} else {
			SceneManager::getInstance().changeScene<CreditScene>();
		}
		Sound::createOrGet("decide.ogg")->createInstance()->play();
	} else {
		const auto& dir = Input::getInstance().getDirection();
		if (dir.y <= -0.8 || 0.8 <= dir.y) {
			if (!repeated_) {
				selectedItem_ = 1 - selectedItem_;
				repeated_ = true;
			}
		} else {
			repeated_ = false;
		}
	}
}

void TitleScene::draw() {
	glClear(GL_COLOR_BUFFER_BIT);

	vertexArray_.bind();
	titleProgram_->use();
	titleProgram_->setUniform("pos", glm::zero<glm::vec2>());
	titleProgram_->setUniform("size", glm::one<glm::vec2>());
	titleProgram_->setUniform("tex", static_cast<GLuint>(0));
	titleProgram_->setUniform("selectedItem", selectedItem_);
	titleProgram_->setUniform("time", static_cast<glm::float32>(glfwGetTime()));
	titleTexture_->bind(0);

	glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
	glDrawArrays(GL_POINTS, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

IntroductionScene::IntroductionScene() :
	keyboardIntroImage_(Texture2D::createOrGet("intro_keyboard.png")),
	gamepadIntroImage_(Texture2D::createOrGet("intro_gamepad.png")),
	dualShock4IntroImage_(Texture2D::createOrGet("intro_dualshock4.png")) {}

void IntroductionScene::update() {
	if (Input::getInstance().anyButtonPressed()) {
		SceneManager::getInstance().changeScene<GameScene>(true, "forest.json");
		Sound::createOrGet("decide.ogg")->createInstance()->play();
	}
}

void IntroductionScene::draw() {
	glClear(GL_COLOR_BUFFER_BIT);
	if (Input::getInstance().getGamepad().isPresent()) {
		if (Input::getInstance().getGamepad().isDualShock4()) {
			dualShock4IntroImage_.draw();
		} else {
			gamepadIntroImage_.draw();
		}
	} else {
		keyboardIntroImage_.draw();
	}
}

LevelSelectionScene::LevelSelectionScene() :
	forestImage_(Texture2D::createOrGet("select_forest.png")),
	seaImage_(Texture2D::createOrGet("select_sea.png")),
	selectedItem_(0),
	repeated_(false) {}

void LevelSelectionScene::update() {
	if (Input::getInstance().anyButtonExceptArrowPressed()) {
		static bool introShowed = false;
		if (introShowed) {
			SceneManager::getInstance().changeScene<GameScene>(
				true, selectedItem_ == 0 ? "forest.json" : "sea.json");
		} else {
			introShowed = true;
			SceneManager::getInstance().changeScene<IntroductionScene>();
		}
		Sound::createOrGet("decide.ogg")->createInstance()->play();
	} else {
		const auto& dir = Input::getInstance().getDirection();
		if (dir.y <= -0.8 || 0.8 <= dir.y) {
			if (!repeated_) {
				selectedItem_ = 1 - selectedItem_;
				repeated_ = true;
			}
		} else {
			repeated_ = false;
		}
	}
}

void LevelSelectionScene::draw() {
	glClear(GL_COLOR_BUFFER_BIT);
	switch (selectedItem_) {
	case 0:
		forestImage_.draw();
		break;
	case 1:
		seaImage_.draw();
		break;
	default:
		throw std::exception("unreachable");
	}
}

CreditScene::CreditScene() : creditImage_(Texture2D::createOrGet("credit.png")) {}

void CreditScene::update() {
	if (Input::getInstance().anyButtonPressed()) {
		SceneManager::getInstance().changeScene<TitleScene>();
		Sound::createOrGet("decide.ogg")->createInstance()->play();
	}
}

void CreditScene::draw() {
	glClear(GL_COLOR_BUFFER_BIT);
	creditImage_.draw();
}

GameOverScene::GameOverScene() :
	gameOverImage_(Texture2D::createOrGet("game_over.png")),
	startedAt_(glfwGetTime()) {

	Sound::createOrGet("game_over.ogg")->createInstance()->play();
}

void GameOverScene::update() {
	if (Input::getInstance().anyButtonPressed()) {
		SceneManager::getInstance().changeScene<TitleScene>();
	}
	gameOverImage_.setAlpha(static_cast<float>(glfwGetTime() - startedAt_));
}

void GameOverScene::draw() {
	SceneManager::getInstance().getPreviousScene()->draw();
	gameOverImage_.draw();
}

GameClearScene::GameClearScene() :
	gameClearImage_(Texture2D::createOrGet("game_clear.png")),
	startedAt_(glfwGetTime()) {

	Sound::createOrGet("game_clear.ogg")->createInstance()->play();
}

void GameClearScene::update() {
	if (Input::getInstance().anyButtonPressed()) {
		SceneManager::getInstance().changeScene<TitleScene>();
	}
	gameClearImage_.setAlpha(static_cast<float>(glfwGetTime() - startedAt_));
}

void GameClearScene::draw() {
	SceneManager::getInstance().getPreviousScene()->draw();
	gameClearImage_.draw();
}

}
