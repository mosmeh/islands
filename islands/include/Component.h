#pragma once

#include "Entity.h"

namespace islands {

class Chunk;

class Component {
public:
	Component() :
		entity_(nullptr),
		isFirstUpdate_(true),
		destroyed_(false) {}
	virtual ~Component() = default;

	void setEntity(std::shared_ptr<Entity> entity) {
		entity_ = entity;
	}

	Entity& getEntity() const {
		assert(entity_);
		return *entity_;
	}

	void startAndUpdate() {
		if (isFirstUpdate_) {
			start();
			isFirstUpdate_ = false;
		}

		update();
	}

	void destroy() {
		destroyed_ = true;
	}

	bool isDestroyed() const {
		return destroyed_;
	}

protected:
	Chunk& getChunk() const {
		return getEntity().getChunk();
	}

	virtual void start() {}
	virtual void update() = 0;

	bool isFirstUpdate_;

private:
	std::shared_ptr<Entity> entity_;
	bool destroyed_;
};

class Drawable : public Component {
public:
	Drawable() = default;
	virtual ~Drawable() = default;

	void startAndDraw() {
		if (isFirstUpdate_) {
			start();
			isFirstUpdate_ = false;
		}

		draw();
	}

protected:
	virtual void update() {}
	virtual void draw() = 0;
};

}
