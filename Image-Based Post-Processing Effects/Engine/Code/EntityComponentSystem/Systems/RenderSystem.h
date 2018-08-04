#pragma once
#include <vector>
#include ".\..\IOnComponentAddedListener.h"
#include ".\..\IOnComponentRemovedListener.h"
#include ".\..\IOnEntityDestructionListener.h"
#include ".\..\System.h"
#include ".\..\..\Graphics\Scene.h"
#include ".\..\..\Graphics\Effects.h"

class GraphicsFramework;
class Window;
class Camera;
class EntityManager;
template<typename Type>
class Setting;

class RenderSystem : public System<RenderSystem>, IOnComponentAddedListener, IOnComponentRemovedListener, IOnEntityDestructionListener
{
public:
	explicit RenderSystem(std::shared_ptr<Window> _window);
	~RenderSystem();
	void init() override;
	void input(double _currentTime, double _timeDelta) override;
	void update(double _currentTime, double _timeDelta) override;
	void render() override;
	void onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent) override;
	void onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent) override;
	void onDestruction(const Entity *_entity) override;
	std::shared_ptr<Window> getWindow();
	std::shared_ptr<Camera> getActiveCamera();
	unsigned int getFinishedFrameTexture();
	float getExposureMultiplier();
	void setExposureMultiplier(float _multiplier);

private:
	EntityManager &entityManager;
	std::shared_ptr<Window> window;
	GraphicsFramework *graphicsFramework;
	Scene scene;
	Effects effects;
	float exposureMultiplier;
	std::vector<const Entity *> managedEntities;
	std::vector<std::uint64_t> validBitMaps;
	
	// settings
	std::shared_ptr<Setting<int>> shadowQuality;
	std::shared_ptr<Setting<int>> anisotropicFiltering;
	std::shared_ptr<Setting<bool>> bloomEnabled;
	std::shared_ptr<Setting<double>> bloomStrength;
	std::shared_ptr<Setting<bool>> lensDirtEnabled;
	std::shared_ptr<Setting<double>> lensDirtStrength;
	std::shared_ptr<Setting<bool>> chromaticAberrationEnabled;
	std::shared_ptr<Setting<double>> chromaticAberrationOffsetMult;
	std::shared_ptr<Setting<bool>> filmGrainEnabled;
	std::shared_ptr<Setting<double>> filmGrainStrength;
	std::shared_ptr<Setting<bool>> fxaaEnabled;
	std::shared_ptr<Setting<double>> fxaaSubPixelAA;
	std::shared_ptr<Setting<double>> fxaaEdgeThreshold;
	std::shared_ptr<Setting<double>> fxaaEdgeThresholdMin;
	std::shared_ptr<Setting<bool>> smaaEnabled;
	std::shared_ptr<Setting<bool>> smaaTemporalAA;
	std::shared_ptr<Setting<bool>> lensFlaresEnabled;
	std::shared_ptr<Setting<double>> lensFlaresChromaticDistortion;
	std::shared_ptr<Setting<int>> lensFlaresCount;
	std::shared_ptr<Setting<double>> lensFlaresSpacing;
	std::shared_ptr<Setting<double>> lensFlaresHaloWidth;
	std::shared_ptr<Setting<bool>> vignetteEnabled;
	std::shared_ptr<Setting<int>> ambientOcclusion;
	std::shared_ptr<Setting<int>> ssaoKernelSize;
	std::shared_ptr<Setting<double>> ssaoRadius;
	std::shared_ptr<Setting<double>> ssaoBias;
	std::shared_ptr<Setting<double>> ssaoStrength;
	std::shared_ptr<Setting<double>> ssaoBlurSharpness;
	std::shared_ptr<Setting<int>> ssaoBlurRadius;
	std::shared_ptr<Setting<int>> hbaoDirections;
	std::shared_ptr<Setting<int>> hbaoSteps;
	std::shared_ptr<Setting<double>> hbaoStrength;
	std::shared_ptr<Setting<double>> hbaoRadius;
	std::shared_ptr<Setting<double>> hbaoMaxRadiusPixels;
	std::shared_ptr<Setting<double>> hbaoAngleBias;
	std::shared_ptr<Setting<double>> hbaoBlurSharpness;
	std::shared_ptr<Setting<int>> hbaoBlurRadius;
	std::shared_ptr<Setting<int>> gtaoDirections;
	std::shared_ptr<Setting<int>> gtaoSteps;
	std::shared_ptr<Setting<double>> gtaoStrength;
	std::shared_ptr<Setting<double>> gtaoRadius;
	std::shared_ptr<Setting<double>> gtaoMaxRadiusPixels;
	std::shared_ptr<Setting<double>> gtaoBlurSharpness;
	std::shared_ptr<Setting<int>> gtaoBlurRadius;
	std::shared_ptr<Setting<bool>> screenSpaceReflectionsEnabled;
	std::shared_ptr<Setting<bool>> loadEnvironmentFromFile;
	std::shared_ptr<Setting<bool>> saveEnvironmentToFile;
	std::shared_ptr<Setting<int>> motionBlur;
	std::shared_ptr<Setting<int>> depthOfField;

	bool validate(std::uint64_t bitMap);
};