#pragma once

#include "ResourceSystem.h"
#include "Log.h"

namespace islands {

ResourceSystem& ResourceSystem::getInstance() {
	static ResourceSystem instance;
	return instance;
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

}
