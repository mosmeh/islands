#pragma once
#include "Component.h"

namespace islands {

class FireBall : public Component {
public:
	FireBall(const glm::vec3& pos, const glm::quat& quat);
	virtual ~FireBall() = default;

	void start() override;
	void update() override;

private:
	glm::vec3 pos_;
	glm::quat quat_;
};

}
