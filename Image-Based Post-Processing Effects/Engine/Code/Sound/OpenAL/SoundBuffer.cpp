#include "SoundBuffer.h"
#include <iostream>
#include "Utilities\ContainerUtility.h"
#include "Utilities\Utility.h"
#include <cassert>

std::map<std::string, std::weak_ptr<SoundBuffer>> SoundBuffer::m_soundMap;

std::shared_ptr<SoundBuffer> SoundBuffer::createSoundBuffer(const std::string &_file, bool _instantLoading)
{
	if (ContainerUtility::contains(m_soundMap, _file))
	{
		return std::shared_ptr<SoundBuffer>(m_soundMap[_file]);
	}
	else
	{
		std::shared_ptr<SoundBuffer> soundBuffer = std::shared_ptr<SoundBuffer>(new SoundBuffer(_file, _instantLoading));
		m_soundMap[_file] = soundBuffer;
		return soundBuffer;
	}
}

SoundBuffer::~SoundBuffer()
{
	if (m_dataJob)
	{
		m_dataJob->kill();
	}
	ContainerUtility::remove(m_soundMap, m_filepath);
	if (m_valid)
	{
		alDeleteBuffers(1, &m_bufferId);
	}
}

ALuint SoundBuffer::getBufferId() const
{
	assert(m_valid);
	return m_bufferId;
}

bool SoundBuffer::isValid() const
{
	return m_valid;
}

SoundBuffer::SoundBuffer(const std::string &_file, bool _instantLoading)
	:m_filepath(_file), m_valid(false), m_dataJob(nullptr)
{
	auto dataPreparation = [=](JobManager::SharedJob job)
	{
		stb_vorbis_info info;

		std::vector<char> vorbisData = Utility::readBinaryFile(_file);
		int error = 0;
		stb_vorbis *decoder = stb_vorbis_open_memory((unsigned char *)vorbisData.data(), (int)vorbisData.size(), &error, nullptr);
		if (!decoder)
		{
			std::cerr << "Failed to open Ogg Vorbis file.Error: " << error << std::endl;
		}
		info = stb_vorbis_get_info(decoder);

		int channels = info.channels;
		unsigned int lengthSamples = stb_vorbis_stream_length_in_samples(decoder);

		short *data = new short[static_cast<size_t>(lengthSamples)];
		stb_vorbis_get_samples_short_interleaved(decoder, channels, data, lengthSamples);
		stb_vorbis_close(decoder);

		auto *result = new std::tuple<short *, unsigned int, unsigned int, int>(data, static_cast<size_t>(lengthSamples) * sizeof(short), info.sample_rate, info.channels);
		job->setUserData(result);
	};

	ALuint &bId = m_bufferId;

	auto dataInitialization = [&](JobManager::SharedJob job)
	{
		auto *p = (std::tuple<short *, unsigned int, unsigned int, int> *)job->getUserData();

		alGenBuffers(1, &bId);

		// Copy to buffer
		alBufferData(bId, std::get<3>(*p) == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, std::get<0>(*p), std::get<1>(*p), std::get<2>(*p));



		m_valid = true;
		m_dataJob.reset();

		job->markDone(true);
	};

	auto dataCleanup = [](JobManager::SharedJob job)
	{
		auto *p = (std::tuple<short *, unsigned int, unsigned int, int> *)job->getUserData();
			delete[] std::get<0>(*p);
		delete p;
	};

	if (_instantLoading)
	{
		JobManager::getInstance().run(dataPreparation, dataInitialization, dataCleanup);
	}
	else
	{
		m_dataJob = JobManager::getInstance().queue(dataPreparation, dataInitialization, dataCleanup);
	}

}
