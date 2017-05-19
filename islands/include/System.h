#pragma once

#if !defined(_WIN32) && (defined(__WIN32__) || defined(WIN32) || defined(__MINGW32__))
#define _WIN32
#endif

namespace islands {
namespace sys {

namespace {

constexpr char getFilePathSeperator() {
#ifdef _WIN32
	return  '\\';
#else
	return '/';
#endif
}

constexpr const char* basenameImpl(const char* dp, const char* p) {
	return *p == '\0' ? dp : *p == sys::getFilePathSeperator() ?
		basenameImpl(p + 1, p + 1) : basenameImpl(dp, p + 1);
}

}

constexpr const char* basename(const char* s) {
	return basenameImpl(s, s);
}

std::string getVersionString();
void disableIME(GLFWwindow* window);
void sleep(std::chrono::milliseconds duration);

struct MemoryStatus {
	uint64_t available_bytes, total_bytes;
};

MemoryStatus getPhysicalMemoryStatus();

}
}