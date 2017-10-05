
#pragma once

#include "Component.h"
#include "PhysicalBody.h"
#include "Model.h"

namespace islands {

class PlayerController : public Component {
public:
	PlayerController();
	virtual ~PlayerController() = default;

	void update() override;
	void updateParentChunk();

private:
	std::shared_ptr<PhysicalBody> body_;
	std::shared_ptr<ModelDrawer> drawer_;
	bool attacking_;

	std::shared_ptr<Entity> ball_;
	std::shared_ptr<PhysicalBody> ballBody_;

	void start() override;
};

}
