#pragma once

namespace islands {

namespace {

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__MINGW32__)
constexpr char separator = '\\';
#else
constexpr char separator = '/';
#endif

constexpr const char* basenameImpl(const char* dp, const char* p) {
	return *p == '\0' ? dp : *p == separator ? basenameImpl(p + 1, p + 1) : basenameImpl(dp, p + 1);
}

}

constexpr const char* basename(const char* s) {
	return basenameImpl(s, s);
}

const char* getFormattedTime();

#define SLOG std::clog << ::islands::getFormattedTime() << \
	" [" << ::islands::basename(__FILE__) << ":" << __LINE__ << "] "

}