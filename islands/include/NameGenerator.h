#pragma once

namespace islands {

class NameGenerator {
public:
	NameGenerator(const NameGenerator&) = delete;
	NameGenerator& operator=(const NameGenerator&) = delete;
	virtual ~NameGenerator() = default;

	static std::string generate(const std::string& postfix = "n") {
		static NameGenerator instance;
		std::stringstream ss;
		ss << instance.n_++ << "_" << postfix;
		return ss.str();
	}

private:
	unsigned long int n_;

	NameGenerator() : n_(0) {}
};

}
