#include "AssetArchive.h"

#ifdef ENABLE_ASSET_ARCHIVE

namespace islands {

//static constexpr auto ASSET_ARCHIVE_PASSWORD = "password";
#include "AssetArchivePassword.inc"

AssetArchive::AssetArchive() {
	static constexpr auto ASSETS_FILENAME = "assets";

	int error;
	zip_ = zip_open(ASSETS_FILENAME, ZIP_RDONLY, &error);
	assert(zip_);
}

AssetArchive::~AssetArchive() {
	zip_close(zip_);
}

AssetArchive& AssetArchive::getInstance() {
	static AssetArchive instance;
	return instance;
}

std::vector<char> AssetArchive::readFile(const std::string& filename) const {
	struct zip_stat stat = {};
	const auto ret = zip_stat(zip_, filename.c_str(), 0, &stat);
	assert(ret == 0);
	const auto file = zip_fopen_encrypted(zip_, filename.c_str(), 0, ASSET_ARCHIVE_PASSWORD);
	if (!file) {
		return {};
	}

	std::vector<char> buf(static_cast<size_t>(stat.size));
	const auto size = zip_fread(file, buf.data(), stat.size);
	assert(size == static_cast<zip_int64_t>(stat.size));
	zip_fclose(file);

	return buf;
}

std::string AssetArchive::readTextFile(const std::string& filename) const {
	auto text = readFile(filename);
	text.emplace_back('\0');
	return {text.data()};
}

}

#endif
