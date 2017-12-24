#pragma once

#include "Component.h"
#include "Shader.h"
#include "Model.h"

namespace islands {
namespace effect {

class Damage : public Component {
public:
	Damage(double duration = 0.3);
	virtual ~Damage() = default;

	void start() override;
	void update() override;

private:
	const double duration_;
	std::shared_ptr<ModelDrawer> drawer_;
	double startedAt_;
};

class Scatter : public Component {
public:
	using FinishCallback = std::function<void(void)>;

	Scatter(const FinishCallback& callback);
	virtual ~Scatter() = default;

	void start() override;
	void update() override;

private:
	const FinishCallback callback_;
	std::shared_ptr<ModelDrawer> drawer_;
	double startedAt_;
};

class Sea : public Component {
public:
	Sea() = default;
	virtual ~Sea() = default;

	void start() override;
	void update() override;

private:
	std::shared_ptr<ModelDrawer> drawer_;
};

class SwimRing : public Component {
public:
	SwimRing() = default;
	virtual ~SwimRing() = default;

	void start() override;
	void update() override;

private:
	glm::vec3 initPos_;
	double startedAt_;
};

class Fish : public Component {
public:
	Fish() = default;
	virtual ~Fish() = default;

	void start() override;
	void update() override;

private:
	glm::vec3 initPos_;
	double startedAt_;
};

}
}
