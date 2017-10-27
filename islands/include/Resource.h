#pragma once

namespace islands {

class Resource {
public:
	Resource();
	Resource(const std::string& name);

	virtual ~Resource() = default;

	const std::string& getName() const;
	virtual void loadImpl() {}
	virtual void uploadImpl() {}

	void load();
	void upload();
	virtual bool isLoaded() const;
	bool isUploaded() const;

private:
	enum class State {
		Unloaded,
		Loaded,
		Uploaded
	};

	const std::string name_;
	std::atomic<State> status_;
};

}