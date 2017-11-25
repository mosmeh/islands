#pragma once

#include "Resource.h"

namespace islands {

class Sound : public Resource<Sound> {
public:
	class Instance {
	public:
		Instance(const Sound& sound);
		virtual ~Instance();

		void play(bool loop = false);
		void stop();
		bool isPlaying() const;

	private:
		static constexpr auto FRAMES_PER_BUFFER = 512;

		const Sound& sound_;
		PaStream* stream_;
		bool playing_;
		std::thread thread_;
	};

	Sound(const std::string& name, const std::string& filename);
	virtual ~Sound();

	std::shared_ptr<Instance> createInstance();

private:
	const std::string filename_;
	short* buffer_;
	int numChannels_, sampleRate_, length_;
	std::vector<std::shared_ptr<Instance>> instances_;

	void loadImpl();
};


}
