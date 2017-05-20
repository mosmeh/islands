#include "Log.h"

namespace islands {

const char* getFormattedTime() {
	const auto time = sys::getTime();
	static char buf[32];
	strftime(buf, sizeof(buf), "%Y/%m/%d %I:%M:%S", &time);

	return buf;
}

}