#pragma once

#include "Resource.h"
#include "Shader.h"

namespace islands {

class ResourceSystem {
public:
	ResourceSystem(const ResourceSystem&) = delete;
	ResourceSystem& operator=(const ResourceSystem&) = delete;
	virtual ~ResourceSystem();

	static ResourceSystem& getInstance();
	void pushToLoadQueue(std::shared_ptr<Resource> resource);

	template <class Real, class... Args>
	std::enable_if_t<std::is_base_of<Resource, Real>::value, std::shared_ptr<Real>>
	create(const std::string& name, Args... args);

	template <class Real>
	std::enable_if_t<std::is_base_of<Resource, Real>::value, std::shared_ptr<Real>>
	get(const std::string& name) const;

	void setDefaultProgram(const std::string& name);
	std::shared_ptr<Program> getDefaultProgram() const;

private:
	std::unordered_map<std::string, std::shared_ptr<Resource>> resources_;
	std::shared_ptr<Program> defaultProgram_;

	std::queue<std::shared_ptr<Resource>> loadQueue_;
	std::mutex loadQueueMutex_;
	std::thread loader_;
	std::atomic_bool loaderShouldStop_;

	ResourceSystem();

	void consumeLoadQueue();
};

template<class Real, class... Args>
inline std::enable_if_t<std::is_base_of<Resource, Real>::value, std::shared_ptr<Real>>
ResourceSystem::create(const std::string& name, Args... args) {
	if (resources_.find(name) == resources_.end()) {
		const auto resource = std::make_shared<Real>(name, std::forward<Args>(args)...);
		resources_.emplace(name, resource);
		return resource;
	} else {
		return get<Real>(name);
	}
}

template<class Real>
inline std::enable_if_t<std::is_base_of<Resource, Real>::value, std::shared_ptr<Real>>
ResourceSystem::get(const std::string& name) const {
	return std::dynamic_pointer_cast<Real>(resources_.at(name));
}

}