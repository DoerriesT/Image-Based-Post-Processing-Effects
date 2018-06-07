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
	static std::uint64_t typeCount;
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
	static const std::uint64_t type = ONE << typeCount++;
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
		:maxCorner(_maxCorner), rotation(_rotation) { };
	glm::vec3 maxCorner;
	glm::quat rotation;

	static const std::uint64_t FAMILY_ID;
};

struct CleanupComponent : public Component<CleanupComponent>
{
	explicit CleanupComponent(double _cleanupTime, std::function<void()> _onCleanup)
		:cleanupTime(_cleanupTime), onCleanup(_onCleanup) {	};
	double cleanupTime;
	std::function<void()> onCleanup;

	static const std::uint64_t FAMILY_ID;
};

struct CustomTransparencyShaderComponent : public Component<CustomTransparencyShaderComponent>
{
	explicit CustomTransparencyShaderComponent(const std::shared_ptr<ShaderProgram> &_transparencyShader, std::function<void(const RenderData &, const std::shared_ptr<Level> &, const std::unique_ptr<EntityRenderData> &)> _renderTransparency)
		: transparencyShader(_transparencyShader), renderTransparency(_renderTransparency) { };
	std::shared_ptr<ShaderProgram> transparencyShader;
	std::function<void(const RenderData &, const std::shared_ptr<Level> &, const std::unique_ptr<EntityRenderData> &)> renderTransparency;

	static const std::uint64_t FAMILY_ID;
};

struct CustomOpaqueShaderComponent : public Component<CustomOpaqueShaderComponent>
{
	explicit CustomOpaqueShaderComponent(const std::shared_ptr<ShaderProgram> &_opaqueShader, std::function<void(const RenderData &, const std::shared_ptr<Level> &, const std::unique_ptr<EntityRenderData> &)> _renderOpaque)
		: opaqueShader(_opaqueShader), renderOpaque(_renderOpaque) { };
	std::shared_ptr<ShaderProgram> opaqueShader;
	std::function<void(const RenderData &, const std::shared_ptr<Level> &, const std::unique_ptr<EntityRenderData> &)> renderOpaque;

	static const std::uint64_t FAMILY_ID;
};

struct GrabbedComponent : public Component<GrabbedComponent>
{
	explicit GrabbedComponent(const glm::vec3 &_planeOrigin, const glm::vec3 &_planeNormal)
		:planeOrigin(_planeOrigin), planeNormal(_planeNormal) { };
	glm::vec3 planeOrigin;
	glm::vec3 planeNormal;

	static const std::uint64_t FAMILY_ID;
};

struct ModelComponent : public Component<ModelComponent>
{
	explicit ModelComponent(const Model &_model)
		:model(_model) { };
	Model model;

	static const std::uint64_t FAMILY_ID;
};

struct PathSegment
{
	explicit PathSegment(const glm::vec3 &_startPosition, const glm::vec3 &_endPosition, const glm::vec3 &_startTangent, const glm::vec3 &_endTangent, double _totalDuration, double(*_easingFunction)(double, double), std::function<void()> _onCompleted = []() {})
		: startPosition(_startPosition), endPosition(_endPosition), startTangent(_startTangent), endTangent(_endTangent), totalDuration(_totalDuration), easingFunction(_easingFunction), onCompleted(_onCompleted) { };
	glm::vec3 startPosition;
	glm::vec3 endPosition;
	glm::vec3 startTangent;
	glm::vec3 endTangent;
	double totalDuration;
	double(*easingFunction)(double, double);
	std::function<void()> onCompleted;
};

struct MovementPathComponent : public Component<MovementPathComponent>
{
	explicit MovementPathComponent(const std::vector<PathSegment> &_pathSegments, double _startTime, bool _repeat)
		:pathSegments(_pathSegments), startTime(_startTime), repeat(_repeat), currentStartTime(_startTime), currentSegmentIndex(0)
	{
		assert(!_pathSegments.empty());
	};
	std::vector<PathSegment> pathSegments;
	double startTime;
	bool repeat;
	double currentStartTime;
	size_t currentSegmentIndex;

	static const std::uint64_t FAMILY_ID;
};

struct MovementComponent : public Component<MovementComponent>
{
	explicit MovementComponent(const glm::vec3 &_startPosition, const glm::vec3 &_endPosition, double _startTime, double _totalDuration, double(*_easingFunction)(double, double), std::function<void()> _onCompleted = []() {})
		:startPosition(_startPosition), endPosition(_endPosition), path(_endPosition - _startPosition), startTime(_startTime), totalDuration(_totalDuration), easingFunction(_easingFunction), onCompleted(_onCompleted) { };
	glm::vec3 startPosition;
	glm::vec3 endPosition;
	glm::vec3 path;
	double startTime;
	double totalDuration;
	double(*easingFunction)(double, double);
	std::function<void()> onCompleted;

	static const std::uint64_t FAMILY_ID;
};

struct OutlineComponent : public Component<OutlineComponent>
{
	explicit OutlineComponent(float _scaleMultiplier, const glm::vec4 &_outlineColor)
		:scaleMultiplier(_scaleMultiplier), outlineColor(_outlineColor) { };
	float scaleMultiplier;
	glm::vec4 outlineColor;

	static const std::uint64_t FAMILY_ID;
};

struct PerpetualRotationComponent : public Component<PerpetualRotationComponent>
{
	explicit PerpetualRotationComponent(const glm::vec3 &_rotationIncrement)
		:rotationIncrement(_rotationIncrement) { };
	glm::vec3 rotationIncrement;

	static const std::uint64_t FAMILY_ID;
};

struct RenderableComponent : public Component<RenderableComponent>
{
	static const std::uint64_t FAMILY_ID;
};

struct RotationComponent : public Component<RotationComponent>
{
	explicit RotationComponent(const glm::quat &_startRotation, const glm::quat &_endRotation, double _startTime, double _totalDuration, double(*_easingFunction)(double, double), std::function<void()> _onCompleted = []() {})
		: startRotation(_startRotation), endRotation(_endRotation), startTime(_startTime), totalDuration(_totalDuration), easingFunction(_easingFunction), onCompleted(_onCompleted) { };
	glm::quat startRotation;
	glm::quat endRotation;
	double startTime;
	double totalDuration;
	double(*easingFunction)(double, double);
	std::function<void()> onCompleted;

	static const std::uint64_t FAMILY_ID;
};

enum class SoundType
{
	MUSIC, EFFECT, UI
};

struct SoundComponent : public Component<SoundComponent>
{
	explicit SoundComponent(const std::string &_soundFile, const SoundType &_soundType, float _volume = 1.0f, bool _looping = false, bool _paused = true, bool _loadInstantly = false)
		: soundFile(_soundFile), soundType(_soundType), volume(_volume), looping(_looping), paused(_paused), loadInstantly(_loadInstantly) { };
	std::string soundFile;
	SoundType soundType;
	float volume;
	bool looping;
	bool paused;
	bool loadInstantly;

	static const std::uint64_t FAMILY_ID;
};

struct TextureAtlasIndexComponent : public Component<TextureAtlasIndexComponent>
{
	explicit TextureAtlasIndexComponent(unsigned int _rows, unsigned int _columns, const std::map<std::shared_ptr<SubMesh>, unsigned int> &_meshToIndexMap)
		:rows(_rows), columns(_columns), meshToIndexMap(_meshToIndexMap) { };
	unsigned int rows;
	unsigned int columns;
	std::map<std::shared_ptr<SubMesh>, unsigned int> meshToIndexMap;

	static const std::uint64_t FAMILY_ID;
};

struct TransformationComponent : public Component<TransformationComponent>
{
	explicit TransformationComponent(const glm::vec3 &_position = glm::vec3(), const glm::quat &_rotation = glm::quat(), const glm::vec3 &_scale = glm::vec3(1.0f))
		:position(_position), rotation(_rotation), scale(_scale) { };
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
	glm::mat4 prevTransformation;
	glm::vec2 vel;

	static const std::uint64_t FAMILY_ID;
};

struct TransparencyComponent : public Component<TransparencyComponent>
{
	explicit TransparencyComponent(const std::vector<std::shared_ptr<SubMesh>> &_transparentSubMeshes)
		:transparentSubMeshes(_transparentSubMeshes) { };
	std::vector<std::shared_ptr<SubMesh>> transparentSubMeshes;
	static const std::uint64_t FAMILY_ID;
};