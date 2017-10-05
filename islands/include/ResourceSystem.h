#pragma once

#include "Resource.h"
#include "Shader.h"

namespace islands {

class ResourceSystem {
public:
	enum class ProgramType {
		Default,
		Skinning,
		Lightmap
	};

	ResourceSystem(const ResourceSystem&) = delete;
	ResourceSystem& operator=(const ResourceSystem&) = delete;
	virtual ~ResourceSystem() = default;

	static ResourceSystem& getInstance();

	template <class T, class... Args>
	std::enable_if_t<std::is_base_of<Resource, T>::value, std::shared_ptr<T>>
	createOrGet(const std::string& name, Args... args);

	template <class T>
	std::enable_if_t<std::is_base_of<Resource, T>::value, std::shared_ptr<T>>
	get(const std::string& name) const;

	void setDefaultProgram(ProgramType type, std::shared_ptr<Program> program);
	std::shared_ptr<Program> getDefaultProgram(ProgramType type = ProgramType::Default) const;

private:
	std::unordered_map<std::string, std::shared_ptr<Resource>> resources_;
	std::unordered_map<ProgramType, std::shared_ptr<Program>> defaultPrograms_;

	ResourceSystem() = default;
};

template<class T, class... Args>
inline std::enable_if_t<std::is_base_of<Resource, T>::value, std::shared_ptr<T>>
ResourceSystem::createOrGet(const std::string& name, Args... args) {
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