#pragma once

namespace islands {

class Resource : public std::enable_shared_from_this<Resource> {
	friend class ResourceSystem;

public:
	Resource();
	Resource(const std::string& name);

	virtual ~Resource() = default;

	const std::string& getName() const;
	virtual void loadImpl() {}
	virtual void uploadImpl() {}

	void load();
	void loadAsync();
	void upload();
	virtual bool isLoaded() const;
	bool isUploaded() const;

private:
	enum class State {
		Unloaded,
		Loading,
		Loaded,
		Uploaded
	};

	const std::string name_;
	std::atomic<State> status_;

	State getStatus() const;
	void setStatus(State status);
};

}