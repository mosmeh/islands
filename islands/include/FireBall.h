#pragma once
#include <Component.h>
#include <Entity.h>
#include <PhysicalBody.h>

namespace islands {

class FireBall : public Component {
public:
	FireBall() = default;
	virtual ~FireBall() = default;

	void start() override;
	void update() override;

	void fire() const;

private:
	std::shared_ptr<Entity> entity_;
	std::shared_ptr<PhysicalBody> body_;
	std::shared_ptr<ModelDrawer> drawer_;
};

}