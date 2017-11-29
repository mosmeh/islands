#pragma once

#include "Component.h"
#include "PhysicalBody.h"
#include "Model.h"
#include "Health.h"

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
	std::shared_ptr<Health> health_;
	double attackAnimStartedAt_;

	void start() override;
};

}
