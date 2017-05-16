#pragma once

#include "ResourceSystem.h"
#include "Log.h"

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

void ResourceSystem::pushToLoadQueue(std::shared_ptr<Resource> resource) {
	loadQueueMutex_.lock();
	loadQueue_.push(resource);
	loadQueueMutex_.unlock();
}

void ResourceSystem::setDefaultProgram(ProgramType type, std::shared_ptr<Program> program) {
	if (defaultPrograms_.find(type) == defaultPrograms_.end()) {
		defaultPrograms_.emplace(type, program);
	} else {
		defaultPrograms_.at(type) = program;
	}
}

std::shared_ptr<Program> ResourceSystem::getDefaultProgram(ProgramType type) const {
	assert(defaultPrograms_.find(type) != defaultPrograms_.end());
	return defaultPrograms_.at(type);
}

void ResourceSystem::consumeLoadQueue() {
	while (!loaderShouldStop_) {
		loadQueueMutex_.lock();
		if (loadQueue_.empty()) {
			loadQueueMutex_.unlock();
			continue;
		}
		const auto resource = loadQueue_.front();
		loadQueue_.pop();
		loadQueueMutex_.unlock();

		assert(resource->getStatus() == Resource::State::Loading);
		resource->loadImpl();
		resource->setStatus(Resource::State::Loaded);
	}
}

}
