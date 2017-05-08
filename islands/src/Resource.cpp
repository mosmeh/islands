#include "Resource.h"
#include "ResourceSystem.h"
#include "NameGenerator.h"
#include "Log.h"

namespace islands {

Resource::Resource() : Resource(NameGenerator::generate("resource")) {}

Resource::Resource(const std::string& name) :
	name_(name),
	status_(State::UNLOADED) {}

const std::string& Resource::getName() const {
	return name_;
}

void Resource::loadAsync() {
	auto unloaded = State::UNLOADED;
	if (status_.compare_exchange_strong(unloaded, State::LOADING)) {
		SLOG << "Loading " << getName() << std::endl;
		ResourceSystem::getInstance().pushToLoadQueue(shared_from_this());
	}
}

bool Resource::isLoaded() const {
	return status_ == State::LOADED || status_ == State::UPLOADED;
}

bool Resource::isUploaded() const {
	return status_ == State::UPLOADED;
}

void Resource::waitUntilLoaded() {
	loadAsync();
	while (!isLoaded());
}

void Resource::upload() {
	if (status_ != State::UPLOADED) {
		SLOG << "Uploading " << getName() << std::endl;
		waitUntilLoaded();
		uploadImpl();
		status_ = State::UPLOADED;
	}
}

Resource::State Resource::getStatus() const {
	return status_;
}

void Resource::setStatus(State status) {
	status_ = status;
}

}