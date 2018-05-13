#include <algorithm>
#include <cassert>
#include "RenderSystem.h"
#include ".\..\Component.h"
#include ".\..\..\Utilities\Utility.h"
#include ".\..\..\Engine.h"
#include ".\..\..\Gui\Gui.h"
#include ".\..\..\Framework\Graphics\GraphicsFramework.h"
#include ".\..\EntityManager.h"
#include "Window.h"
#include ".\..\..\Graphics\Camera.h"
#include ".\..\..\Graphics\Texture.h"
#include "Level.h"
#include ".\..\SystemManager.h"
#include ".\..\..\Graphics\EntityRenderData.h"
#include ".\..\..\Settings.h"

RenderSystem::RenderSystem(std::shared_ptr<Window> _window)
	:graphicsFramework(new GraphicsFramework(_window)), 
	window(_window), 
	exposureMultiplier(1.0f),
	entityManager(EntityManager::getInstance())
{
	validBitMaps.push_back(Component<TransformationComponent>::getTypeId() | Component<ModelComponent>::getTypeId() | Component<RenderableComponent>::getTypeId());
}

RenderSystem::~RenderSystem()
{
	delete graphicsFramework;
}

void RenderSystem::init()
{
	graphicsFramework->init();

	entityManager.addOnComponentAddedListener(this);
	entityManager.addOnComponentRemovedListener(this);
	entityManager.addOnEntityDestructionListener(this);

	SettingsManager &settingsManager = SettingsManager::getInstance();
	
	shadowQuality = settingsManager.getIntSetting("graphics", "shadow_quality", 0);
	shadowQuality->addListener([&](int _value) { effects.shadowQuality = (ShadowQuality)_value; });
	effects.shadowQuality = (ShadowQuality)shadowQuality->get();

	anisotropicFiltering = settingsManager.getIntSetting("graphics", "anisotropic_filtering", 1);
	anisotropicFiltering->addListener([&](int _value) { Texture::setAnisotropicFilteringAll((float)_value); });
	Texture::setAnisotropicFilteringAll((float)anisotropicFiltering->get());

	bloomEnabled = settingsManager.getBoolSetting("graphics", "bloom_enabled", false);
	bloomEnabled->addListener([&](bool _value) { effects.bloom.enabled = _value; });
	effects.bloom.enabled = bloomEnabled->get();

	bloomStrength = settingsManager.getDoubleSetting("graphics", "bloom_strength", 0.01);
	bloomStrength->addListener([&](double _value) { effects.bloom.strength = (float)_value; });
	effects.bloom.strength = (float)bloomStrength->get();

	bloomLensDirtStrength = settingsManager.getDoubleSetting("graphics", "bloom_lens_dirt_strength", 1.0);
	bloomLensDirtStrength->addListener([&](double _value) { effects.bloom.lensDirtStrength = (float)_value; });
	effects.bloom.lensDirtStrength = (float)bloomLensDirtStrength->get();

	chromaticAberrationEnabled = settingsManager.getBoolSetting("graphics", "chromatic_aberration_enabled", false);
	chromaticAberrationEnabled->addListener([&](bool _value) { effects.chromaticAberration.enabled = _value; });
	effects.chromaticAberration.enabled = chromaticAberrationEnabled->get();

	chromaticAberrationOffsetMult = settingsManager.getDoubleSetting("graphics", "chromatic_aberration_offset_mult", 0.005);
	chromaticAberrationOffsetMult->addListener([&](double _value) { effects.chromaticAberration.offsetMultiplier = (float)_value; });
	effects.chromaticAberration.offsetMultiplier = (float)chromaticAberrationOffsetMult->get();

	filmGrainEnabled = settingsManager.getBoolSetting("graphics", "film_grain_enabled", false);
	filmGrainEnabled->addListener([&](bool _value) { effects.filmGrain.enabled = _value; });
	effects.filmGrain.enabled = filmGrainEnabled->get();

	filmGrainStrength = settingsManager.getDoubleSetting("graphics", "film_grain_strength", 0.055);
	filmGrainStrength->addListener([&](double _value) { effects.filmGrain.strength = (float)_value; });
	effects.filmGrain.strength = (float)filmGrainStrength->get();

	fxaaEnabled = settingsManager.getBoolSetting("graphics", "fxaa_enabled", false);
	fxaaEnabled->addListener([&](bool _value) { effects.fxaa.enabled = _value; });
	effects.fxaa.enabled = fxaaEnabled->get();

	fxaaSubPixelAA = settingsManager.getDoubleSetting("graphics", "fxaa_subpixel_aa", 0.75);
	fxaaSubPixelAA->addListener([&](double _value) { effects.fxaa.subPixelAA = (float)_value; });
	effects.fxaa.subPixelAA = (float)fxaaSubPixelAA->get();

	fxaaEdgeThreshold = settingsManager.getDoubleSetting("graphics", "fxaa_edge_theshold", 0.166);
	fxaaEdgeThreshold->addListener([&](double _value) { effects.fxaa.edgeThreshold = (float)_value; });
	effects.fxaa.edgeThreshold = (float)fxaaEdgeThreshold->get();

	fxaaEdgeThresholdMin = settingsManager.getDoubleSetting("graphics", "fxaa_edge_theshold_min", 0.0833);
	fxaaEdgeThresholdMin->addListener([&](double _value) { effects.fxaa.edgeThresholdMin = (float)_value; });
	effects.fxaa.edgeThresholdMin = (float)fxaaEdgeThresholdMin->get();

	lensFlaresEnabled = settingsManager.getBoolSetting("graphics", "lens_flares_enabled", false);
	lensFlaresEnabled->addListener([&](bool _value) { effects.lensFlares.enabled = _value; });
	effects.lensFlares.enabled = lensFlaresEnabled->get();

	lensFlaresChromaticDistortion = settingsManager.getDoubleSetting("graphics", "lens_flares_chromatic_distortion", 1.5);
	lensFlaresChromaticDistortion->addListener([&](double _value) { effects.lensFlares.chromaticDistortion = (float)_value; });
	effects.lensFlares.chromaticDistortion = (float)lensFlaresChromaticDistortion->get();

	lensFlaresCount = settingsManager.getIntSetting("graphics", "lens_flares_count", 4);
	lensFlaresCount->addListener([&](int _value) { effects.lensFlares.flareCount = _value; });
	effects.lensFlares.flareCount = lensFlaresCount->get();

	lensFlaresSpacing = settingsManager.getDoubleSetting("graphics", "lens_flares_spacing", 0.33);
	lensFlaresSpacing->addListener([&](double _value) { effects.lensFlares.flareSpacing = (float)_value; });
	effects.lensFlares.flareSpacing = (float)lensFlaresSpacing->get();

	lensFlaresHaloWidth = settingsManager.getDoubleSetting("graphics", "lens_flares_halo_width", 0.33);
	lensFlaresHaloWidth->addListener([&](double _value) { effects.lensFlares.haloWidth = (float)_value; });
	effects.lensFlares.haloWidth = (float)lensFlaresHaloWidth->get();

	vignetteEnabled = settingsManager.getBoolSetting("graphics", "vignette_enabled", false);
	vignetteEnabled->addListener([&](bool _value) { effects.vignette.enabled = _value; });
	effects.vignette.enabled = vignetteEnabled->get();

	ssaoEnabled = settingsManager.getBoolSetting("graphics", "ssao_enabled", false);
	ssaoEnabled->addListener([&](bool _value) { effects.ssao.enabled = _value; });
	effects.ssao.enabled = ssaoEnabled->get();

	ssaoKernelSize = settingsManager.getIntSetting("graphics", "ssao_kernel_size", 16);
	ssaoKernelSize->addListener([&](int _value) { effects.ssao.kernelSize = _value; });
	effects.ssao.kernelSize = ssaoKernelSize->get();

	ssaoRadius = settingsManager.getDoubleSetting("graphics", "ssao_radius", 0.5);
	ssaoRadius->addListener([&](double _value) { effects.ssao.radius = (float)_value; });
	effects.ssao.radius = (float)ssaoRadius->get();

	ssaoBias = settingsManager.getDoubleSetting("graphics", "ssao_bias", 0.025);
	ssaoBias->addListener([&](double _value) { effects.ssao.bias = (float)_value; });
	effects.ssao.bias = (float)ssaoBias->get();

	screenSpaceReflectionsEnabled = settingsManager.getBoolSetting("graphics", "screen_space_reflections_enabled", false);
	screenSpaceReflectionsEnabled->addListener([&](bool _value) { effects.screenSpaceReflections.enabled = _value; });
	effects.screenSpaceReflections.enabled = screenSpaceReflectionsEnabled->get();

	loadEnvironmentFromFile = settingsManager.getBoolSetting("graphics", "load_environment_from_file", true);
	saveEnvironmentToFile = settingsManager.getBoolSetting("graphics", "save_environment_to_file", false);

	settingsManager.saveToIni();

	effects.depthOfField.enabled = false;
	effects.depthOfField.aperture = 8.0f;
	effects.depthOfField.apertureMax = 8.0f;
	effects.depthOfField.blades = 0;
	effects.depthOfField.focalLength = (0.5f * 0.35f) / tan(glm::radians(window->getFieldOfView()) * 0.5f);
	effects.depthOfField.fStopsMax = effects.depthOfField.focalLength / effects.depthOfField.apertureMax;
	effects.depthOfField.fStopsMin = effects.depthOfField.focalLength;
	effects.depthOfField.shutterAngleMax = glm::radians(45.0f);
}

void RenderSystem::input(double _currentTime, double _timeDelta)
{
}

void RenderSystem::update(double _currentTime, double _timeDelta)
{
}

void RenderSystem::render()
{
	std::shared_ptr<Level> level = SystemManager::getInstance().getLevel();
	if (!level || level->cameras.empty())
	{
		// dont render anything if there is no active camera
		return;
	}
	
	effects.exposure = level->exposure * exposureMultiplier;

	if (level->environment.useAtmosphere && !level->environment.isAtmosphereValid)
	{
		level->environment.environmentMap = graphicsFramework->render(level->environment.atmosphereParams);
		level->environment.isAtmosphereValid = true;
	}
	if (!level->environment.environmentProbe->isValid() && level->loaded && !loadEnvironmentFromFile->get())
	{
		graphicsFramework->render(level->environment.environmentProbe, scene, level, effects);
		level->environment.environmentProbe->setValid(true);
		if (saveEnvironmentToFile->get())
		{
			level->environment.environmentProbe->saveToFile(level->filepath + "reflectance.dds", level->filepath + "irradiance.dds");
		}
	}
	graphicsFramework->render(level->cameras[level->activeCameraIndex], scene, level, effects);
}

void RenderSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(entityManager.getComponentBitField(_entity)))
	{
		if (contains(managedEntities, _entity))
		{
			scene.update(_entity);
		}
		else
		{
			managedEntities.push_back(_entity);
			scene.add(_entity);
		}
		scene.sort();
	}
}

void RenderSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (contains(managedEntities, _entity))
	{
		if (validate(entityManager.getComponentBitField(_entity)))
		{
			scene.update(_entity);
			scene.sort();
		}
		else
		{
			remove(managedEntities, _entity);
			scene.remove(_entity);
		}
	}
}

void RenderSystem::onDestruction(const Entity *_entity)
{
	if (contains(managedEntities, _entity))
	{
		scene.remove(_entity);
	}
}

std::shared_ptr<Window> RenderSystem::getWindow()
{
	return window;
}

std::shared_ptr<Camera> RenderSystem::getActiveCamera()
{
	std::shared_ptr<Level> level = SystemManager::getInstance().getLevel();
	if (!level || level->cameras.empty())
	{
		return nullptr;
	}
	return level->cameras[level->activeCameraIndex];
}

unsigned int RenderSystem::getFinishedFrameTexture()
{
	return graphicsFramework->getFinishedFrameTexture();
}

float RenderSystem::getExposureMultiplier()
{
	return exposureMultiplier;
}

void RenderSystem::setExposureMultiplier(float _multiplier)
{
	exposureMultiplier = _multiplier;
}

bool RenderSystem::validate(std::uint64_t _bitMap)
{
	for (std::uint64_t configuration : validBitMaps)
	{
		if ((configuration & _bitMap) == configuration)
		{
			return true;
		}
	}
	return false;
}
