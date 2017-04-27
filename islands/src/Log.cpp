#include "Log.h"

namespace islands {

const char* getFormattedTime() {
	time_t t;
	std::time(&t);
#ifdef _MSC_VER
	struct tm time;
	localtime_s(&time, &t);
#else
	const auto time = *std::localtime(&t);
#endif

	static char buf[32];
	strftime(buf, sizeof(buf), "%Y/%m/%d %I:%M:%S", &time);

	return buf;
}

}