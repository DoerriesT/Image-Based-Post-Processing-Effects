#pragma once
#include <cstdint>
#include <functional>
#include <vector>
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <glm\gtc\quaternion.hpp>
#include <map>
#include <memory>
#include ".\..\Graphics\Model.h"

class Scene;
class ShaderProgram;
class SubMesh;
class Material;
class btCollisionShape;
class btRigidBody;
class btMotionState;
struct Entity;
struct Level;
struct EntityRenderData;
struct RenderData;

// used as a handle to store components in containers

class BaseComponent
{
public:
	virtual ~BaseComponent() = default;
	virtual std::uint64_t getTypeIdOfDerived() = 0;
	virtual std::uint64_t getFamilyIdOfDerived() = 0;

protected:
	static std::uint64_t m_typeCount;
};

// all components should be derived from this class to be able to determine their type

template<typename Type>
class Component : public BaseComponent
{
public:
	typedef Type ComponentType;

	static std::uint64_t getTypeId();
	static std::uint64_t getFamilyId();
	std::uint64_t getTypeIdOfDerived() override { return getTypeId(); }
	std::uint64_t getFamilyIdOfDerived() override { return getFamilyId(); }
};

template<typename Type>
inline std::uint64_t Component<Type>::getTypeId()
{
	static const std::uint64_t ONE = 1ui64;
	static const std::uint64_t type = ONE << m_typeCount++;
	return type;
}

template<typename Type>
inline std::uint64_t Component<Type>::getFamilyId()
{
	return Type::FAMILY_ID;
}

// Component Families

const std::uint64_t BOUNDING_BOX_FAMILY = 1ui64 << 0ui64;
const std::uint64_t MOVEMENT_FAMILY = 1ui64 << 1ui64;
const std::uint64_t MODEL_FAMILY = 1ui64 << 2ui64;
const std::uint64_t OUTLINE_FAMILY = 1ui64 << 3ui64;
const std::uint64_t RENDERABLE_FAMILY = 1ui64 << 4ui64;
const std::uint64_t ROTATION_FAMILY = 1ui64 << 5ui64;
const std::uint64_t SOUND_FAMILY = 1ui64 << 6ui64;
const std::uint64_t TEXTURE_ATLAS_FAMILY = 1ui64 << 7ui64;
const std::uint64_t TRANSFORMATION_FAMILY = 1ui64 << 8ui64;
const std::uint64_t TRANSPARENCY_FAMILY = 1ui64 << 9ui64;
const std::uint64_t TRANSPARENCY_SHADER_FAMILY = 1ui64 << 10ui64;
const std::uint64_t OPAQUE_SHADER_FAMILY = 1ui64 << 11ui64;

const std::uint64_t MAX_FAMILY_ID = OPAQUE_SHADER_FAMILY;


// default components

struct BoundingBoxComponent : public Component<BoundingBoxComponent>
{
	explicit BoundingBoxComponent(const glm::vec3 &_maxCorner, const glm::quat &_rotation = glm::quat())
		:m_maxCorner(_maxCorner), m_rotation(_rotation) { };
	glm::vec3 m_maxCorner;
	glm::quat m_rotation;

	static const std::uint64_t FAMILY_ID;
};

struct CleanupComponent : public Component<CleanupComponent>
{
	explicit CleanupComponent(double _cleanupTime, std::function<void()> _onCleanup)
		:m_cleanupTime(_cleanupTime), m_onCleanup(_onCleanup) {	};
	double m_cleanupTime;
	std::function<void()> m_onCleanup;

	static const std::uint64_t FAMILY_ID;
};

struct CustomTransparencyShaderComponent : public Component<CustomTransparencyShaderComponent>
{
	explicit CustomTransparencyShaderComponent(const std::shared_ptr<ShaderProgram> &_transparencyShader, std::function<void(const RenderData &, const std::shared_ptr<Level> &, const std::unique_ptr<EntityRenderData> &)> _renderTransparency)
		: m_transparencyShader(_transparencyShader), m_renderTransparency(_renderTransparency) { };
	std::shared_ptr<ShaderProgram> m_transparencyShader;
	std::function<void(const RenderData &, const std::shared_ptr<Level> &, const std::unique_ptr<EntityRenderData> &)> m_renderTransparency;

	static const std::uint64_t FAMILY_ID;
};

struct CustomOpaqueShaderComponent : public Component<CustomOpaqueShaderComponent>
{
	explicit CustomOpaqueShaderComponent(const std::shared_ptr<ShaderProgram> &_opaqueShader, std::function<void(const RenderData &, const std::shared_ptr<Level> &, const std::unique_ptr<EntityRenderData> &)> _renderOpaque)
		: m_opaqueShader(_opaqueShader), m_renderOpaque(_renderOpaque) { };
	std::shared_ptr<ShaderProgram> m_opaqueShader;
	std::function<void(const RenderData &, const std::shared_ptr<Level> &, const std::unique_ptr<EntityRenderData> &)> m_renderOpaque;

	static const std::uint64_t FAMILY_ID;
};

struct GrabbedComponent : public Component<GrabbedComponent>
{
	explicit GrabbedComponent(const glm::vec3 &_planeOrigin, const glm::vec3 &_planeNormal)
		:m_planeOrigin(_planeOrigin), m_planeNormal(_planeNormal) { };
	glm::vec3 m_planeOrigin;
	glm::vec3 m_planeNormal;

	static const std::uint64_t FAMILY_ID;
};

struct ModelComponent : public Component<ModelComponent>
{
	explicit ModelComponent(const Model &_model)
		:m_model(_model) { };
	Model m_model;

	static const std::uint64_t FAMILY_ID;
};

struct PathSegment
{
	explicit PathSegment(const glm::vec3 &_startPosition, const glm::vec3 &_endPosition, const glm::vec3 &_startTangent, const glm::vec3 &_endTangent, double _totalDuration, double(*_easingFunction)(double, double), std::function<void()> _onCompleted = []() {})
		: m_startPosition(_startPosition), m_endPosition(_endPosition), m_startTangent(_startTangent), m_endTangent(_endTangent), m_totalDuration(_totalDuration), m_easingFunction(_easingFunction), m_onCompleted(_onCompleted) { };
	glm::vec3 m_startPosition;
	glm::vec3 m_endPosition;
	glm::vec3 m_startTangent;
	glm::vec3 m_endTangent;
	double m_totalDuration;
	double(*m_easingFunction)(double, double);
	std::function<void()> m_onCompleted;
};

struct MovementPathComponent : public Component<MovementPathComponent>
{
	explicit MovementPathComponent(const std::vector<PathSegment> &_pathSegments, double _startTime, bool _repeat)
		:m_pathSegments(_pathSegments), m_startTime(_startTime), m_repeat(_repeat), m_currentStartTime(_startTime), m_currentSegmentIndex(0)
	{
		assert(!_pathSegments.empty());
	};
	std::vector<PathSegment> m_pathSegments;
	double m_startTime;
	bool m_repeat;
	double m_currentStartTime;
	size_t m_currentSegmentIndex;

	static const std::uint64_t FAMILY_ID;
};

struct MovementComponent : public Component<MovementComponent>
{
	explicit MovementComponent(const glm::vec3 &_startPosition, const glm::vec3 &_endPosition, double _startTime, double _totalDuration, double(*_easingFunction)(double, double), std::function<void()> _onCompleted = []() {})
		:m_startPosition(_startPosition), m_endPosition(_endPosition), m_path(_endPosition - _startPosition), m_startTime(_startTime), m_totalDuration(_totalDuration), m_easingFunction(_easingFunction), m_onCompleted(_onCompleted) { };
	glm::vec3 m_startPosition;
	glm::vec3 m_endPosition;
	glm::vec3 m_path;
	double m_startTime;
	double m_totalDuration;
	double(*m_easingFunction)(double, double);
	std::function<void()> m_onCompleted;

	static const std::uint64_t FAMILY_ID;
};

struct OutlineComponent : public Component<OutlineComponent>
{
	explicit OutlineComponent(float _scaleMultiplier, const glm::vec4 &_outlineColor)
		:m_scaleMultiplier(_scaleMultiplier), m_outlineColor(_outlineColor) { };
	float m_scaleMultiplier;
	glm::vec4 m_outlineColor;

	static const std::uint64_t FAMILY_ID;
};

struct PerpetualRotationComponent : public Component<PerpetualRotationComponent>
{
	explicit PerpetualRotationComponent(const glm::vec3 &_rotationIncrement)
		:m_rotationIncrement(_rotationIncrement) { };
	glm::vec3 m_rotationIncrement;

	static const std::uint64_t FAMILY_ID;
};

struct RenderableComponent : public Component<RenderableComponent>
{
	static const std::uint64_t FAMILY_ID;
};

struct RotationComponent : public Component<RotationComponent>
{
	explicit RotationComponent(const glm::quat &_startRotation, const glm::quat &_endRotation, double _startTime, double _totalDuration, double(*_easingFunction)(double, double), std::function<void()> _onCompleted = []() {})
		: m_startRotation(_startRotation), m_endRotation(_endRotation), m_startTime(_startTime), m_totalDuration(_totalDuration), m_easingFunction(_easingFunction), m_onCompleted(_onCompleted) { };
	glm::quat m_startRotation;
	glm::quat m_endRotation;
	double m_startTime;
	double m_totalDuration;
	double(*m_easingFunction)(double, double);
	std::function<void()> m_onCompleted;

	static const std::uint64_t FAMILY_ID;
};

struct TextureAtlasIndexComponent : public Component<TextureAtlasIndexComponent>
{
	explicit TextureAtlasIndexComponent(unsigned int _rows, unsigned int _columns, const std::map<std::shared_ptr<SubMesh>, unsigned int> &_meshToIndexMap)
		:m_rows(_rows), m_columns(_columns), m_meshToIndexMap(_meshToIndexMap) { };
	unsigned int m_rows;
	unsigned int m_columns;
	std::map<std::shared_ptr<SubMesh>, unsigned int> m_meshToIndexMap;

	static const std::uint64_t FAMILY_ID;
};

enum class Mobility
{
	STATIC, DYNAMIC
};

struct TransformationComponent : public Component<TransformationComponent>
{
	explicit TransformationComponent(Mobility _mobility, const glm::vec3 &_position = glm::vec3(), const glm::quat &_rotation = glm::quat(), const glm::vec3 &_scale = glm::vec3(1.0f))
		:m_mobility(_mobility), m_position(_position), m_rotation(_rotation), m_scale(_scale) { };
	Mobility m_mobility;
	glm::vec3 m_position;
	glm::quat m_rotation;
	glm::vec3 m_scale;
	glm::mat4 m_transformation;
	glm::mat4 m_prevTransformation;
	glm::vec2 m_vel;

	static const std::uint64_t FAMILY_ID;
};

struct TransparencyComponent : public Component<TransparencyComponent>
{
	explicit TransparencyComponent(const std::vector<std::shared_ptr<SubMesh>> &_transparentSubMeshes)
		:m_transparentSubMeshes(_transparentSubMeshes) { };
	std::vector<std::shared_ptr<SubMesh>> m_transparentSubMeshes;
	static const std::uint64_t FAMILY_ID;
};