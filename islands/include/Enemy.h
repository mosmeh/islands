#include "Component.h"
#include "PhysicalBody.h"
#include "Health.h"
#include "Effect.h"

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

}