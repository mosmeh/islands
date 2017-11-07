#pragma once

#include "Component.h"
#include "Shader.h"
#include "Model.h"

namespace islands {

class DamageEffect : public ModelDrawer {
public:
	DamageEffect(std::shared_ptr<Model> model, double duration = 0.3);
	virtual ~DamageEffect() = default;

	void draw() override;

	void activate();
	bool isActive() const;

private:
	const double duration_;
	std::shared_ptr<Program> program_;
	double startedAt_;
};

class ScatterEffect : public ModelDrawer {
public:
	ScatterEffect(std::shared_ptr<Model> model);
	virtual ~ScatterEffect() = default;

	void start() override;
	void update() override;
	void draw() override;

	bool isFinished() const;

private:
	std::shared_ptr<Program> program_;
	double startedAt_;
};

}