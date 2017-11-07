#include "Component.h"
#include "PhysicalBody.h"
#include "Health.h"
#include "Effect.h"
#include "Sound.h"

namespace islands {

class Slime : public Component {
public:
	Slime();
	virtual ~Slime() = default;

	void start() override;
	void update() override;

private:
	enum class State {
		Moving,
		ChangingDirection,
		Dead
	} status_;

	std::shared_ptr<PhysicalBody> body_;
	std::shared_ptr<Model> model_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Health> health_;
	std::shared_ptr<Entity> playerEntity_;
	std::shared_ptr<DamageEffect> damageEffect_;
	std::shared_ptr<ScatterEffect> dyingEffect_;
	glm::vec3 direction_;
	glm::quat targetQuat_;
	double stateChangedAt_;
	glm::vec3 initPos_;
};

class BigSlime : public Component {
public:
	BigSlime();
	virtual ~BigSlime() = default;

	void start() override;
	void update() override;

private:
	enum class State {
		Pausing,
		Jumping,
		Dead
	} status_;

	std::shared_ptr<PhysicalBody> body_;
	std::shared_ptr<Model> model_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Health> health_;
	std::shared_ptr<Entity> playerEntity_;
	std::shared_ptr<DamageEffect> damageEffect_;
	std::shared_ptr<ScatterEffect> dyingEffect_;
	double stateChangedAt_;
};

class Rabbit : public Component {
public:
	Rabbit();
	virtual ~Rabbit() = default;

	void start() override;
	void update() override;

private:
	enum class State {
		Jumping,
		Pausing,
		PreAttack,
		Attacking,
		Dead
	} status_;

	std::shared_ptr<PhysicalBody> body_;
	std::shared_ptr<Model> model_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Health> health_;
	std::shared_ptr<Entity> playerEntity_;
	std::shared_ptr<DamageEffect> damageEffect_;
	std::shared_ptr<ScatterEffect> dyingEffect_;
	std::shared_ptr<Entity> attackEntity_;
	double stateChangedAt_;
	glm::vec3 direction_;
};

class Crab : public Component {
public:
	Crab();
	virtual ~Crab() = default;

	void start() override;
	void update() override;

private:
	enum class State {
		Pausing,
		Moving,
		PreAttack,
		Attacking,
		Dead
	} status_;

	std::shared_ptr<PhysicalBody> body_;
	std::shared_ptr<Model> model_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Health> health_;
	std::shared_ptr<Entity> playerEntity_;
	std::shared_ptr<DamageEffect> damageEffect_;
	std::shared_ptr<ScatterEffect> dyingEffect_;
	std::shared_ptr<Entity> attackEntity_;
	double stateChangedAt_;
	glm::vec3 direction_;
};

class Dragon : public Component {
public:
	Dragon();
	virtual ~Dragon() = default;

	void start() override;
	void update() override;

private:
	enum class State {
		Hovering,
		Moving,
		PreFire,
		PostFire,
		Descending,
		Tackling,
		Ascending,
		Dead
	} status_;

	std::mt19937 engine_;
	std::shared_ptr<PhysicalBody> body_;
	std::shared_ptr<Model> model_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Health> health_;
	std::shared_ptr<Entity> playerEntity_;
	std::shared_ptr<DamageEffect> damageEffect_;
	std::shared_ptr<ScatterEffect> dyingEffect_;
	std::shared_ptr<Entity> attackEntity_;
	double stateChangedAt_;
	glm::vec3 direction_;
	double targetDelta_;

	void lookAtPlayer();
};

class TotemPoll : public Component {
public:
	TotemPoll();
	virtual ~TotemPoll() = default;

	void start() override;
	void update() override;

private:
	std::shared_ptr<Entity> playerEntity_;
	bool activated_;
};

}