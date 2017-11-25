#pragma once

namespace islands {

class Resource {
public:
	Resource();
	Resource(const std::string& name);

	virtual ~Resource() = default;

	const std::string& getName() const;
	void load();
	void upload();
	virtual bool isLoaded() const;
	bool isUploaded() const;

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
};

template <typename T>
class SharedResource : public Resource {
public:
	using Resource::Resource;

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

private:
	static auto& getInstances() {
		static std::unordered_map<std::string, std::shared_ptr<T>> instances;
		return instances;
	}
};

}
