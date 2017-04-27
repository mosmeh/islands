#pragma once

namespace islands {

class NameGenerator {
public:
	NameGenerator(const NameGenerator&) = delete;
	NameGenerator& operator=(const NameGenerator&) = delete;
	virtual ~NameGenerator() = default;

	static std::string generate(const std::string& prefix = "n") {
		static NameGenerator instance;
		std::stringstream ss;
		ss << prefix << "_" << instance.n_++;
		return ss.str();
	}

private:
	unsigned long int n_;
	NameGenerator() : n_(0) {}
};

}