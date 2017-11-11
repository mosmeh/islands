#pragma once

#define ENABLE_ASSET_ARCHIVE

#ifdef ENABLE_ASSET_ARCHIVE

namespace islands {

class AssetArchive {
public:
	AssetArchive(const AssetArchive&) = delete;
	AssetArchive& operator=(const AssetArchive&) = delete;
	virtual ~AssetArchive();

	static AssetArchive& getInstance();

	std::vector<char> readFile(const std::string& filename) const;
	std::string readTextFile(const std::string& filename) const;

private:
	zip_t* zip_;

	AssetArchive();
};

}

#endif
