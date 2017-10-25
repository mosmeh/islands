#include "Component.h"
#include "PhysicalBody.h"

namespace islands {

class Slime : public Component {
public:
	Slime();
	void start() override;
	void update() override;

private:
	enum class State {
		Moving,
		ChangingDirection,
		Dying
	} status_;

	std::shared_ptr<PhysicalBody> body_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Entity> playerEntity_;
	glm::vec3 direction_;
	glm::quat targetQuat_;
	double moveStartedAt_;
	glm::vec3 initPos_;
};

}