#include "CameraPath.h"
#include <glm\gtx\spline.hpp>
#include ".\..\Engine.h"
#include ".\..\Utilities\Utility.h"
#include ".\..\EntityComponentSystem\SystemManager.h"
#include ".\..\EntityComponentSystem\Systems\RenderSystem.h"
#include "Camera.h"

CameraPath::CameraPath(const std::vector<CameraPathSegment> &_pathSegments)
	:pathSegments(_pathSegments),
	started(false),
	repeat(false),
	currentStartTime(),
	currentSegmentIndex()
{
}

void CameraPath::start(std::shared_ptr<Camera> _camera, bool _repeat)
{
	camera = _camera;
	repeat = _repeat;
	started = true;
	currentStartTime = Engine::getTime();
	currentSegmentIndex = 0;
}

void CameraPath::stop()
{
	started = false;
}

bool CameraPath::update(double _currentTime, double _timeDelta)
{
	if (!started)
	{
		return false;
	}
	
	assert(camera);

	CameraPathSegment &segment = pathSegments[currentSegmentIndex];
	float factor = static_cast<float>(segment.easingFunction(_currentTime - currentStartTime, segment.totalDuration));
	if (factor >= 1.0f)
	{
		// set position to segment end position
		camera->setPosition(segment.cameraEndPosition);
		camera->lookAt(segment.targetEndPosition);
		// update start time for next segment
		currentStartTime += segment.totalDuration;
		// increment segment index
		++currentSegmentIndex;

		// if we reached the last segment either wrap around or halt the movement
		size_t totalSegments = pathSegments.size();
		if (currentSegmentIndex >= totalSegments)
		{
			if (repeat)
			{
				currentSegmentIndex %= totalSegments;
				return true;
			}
			else
			{
				started = false;
				return false;
			}
		}
		return true;
	}
	else
	{
		float elapsedTime = float(_currentTime - currentStartTime);
		float fadeFactor = 1.0;
		if (segment.fadeIn && elapsedTime <= segment.fadeTime)
		{
			fadeFactor = glm::smoothstep(0.0f, 1.0f, elapsedTime / (float)segment.fadeTime);
		}
		if (segment.fadeOut && elapsedTime >= segment.totalDuration - segment.fadeTime)
		{
			fadeFactor = glm::smoothstep(1.0f, 0.0f, (elapsedTime - float(segment.totalDuration - segment.fadeTime)) / (float)segment.fadeTime);
		}

		static RenderSystem *rs = SystemManager::getInstance().getSystem<RenderSystem>();
		rs->setExposureMultiplier(fadeFactor);

		camera->setPosition(glm::hermite(segment.cameraStartPosition, segment.cameraStartTangent, segment.cameraEndPosition, segment.cameraEndTangent, factor));
		camera->lookAt(glm::hermite(segment.targetStartPosition, segment.targetStartTangent, segment.targetEndPosition, segment.targetEndTangent, factor));
		return true;
	}
}

std::vector<CameraPathSegment>& CameraPath::getPathSegments()
{
	return pathSegments;
}

bool CameraPath::isRepeating()
{
	return repeat;
}
