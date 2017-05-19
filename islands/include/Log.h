#pragma once

#include "System.h"

namespace islands {

const char* getFormattedTime();

#define SLOG std::clog << ::islands::getFormattedTime() << \
	" [" << ::islands::sys::basename(__FILE__) << ":" << __LINE__ << "] "

}