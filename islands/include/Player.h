#pragma once

#include "Component.h"
#include "PhysicalBody.h"
#include "Model.h"
#include "FireBall.h"
#include "Health.h"
#include "Effect.h"

namespace islands {

class Player : public Component {
public:
	Player();
	virtual ~Player() = default;

	void update() override;

private:
	enum class State {
		Idling,
		Walking,
		PreFire,
		PostFire,
		Dead
	} status_;

	std::shared_ptr<PhysicalBody> body_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Model> model_;
	std::shared_ptr<Health> health_;
	std::shared_ptr<DamageEffect> damageEffect_;
	std::shared_ptr<ScatterEffect> dyingEffect_;
	double attackAnimStartedAt_;

	void start() override;
};

}
