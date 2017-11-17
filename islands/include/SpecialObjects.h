#pragma once

#include "Component.h"

namespace islands {
namespace specialobj {

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
}
