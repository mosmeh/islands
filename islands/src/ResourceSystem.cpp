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

void ResourceSystem::setDefaultSkinningProgram(const std::string& name) {
	defaultSkinningProgram_ = get<Program>(name);
}

std::shared_ptr<Program> ResourceSystem::getDefaultSkinningProgram() const {
	return defaultSkinningProgram_;
}

void ResourceSystem::pushToLoadQueue(std::shared_ptr<Resource> resource) {
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
		loadQueue_.pop();
		loadQueueMutex_.unlock();

		assert(resource->getStatus() == Resource::State::LOADING);
		resource->loadImpl();
		resource->setStatus(Resource::State::LOADED);
	}
}

}
