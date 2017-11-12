#pragma once

#include "System.h"

namespace islands {

class Logger : public std::ostream {
public:
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	virtual ~Logger() = default;

	static std::unique_lock<std::mutex> lock() {
		static std::mutex mutex;
		return std::unique_lock<std::mutex>(mutex);
	}

	static Logger& getInstance() {
		static Logger instance;
		return instance;
	}

private:
	class LogBuffer : public std::stringbuf {
	public:
		LogBuffer() : ofs_("islands.log") {}

		virtual ~LogBuffer() {
			pubsync();
		}

		int sync() override {
			if (str() == "") {
				return 0;
			}

			const auto time = sys::getTime();
			static char buf[32];
			strftime(buf, sizeof(buf), "%Y/%m/%d %I:%M:%S", &time);

			std::clog << buf << str();
			std::clog.flush();

			ofs_ << buf << str();
			ofs_.flush();

			str("");
			return 0;
		}

	private:
		std::ofstream ofs_;
	} buffer_;

	Logger() : std::ostream(&buffer_) {}
};

#define SLOG ::islands::Logger::lock(), ::islands::Logger::getInstance() << \
	" [" << ::islands::sys::basename(__FILE__) << ":" << __LINE__ << "] "


}