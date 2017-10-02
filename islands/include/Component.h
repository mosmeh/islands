#pragma once

#include "Entity.h"

namespace islands {

class Chunk;

class Component {
public:
	Component() :
		entity_(nullptr),
		isFirstUpdate_(true) {}
	virtual ~Component() = default;

	void setEntity(Entity* entity) {
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

protected:
	Chunk& getChunk() const {
		return getEntity().getChunk();
	}

	virtual void start() {}
	virtual void update() = 0;

private:
	Entity* entity_;
	bool isFirstUpdate_;
};

class Drawable : public Component {
public:
	Drawable() = default;
	virtual ~Drawable() = default;

	virtual void update() {}
	virtual void draw() = 0;
};

}