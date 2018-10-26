#pragma once
#include <vector>
#include <memory>
#include <glm\vec3.hpp>

class Camera;

struct CameraPathSegment
{
	glm::vec3 m_cameraStartPosition;
	glm::vec3 m_cameraEndPosition;
	glm::vec3 m_cameraStartTangent;
	glm::vec3 m_cameraEndTangent;
	glm::vec3 m_targetStartPosition;
	glm::vec3 m_targetEndPosition;
	glm::vec3 m_targetStartTangent;
	glm::vec3 m_targetEndTangent;
	double m_totalDuration = 1.0;
	double(*m_easingFunction)(double, double) = nullptr;
	bool m_fadeIn = false;
	bool m_fadeOut = false;
	double m_fadeTime = 2.0;
};

class CameraPath
{
public:
	explicit CameraPath(const std::vector<CameraPathSegment> &_pathSegments = std::vector<CameraPathSegment>());
	void start(std::shared_ptr<Camera> _camera, bool _repeat);
	void stop();
	bool update(double _currentTime, double _timeDelta);
	std::vector<CameraPathSegment> &getPathSegments();
	bool isRepeating();

private:
	std::vector<CameraPathSegment> m_pathSegments;
	std::shared_ptr<Camera> m_camera;
	bool m_started;
	bool m_repeat;
	double m_currentStartTime;
	size_t m_currentSegmentIndex;
};