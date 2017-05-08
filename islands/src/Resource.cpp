#include "Resource.h"
#include "ResourceSystem.h"
#include "NameGenerator.h"
#include "Log.h"

namespace islands {

Resource::Resource() : Resource(NameGenerator::generate("resource")) {}

Resource::Resource(const std::string& name) :
	name_(name),
	status_(State::Unloaded) {}

const std::string& Resource::getName() const {
	return name_;
}

void Resource::loadAsync() {
	auto unloaded = State::Unloaded;
	if (status_.compare_exchange_strong(unloaded, State::Loading)) {
		SLOG << "Loading " << getName() << std::endl;
		ResourceSystem::getInstance().pushToLoadQueue(shared_from_this());
	}
}

bool Resource::isLoaded() const {
	return status_ == State::Loaded || status_ == State::Uploaded;
}

bool Resource::isUploaded() const {
	return status_ == State::Uploaded;
}

void Resource::waitUntilLoaded() {
	loadAsync();
	while (!isLoaded());
}

void Resource::upload() {
	if (status_ != State::Uploaded) {
		SLOG << "Uploading " << getName() << std::endl;
		waitUntilLoaded();
		uploadImpl();
		status_ = State::Uploaded;
	}
}

Resource::State Resource::getStatus() const {
	return status_;
}

void Resource::setStatus(State status) {
	status_ = status;
}

}