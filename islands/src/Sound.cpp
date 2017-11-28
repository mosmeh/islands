#pragma warning(push)
#pragma warning(disable: 4100)
#pragma warning(disable: 4244)
#pragma warning(disable: 4245)
#pragma warning(disable: 4456)
#pragma warning(disable: 4457)
#pragma warning(disable: 4701)
#include <stb_vorbis.c>
#pragma warning(pop)

#include "Sound.h"
#include "AssetArchive.h"
#include "Log.h"

namespace islands {

#define CHECK_PA(x) { \
	PaError err = x; \
	if (err != paNoError) \
		SLOG << "PortAudio: " << Pa_GetErrorText(err) << std::endl; \
}

Sound::Sound(const std::string& filename) :	SharedResource(filename) {}

Sound::~Sound() {
	if (isLoaded()) {
		instances_.clear();
		if (buffer_) {
			std::free(buffer_);
		}
	}
}

std::shared_ptr<Sound::Instance> Sound::createInstance() {
	load();

	instances_.erase(std::remove_if(instances_.begin(), instances_.end(), [](std::shared_ptr<Instance> inst) {
		return !inst->isPlaying() && inst.use_count() == 1;
	}), instances_.end());

	const auto instance = std::make_shared<Sound::Instance>(*this);
	instances_.emplace_back(instance);
	return instance;
}

void Sound::loadImpl() {
	static const std::string SOUND_DIR = "sound";

#ifdef ENABLE_ASSET_ARCHIVE
	const auto filePath = SOUND_DIR + '/' + getName();
	auto rawData = AssetArchive::getInstance().readFile(filePath);
	length_ = stb_vorbis_decode_memory(reinterpret_cast<uint8*>(rawData.data()), rawData.size(),
		&numChannels_, &sampleRate_, &buffer_);
#else
	const auto filePath = SOUND_DIR + sys::getFilePathSeparator() + getName();
	length_ = stb_vorbis_decode_filename(filePath.c_str(), &numChannels_, &sampleRate_, &buffer_);
#endif
	assert(length_ > 0);
}

Sound::Instance::Instance(const Sound& sound) :
	sound_(sound),
	playing_(false),
	stream_(nullptr) {

	CHECK_PA(Pa_OpenDefaultStream(&stream_, 0, sound_.numChannels_,
		paInt16, sound_.sampleRate_, FRAMES_PER_BUFFER, nullptr, nullptr));
}

Sound::Instance::~Instance() {
	stop();
	CHECK_PA(Pa_CloseStream(stream_));
}

void Sound::Instance::play(bool loop) {
	stop();
	playing_ = true;
	thread_ = std::thread([this, loop] {
		CHECK_PA(Pa_StartStream(stream_));

		const auto size = static_cast<size_t>(sound_.numChannels_ * sound_.length_);
		const auto stride = sound_.numChannels_ * FRAMES_PER_BUFFER;
		do {
			for (size_t i = 0; playing_ && i < size; i += stride) {
				CHECK_PA(Pa_WriteStream(stream_, &sound_.buffer_[i], FRAMES_PER_BUFFER));
			}
		} while (loop && playing_);
		playing_ = false;

		CHECK_PA(Pa_StopStream(stream_));
	});
}

void Sound::Instance::stop() {
	playing_ = false;
	if (thread_.joinable()) {
		thread_.join();
	}
}

bool Sound::Instance::isPlaying() const {
	return playing_;
}


}
