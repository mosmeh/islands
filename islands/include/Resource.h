#pragma once

#include "NameGenerator.h"
#include "Log.h"

namespace islands {

template <typename T>
class Resource {
public:
	Resource() : Resource(NameGenerator::generate("resource")) {}
	Resource(const std::string& name) :
		name_(name),
		status_(State::Unloaded) {}

	virtual ~Resource() = default;

	const std::string& getName() const {
		return name_;
	}

	void load() {
		if (status_ == State::Unloaded) {
			SLOG << "Loading " << getName() << std::endl;
			loadImpl();
			status_ = State::Loaded;
		}
	}

	void upload() {
		if (status_ != State::Uploaded) {
			load();

			SLOG << "Uploading " << getName() << std::endl;
			uploadImpl();
			status_ = State::Uploaded;
		}
	}

	virtual bool isLoaded() const {
		return status_ == State::Loaded || status_ == State::Uploaded;
	}

	bool isUploaded() const {
		return status_ == State::Uploaded;
	}

	template <class... Args>
	static std::shared_ptr<T> createOrGet(const std::string& name, Args&&... args) {
		const auto iter = getInstances().find(name);
		if (iter == getInstances().end()) {
			const auto instance = std::make_shared<T>(name, std::forward<Args>(args)...);
			getInstances().emplace(name, instance);
			return instance;
		} else {
			return iter->second;
		}
	}

	static std::shared_ptr<T> get(const std::string& name) {
		const auto iter = getInstances().find(name);
		if (iter == getInstances().end()) {
			throw std::exception("not found");
		} else {
			return iter->second;
		}
	}

protected:
	virtual void loadImpl() {}
	virtual void uploadImpl() {}

private:
	enum class State {
		Unloaded,
		Loaded,
		Uploaded
	};

	const std::string name_;
	State status_;

	static std::unordered_map<std::string, std::shared_ptr<T>>& getInstances() {
		static std::unordered_map<std::string, std::shared_ptr<T>> instances;
		return instances;
	}
};

}
