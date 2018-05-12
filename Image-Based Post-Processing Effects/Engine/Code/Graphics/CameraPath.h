#pragma once
#include <vector>
#include <memory>
#include <glm\vec3.hpp>

class Camera;

struct CameraPathSegment
{
	glm::vec3 cameraStartPosition;
	glm::vec3 cameraEndPosition;
	glm::vec3 cameraStartTangent;
	glm::vec3 cameraEndTangent;
	glm::vec3 targetStartPosition;
	glm::vec3 targetEndPosition;
	glm::vec3 targetStartTangent;
	glm::vec3 targetEndTangent;
	double totalDuration;
	double(*easingFunction)(double, const double &);
	bool fadeIn = false;
	bool fadeOut = false;
	double fadeTime = 2.0;
};

class CameraPath
{
public:
	explicit CameraPath(const std::vector<CameraPathSegment> &_pathSegments = std::vector<CameraPathSegment>());
	void start(std::shared_ptr<Camera> _camera, bool _repeat);
	void stop();
	bool update(const double &_currentTime, const double &_timeDelta);
	std::vector<CameraPathSegment> &getPathSegments();
	bool isRepeating();

private:
	std::vector<CameraPathSegment> pathSegments;
	std::shared_ptr<Camera> camera;
	bool started;
	bool repeat;
	double currentStartTime;
	size_t currentSegmentIndex;
};