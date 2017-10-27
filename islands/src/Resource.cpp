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

void Resource::load() {
	if (status_ == State::Unloaded) {
		SLOG << "Loading " << getName() << std::endl;
		loadImpl();
		status_ = State::Loaded;
	}
}

void Resource::upload() {
	if (status_ != State::Uploaded) {
		load();

		SLOG << "Uploading " << getName() << std::endl;
		uploadImpl();
		status_ = State::Uploaded;
	}
}

bool Resource::isLoaded() const {
	return status_ == State::Loaded || status_ == State::Uploaded;
}

bool Resource::isUploaded() const {
	return status_ == State::Uploaded;
}

}