
#pragma once

#include "Component.h"
#include "PhysicalBody.h"
#include "Model.h"

namespace islands {

class PlayerController : public Component {
public:
	PlayerController();
	virtual ~PlayerController() = default;

	void awake() override;
	void update() override;

private:
	std::shared_ptr<PhysicalBody> body_;
	std::shared_ptr<ModelDrawer> drawer_;
	bool attacking_;
};

}
