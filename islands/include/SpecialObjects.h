#pragma once

#include "Component.h"

namespace islands {
namespace specialobj {

class Curer : public Component {
public:
	Curer(float radius);
	virtual ~Curer() = default;

	void start() override;
	void update() override;

private:
	float radius_;
	std::shared_ptr<Entity> playerEntity_;
	bool activated_;
};

}
}
