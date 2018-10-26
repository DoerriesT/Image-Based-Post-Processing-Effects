#pragma once
#include <al.h>
#include <string>
#include <memory>
#include <map>
#include "stb_vorbis.h"
#include ".\..\..\JobManager.h"

class SoundBuffer
{
public:
	static std::shared_ptr<SoundBuffer> createSoundBuffer(const std::string &_file, bool _instantLoading = false);

	SoundBuffer(const SoundBuffer &) = delete;
	SoundBuffer(const SoundBuffer &&) = delete;
	SoundBuffer &operator= (const SoundBuffer &) = delete;
	SoundBuffer &operator= (const SoundBuffer &&) = delete;
	~SoundBuffer();
	ALuint getBufferId() const;
	bool isValid() const;

private:
	static std::map<std::string, std::weak_ptr<SoundBuffer>> m_soundMap;
	std::string m_filepath;
	bool m_valid;
	JobManager::SharedJob m_dataJob;
	ALuint m_bufferId;

	explicit SoundBuffer(const std::string &_file, bool _instantLoading = false);
};