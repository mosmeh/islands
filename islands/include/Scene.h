#pragma once

#include "Sprite.h"
#include "Shader.h"

namespace islands {

class Scene {
public:
	Scene() = default;
	virtual ~Scene() = default;

	virtual void update() = 0;
	virtual void draw() = 0;
};

enum class SceneKey {
	Title,
	Introduction,
	Credit,
	Game,
	GameOver,
	GameClear
};

class SceneManager {
public:
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
	static SceneManager& getInstance();

	void update();
	void draw();

	template <class T>
	std::enable_if_t<std::is_base_of<Scene, T>::value, bool>
	add(const SceneKey& key);

	bool changeScene(const SceneKey& key, bool fade = true);
	std::shared_ptr<Scene> getPreviousScene() const;

private:
	enum class TransitionState {
		None,
		FadeOut,
		FadeIn
	};

	std::unordered_map<SceneKey, std::function<std::shared_ptr<Scene>(void)>> factories_;
	std::shared_ptr<Scene> prev_, current_;

	std::shared_ptr<Program> blackOutProgram_;
	RenderTexture renderTexture_;
	GLuint frameBuffer_, renderBuffer_;
	struct Transition {
		TransitionState status;
		double startedAt;
		double getProgress();
	} transition_;

	SceneManager();
	virtual ~SceneManager();
};

template<class T>
inline std::enable_if_t<std::is_base_of<Scene, T>::value, bool>
SceneManager::add(const SceneKey& key) {
	if (factories_.find(key) != factories_.end()) {
		return false;
	}

	factories_.emplace(key, []() {
		return std::make_shared<T>();
	});
	return true;
}

class TitleScene : public Scene {
public:
	TitleScene();
	virtual ~TitleScene() = default;
	
	void update() override;
	void draw() override;

private:
	VertexArray vertexArray_;
	std::shared_ptr<Program> titleProgram_;
	std::shared_ptr<Texture2D> titleTexture_;
	size_t selectedItem_;
	bool repeated_;
};

class IntroductionScene : public Scene {
public:
	IntroductionScene();
	virtual ~IntroductionScene() = default;
	
	void update() override;
	void draw() override;

private:
	Sprite keyboardIntroImage_, gamepadIntroImage_, dualShock4IntroImage_;
};

class CreditScene : public Scene {
public:
	CreditScene();
	virtual ~CreditScene() = default;
	
	void update() override;
	void draw() override;

private:
	Sprite creditImage_;
};

class GameScene : public Scene {
public:
	GameScene();
	virtual ~GameScene() = default;

	void update() override;
	void draw() override;
};

class GameOverScene : public Scene {
public:
	GameOverScene();
	virtual ~GameOverScene() = default;

	void update() override;
	void draw() override;

private:
	Sprite gameOverImage_;
	double startedAt_;
};

class GameClearScene : public Scene {
public:
	GameClearScene();
	virtual ~GameClearScene() = default;

	void update() override;
	void draw() override;

private:
	Sprite gameClearImage_;
	double startedAt_;
};

}