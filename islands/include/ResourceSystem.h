#pragma once

#include "Resource.h"

namespace islands {

class ResourceSystem {
public:
	ResourceSystem(const ResourceSystem&) = delete;
	ResourceSystem& operator=(const ResourceSystem&) = delete;
	virtual ~ResourceSystem() = default;

	static ResourceSystem& getInstance();

	template <class T, class... Args>
	std::enable_if_t<std::is_base_of<Resource, T>::value, std::shared_ptr<T>>
	createOrGet(const std::string& name, Args&&... args);

	template <class T>
	std::enable_if_t<std::is_base_of<Resource, T>::value, std::shared_ptr<T>>
	get(const std::string& name) const;

private:
	std::unordered_map<std::string, std::shared_ptr<Resource>> resources_;

	ResourceSystem() = default;
};

template<class T, class... Args>
inline std::enable_if_t<std::is_base_of<Resource, T>::value, std::shared_ptr<T>>
ResourceSystem::createOrGet(const std::string& name, Args&&... args) {
	if (resources_.find(name) == resources_.end()) {
		const auto resource = std::make_shared<T>(name, std::forward<Args>(args)...);
		resources_.emplace(name, resource);
		return resource;
	} else {
		return get<T>(name);
	}
}

template<class T>
inline std::enable_if_t<std::is_base_of<Resource, T>::value, std::shared_ptr<T>>
ResourceSystem::get(const std::string& name) const {
	assert(resources_.find(name) != resources_.end());
	const auto resource = std::dynamic_pointer_cast<T>(resources_.at(name));
	assert(resource);
	return resource;
}

}
