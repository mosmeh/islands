#pragma once

#include "Entity.h"

namespace islands {

class Chunk;

class Component {
public:
	Component() :
		entity_(nullptr) {}
	virtual ~Component() = default;

	void setEntity(Entity* entity) {
		entity_ = entity;
	}

	Entity& getEntity() const {
		assert(entity_);
		return *entity_;
	}

	void startOnce() {
		static bool isFirstTime = true;
		if (isFirstTime) {
			start();
			isFirstTime = false;
		}
	}

	virtual void update() = 0;

protected:
	Chunk& getChunk() const {
		return getEntity().getChunk();
	}

	virtual void start() {}

private:
	Entity* entity_;
};

class Drawable : public Component {
public:
	Drawable() = default;
	virtual ~Drawable() = default;

	virtual void update() {}
	virtual void draw() = 0;
};

}