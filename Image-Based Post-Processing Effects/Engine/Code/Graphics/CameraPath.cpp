#include "CameraPath.h"
#include <glm\gtx\spline.hpp>
#include ".\..\Engine.h"
#include ".\..\Utilities\Utility.h"
#include ".\..\EntityComponentSystem\SystemManager.h"
#include ".\..\EntityComponentSystem\Systems\RenderSystem.h"
#include "Camera.h"

CameraPath::CameraPath(const std::vector<CameraPathSegment> &_pathSegments)
	:m_pathSegments(_pathSegments),
	m_started(false),
	m_repeat(false),
	m_currentStartTime(),
	m_currentSegmentIndex()
{
}

void CameraPath::start(std::shared_ptr<Camera> _camera, bool _repeat)
{
	m_camera = _camera;
	m_repeat = _repeat;
	m_started = true;
	m_currentStartTime = Engine::getTime();
	m_currentSegmentIndex = 0;
}

void CameraPath::stop()
{
	m_started = false;
}

bool CameraPath::update(double _currentTime, double _timeDelta)
{
	if (!m_started)
	{
		return false;
	}
	
	assert(m_camera);

	CameraPathSegment &segment = m_pathSegments[m_currentSegmentIndex];
	float factor = static_cast<float>(segment.m_easingFunction(_currentTime - m_currentStartTime, segment.m_totalDuration));
	if (factor >= 1.0f)
	{
		// set position to segment end position
		m_camera->setPosition(segment.m_cameraEndPosition);
		m_camera->lookAt(segment.m_targetEndPosition);
		// update start time for next segment
		m_currentStartTime += segment.m_totalDuration;
		// increment segment index
		++m_currentSegmentIndex;

		// if we reached the last segment either wrap around or halt the movement
		size_t totalSegments = m_pathSegments.size();
		if (m_currentSegmentIndex >= totalSegments)
		{
			if (m_repeat)
			{
				m_currentSegmentIndex %= totalSegments;
				return true;
			}
			else
			{
				m_started = false;
				return false;
			}
		}
		return true;
	}
	else
	{
		float elapsedTime = float(_currentTime - m_currentStartTime);
		float fadeFactor = 1.0;
		if (segment.m_fadeIn && elapsedTime <= segment.m_fadeTime)
		{
			fadeFactor = glm::smoothstep(0.0f, 1.0f, elapsedTime / (float)segment.m_fadeTime);
		}
		if (segment.m_fadeOut && elapsedTime >= segment.m_totalDuration - segment.m_fadeTime)
		{
			fadeFactor = glm::smoothstep(1.0f, 0.0f, (elapsedTime - float(segment.m_totalDuration - segment.m_fadeTime)) / (float)segment.m_fadeTime);
		}

		static RenderSystem *rs = SystemManager::getInstance().getSystem<RenderSystem>();
		rs->setExposureMultiplier(fadeFactor);

		m_camera->setPosition(glm::hermite(segment.m_cameraStartPosition, segment.m_cameraStartTangent, segment.m_cameraEndPosition, segment.m_cameraEndTangent, factor));
		m_camera->lookAt(glm::hermite(segment.m_targetStartPosition, segment.m_targetStartTangent, segment.m_targetEndPosition, segment.m_targetEndTangent, factor));
		return true;
	}
}

std::vector<CameraPathSegment>& CameraPath::getPathSegments()
{
	return m_pathSegments;
}

bool CameraPath::isRepeating()
{
	return m_repeat;
}
