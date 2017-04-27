#pragma once

#include "ResourceSystem.h"

namespace islands {

ResourceSystem::ResourceSystem() :
	loaderShouldStop_(false),
	loader_([&] { consumeLoadQueue(); }) {}

ResourceSystem& ResourceSystem::getInstance() {
	static ResourceSystem instance;
	return instance;
}

ResourceSystem::~ResourceSystem() {
	loaderShouldStop_ = true;
	loader_.join();
}

void ResourceSystem::setDefaultProgram(const std::string& name) {
	defaultProgram_ = get<Program>(name);
}

std::shared_ptr<Program> ResourceSystem::getDefaultProgram() const {
	return defaultProgram_;
}

void ResourceSystem::pushToLoadQueue(std::shared_ptr<Resource> resource) {
	assert(resource->getStatus() == Resource::State::UNLOADED);
	resource->setStatus(Resource::State::LOADING);

	loadQueueMutex_.lock();
	loadQueue_.push(resource);
	loadQueueMutex_.unlock();
}

void ResourceSystem::consumeLoadQueue() {
	while (!loaderShouldStop_) {
		loadQueueMutex_.lock();
		if (loadQueue_.empty()) {
			loadQueueMutex_.unlock();
			continue;
		}
		const auto resource = loadQueue_.front();
		assert(resource->getStatus() == Resource::State::LOADING);
		loadQueue_.pop();
		loadQueueMutex_.unlock();

		resource->loadImpl();
		resource->setStatus(Resource::State::LOADED);
	}
}

}
