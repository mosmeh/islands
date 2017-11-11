#pragma once

#include "Component.h"
#include "Shader.h"
#include "Model.h"

namespace islands {

class DamageEffect : public Drawable {
public:
	DamageEffect(double duration = 0.3);
	virtual ~DamageEffect() = default;

	void start() override;
	void update() override;
	void draw() override;

private:
	const double duration_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Program> program_;
	double startedAt_;
};

class ScatterEffect : public Drawable {
public:
	using FinishCallback = std::function<void(void)>;

	ScatterEffect(const FinishCallback& callback);
	virtual ~ScatterEffect() = default;

	void start() override;
	void update() override;
	void draw() override;

private:
	const FinishCallback callback_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Program> program_;
	double startedAt_;
};

}