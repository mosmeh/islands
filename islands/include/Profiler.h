#pragma once

namespace islands {

#ifdef _DEBUG

class Profiler {
public:
	Profiler(const Profiler&) = delete;
	Profiler& operator=(const Profiler&) = delete;
	virtual ~Profiler() = default;

	static Profiler& getInstance() {
		static Profiler instance;
		return instance;
	}

	using Real = float;

	void markFrame() {
		const auto time = static_cast<Real>(glfwGetTime());
		if (prevTime_ >= 0) {
			lastDeltaTime_ = time - prevTime_;
		}
		prevTime_ = time;
	}

	Real getLastFPS() const {
		return 1.f / lastDeltaTime_;
	}

	Real getLastDeltaTime() const {
		return lastDeltaTime_;
	}

	void enterSection(const std::string& name) {
		if (samples_.find(name) == samples_.end()) {
			samples_.emplace(name, Sample{static_cast<Real>(glfwGetTime()), 0});
		} else {
			samples_.at(name).startTime = static_cast<Real>(glfwGetTime());
		}
	}

	void leaveSection(const std::string& name) {
		samples_.at(name).elapsedTime += static_cast<Real>(glfwGetTime()) - samples_.at(name).startTime;
	}

	Real getElapsedTime(const std::string& name) const {
		return samples_.at(name).elapsedTime;
	}

	void clearSamples() {
		samples_.clear();
	}

private:
	struct Sample {
		Real startTime;
		Real elapsedTime;
	};

	Real prevTime_;
	Real lastDeltaTime_;
	std::unordered_map<std::string, Sample> samples_;

	Profiler() :
		prevTime_(-INFINITY),
		lastDeltaTime_(NAN) {}
};

#endif

}