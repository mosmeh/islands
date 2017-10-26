#pragma once

#include "ResourceSystem.h"
#include "Log.h"

namespace islands {

ResourceSystem& ResourceSystem::getInstance() {
	static ResourceSystem instance;
	return instance;
}

}
