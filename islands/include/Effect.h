#pragma once

#include "Component.h"
#include "Shader.h"
#include "Model.h"

namespace islands {
namespace effect {

class Damage : public Drawable {
public:
	Damage(double duration = 0.3);
	virtual ~Damage() = default;

	void start() override;
	void update() override;
	void draw() override;
	bool isOpaque() const override;

private:
	const double duration_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Program> program_;
	double startedAt_;
};

class Scatter : public Drawable {
public:
	using FinishCallback = std::function<void(void)>;

	Scatter(const FinishCallback& callback);
	virtual ~Scatter() = default;

	void start() override;
	void update() override;
	void draw() override;
	bool isOpaque() const override;

private:
	const FinishCallback callback_;
	std::shared_ptr<ModelDrawer> drawer_;
	std::shared_ptr<Program> program_;
	double startedAt_;
};

}
}
