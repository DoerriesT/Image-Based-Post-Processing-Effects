#include <algorithm>
#include <cassert>
#include "RenderSystem.h"
#include ".\..\Component.h"
#include "Utilities\ContainerUtility.h"
#include "Engine.h"
#include "Gui\Gui.h"
#include "Graphics\OpenGL\GraphicsFramework.h"
#include ".\..\EntityManager.h"
#include "Window\Window.h"
#include "Graphics\Camera.h"
#include "Graphics\Texture.h"
#include "Level.h"
#include ".\..\SystemManager.h"
#include "Graphics\EntityRenderData.h"
#include "Settings.h"
#include <glm\ext.hpp>

RenderSystem::RenderSystem(std::shared_ptr<Window> _window)
	:m_graphicsFramework(new GraphicsFramework(_window)),
	m_window(_window),
	m_exposureMultiplier(1.0f),
	m_entityManager(EntityManager::getInstance()),
	m_bakedReflections(false),
	m_bakedIrradianceVolume(false),
	m_scene(),
	m_effects()
{
	m_validBitMaps.push_back(Component<TransformationComponent>::getTypeId() | Component<ModelComponent>::getTypeId() | Component<RenderableComponent>::getTypeId());
}

RenderSystem::~RenderSystem()
{
	delete m_graphicsFramework;
}

void RenderSystem::init()
{
	m_graphicsFramework->init();

	m_entityManager.addOnComponentAddedListener(this);
	m_entityManager.addOnComponentRemovedListener(this);
	m_entityManager.addOnEntityDestructionListener(this);

	SettingsManager &settingsManager = SettingsManager::getInstance();

	shadowQuality = settingsManager.getIntSetting("graphics", "shadow_quality", 0);
	shadowQuality->addListener([&](int _value) { m_effects.m_shadowQuality = (ShadowQuality)_value; });
	m_effects.m_shadowQuality = (ShadowQuality)shadowQuality->get();

	anisotropicFiltering = settingsManager.getIntSetting("graphics", "anisotropic_filtering", 1);
	anisotropicFiltering->addListener([&](int _value) { Texture::setAnisotropicFilteringAll((float)_value); });
	Texture::setAnisotropicFilteringAll((float)anisotropicFiltering->get());

	bloomEnabled = settingsManager.getBoolSetting("graphics", "bloom_enabled", false);
	bloomEnabled->addListener([&](bool _value) { m_effects.m_bloom.m_enabled = _value; });
	m_effects.m_bloom.m_enabled = bloomEnabled->get();

	bloomStrength = settingsManager.getDoubleSetting("graphics", "bloom_strength", 0.01);
	bloomStrength->addListener([&](double _value) { m_effects.m_bloom.m_strength = (float)_value; });
	m_effects.m_bloom.m_strength = (float)bloomStrength->get();

	lensDirtEnabled = settingsManager.getBoolSetting("graphics", "lens_dirt_enabled", false);
	lensDirtEnabled->addListener([&](bool _value) { m_effects.m_lensDirt.m_enabled = _value; });
	m_effects.m_lensDirt.m_enabled = lensDirtEnabled->get();

	lensDirtStrength = settingsManager.getDoubleSetting("graphics", "lens_dirt_strength", 2.0);
	lensDirtStrength->addListener([&](double _value) { m_effects.m_lensDirt.m_strength = (float)_value; });
	m_effects.m_lensDirt.m_strength = (float)lensDirtStrength->get();

	chromaticAberrationEnabled = settingsManager.getBoolSetting("graphics", "chromatic_aberration_enabled", false);
	chromaticAberrationEnabled->addListener([&](bool _value) { m_effects.m_chromaticAberration.m_enabled = _value; });
	m_effects.m_chromaticAberration.m_enabled = chromaticAberrationEnabled->get();

	chromaticAberrationOffsetMult = settingsManager.getDoubleSetting("graphics", "chromatic_aberration_offset_mult", 0.005);
	chromaticAberrationOffsetMult->addListener([&](double _value) { m_effects.m_chromaticAberration.m_offsetMultiplier = (float)_value; });
	m_effects.m_chromaticAberration.m_offsetMultiplier = (float)chromaticAberrationOffsetMult->get();

	filmGrainEnabled = settingsManager.getBoolSetting("graphics", "film_grain_enabled", false);
	filmGrainEnabled->addListener([&](bool _value) { m_effects.m_filmGrain.m_enabled = _value; });
	m_effects.m_filmGrain.m_enabled = filmGrainEnabled->get();

	filmGrainStrength = settingsManager.getDoubleSetting("graphics", "film_grain_strength", 0.055);
	filmGrainStrength->addListener([&](double _value) { m_effects.m_filmGrain.m_strength = (float)_value; });
	m_effects.m_filmGrain.m_strength = (float)filmGrainStrength->get();

	fxaaEnabled = settingsManager.getBoolSetting("graphics", "fxaa_enabled", false);
	fxaaEnabled->addListener([&](bool _value) { m_effects.m_fxaa.m_enabled = _value; });
	m_effects.m_fxaa.m_enabled = fxaaEnabled->get();

	fxaaSubPixelAA = settingsManager.getDoubleSetting("graphics", "fxaa_subpixel_aa", 0.75);
	fxaaSubPixelAA->addListener([&](double _value) { m_effects.m_fxaa.m_subPixelAA = (float)_value; });
	m_effects.m_fxaa.m_subPixelAA = (float)fxaaSubPixelAA->get();

	fxaaEdgeThreshold = settingsManager.getDoubleSetting("graphics", "fxaa_edge_theshold", 0.166);
	fxaaEdgeThreshold->addListener([&](double _value) { m_effects.m_fxaa.m_edgeThreshold = (float)_value; });
	m_effects.m_fxaa.m_edgeThreshold = (float)fxaaEdgeThreshold->get();

	fxaaEdgeThresholdMin = settingsManager.getDoubleSetting("graphics", "fxaa_edge_theshold_min", 0.0833);
	fxaaEdgeThresholdMin->addListener([&](double _value) { m_effects.m_fxaa.m_edgeThresholdMin = (float)_value; });
	m_effects.m_fxaa.m_edgeThresholdMin = (float)fxaaEdgeThresholdMin->get();

	smaaEnabled = settingsManager.getBoolSetting("graphics", "smaa_enabled", false);
	smaaEnabled->addListener([&](bool _value) { m_effects.m_smaa.m_enabled = _value; });
	m_effects.m_smaa.m_enabled = smaaEnabled->get();

	smaaTemporalAA = settingsManager.getBoolSetting("graphics", "smaa_temporal_aa", false);
	smaaTemporalAA->addListener([&](bool _value) { m_effects.m_smaa.m_temporalAntiAliasing = _value; });
	m_effects.m_smaa.m_temporalAntiAliasing = smaaTemporalAA->get();

	lensFlaresEnabled = settingsManager.getBoolSetting("graphics", "lens_flares_enabled", false);
	lensFlaresEnabled->addListener([&](bool _value) { m_effects.m_lensFlares.m_enabled = _value; });
	m_effects.m_lensFlares.m_enabled = lensFlaresEnabled->get();

	lensFlaresChromaticDistortion = settingsManager.getDoubleSetting("graphics", "lens_flares_chromatic_distortion", 1.5);
	lensFlaresChromaticDistortion->addListener([&](double _value) { m_effects.m_lensFlares.m_chromaticDistortion = (float)_value; });
	m_effects.m_lensFlares.m_chromaticDistortion = (float)lensFlaresChromaticDistortion->get();

	lensFlaresCount = settingsManager.getIntSetting("graphics", "lens_flares_count", 4);
	lensFlaresCount->addListener([&](int _value) { m_effects.m_lensFlares.m_flareCount = _value; });
	m_effects.m_lensFlares.m_flareCount = lensFlaresCount->get();

	lensFlaresSpacing = settingsManager.getDoubleSetting("graphics", "lens_flares_spacing", 0.33);
	lensFlaresSpacing->addListener([&](double _value) { m_effects.m_lensFlares.m_flareSpacing = (float)_value; });
	m_effects.m_lensFlares.m_flareSpacing = (float)lensFlaresSpacing->get();

	lensFlaresHaloWidth = settingsManager.getDoubleSetting("graphics", "lens_flares_halo_width", 0.33);
	lensFlaresHaloWidth->addListener([&](double _value) { m_effects.m_lensFlares.m_haloWidth = (float)_value; });
	m_effects.m_lensFlares.m_haloWidth = (float)lensFlaresHaloWidth->get();

	vignetteEnabled = settingsManager.getBoolSetting("graphics", "vignette_enabled", false);
	vignetteEnabled->addListener([&](bool _value) { m_effects.m_vignette.m_enabled = _value; });
	m_effects.m_vignette.m_enabled = vignetteEnabled->get();

	ambientOcclusion = settingsManager.getIntSetting("graphics", "ambient_occlusion", 0);
	ambientOcclusion->addListener([&](int _value) { m_effects.m_ambientOcclusion = static_cast<AmbientOcclusion>(_value); });
	m_effects.m_ambientOcclusion = static_cast<AmbientOcclusion>(ambientOcclusion->get());

	ssaoKernelSize = settingsManager.getIntSetting("graphics", "ssao_kernel_size", 16);
	ssaoKernelSize->addListener([&](int _value) { m_effects.m_ssao.m_kernelSize = _value; });
	m_effects.m_ssao.m_kernelSize = ssaoKernelSize->get();

	ssaoRadius = settingsManager.getDoubleSetting("graphics", "ssao_radius", 0.5);
	ssaoRadius->addListener([&](double _value) { m_effects.m_ssao.m_radius = (float)_value; });
	m_effects.m_ssao.m_radius = (float)ssaoRadius->get();

	ssaoBias = settingsManager.getDoubleSetting("graphics", "ssao_bias", 0.025);
	ssaoBias->addListener([&](double _value) { m_effects.m_ssao.m_bias = (float)_value; });
	m_effects.m_ssao.m_bias = (float)ssaoBias->get();

	ssaoStrength = settingsManager.getDoubleSetting("graphics", "ssao_strength", 1.0);
	ssaoStrength->addListener([&](double _value) { m_effects.m_ssao.m_strength = (float)_value; });
	m_effects.m_ssao.m_strength = (float)ssaoStrength->get();

	hbaoDirections = settingsManager.getIntSetting("graphics", "hbao_directions", 4);
	hbaoDirections->addListener([&](int _value) { m_effects.m_hbao.m_directions = _value; });
	m_effects.m_hbao.m_directions = hbaoDirections->get();

	hbaoSteps = settingsManager.getIntSetting("graphics", "hbao_steps", 4);
	hbaoSteps->addListener([&](int _value) { m_effects.m_hbao.m_steps = _value; });
	m_effects.m_hbao.m_steps = hbaoSteps->get();

	hbaoStrength = settingsManager.getDoubleSetting("graphics", "hbao_strength", 0.5);
	hbaoStrength->addListener([&](double _value) { m_effects.m_hbao.m_strength = (float)_value; });
	m_effects.m_hbao.m_strength = (float)hbaoStrength->get();

	hbaoRadius = settingsManager.getDoubleSetting("graphics", "hbao_radius", 0.3);
	hbaoRadius->addListener([&](double _value) { m_effects.m_hbao.m_radius = (float)_value; });
	m_effects.m_hbao.m_radius = (float)hbaoRadius->get();

	hbaoMaxRadiusPixels = settingsManager.getDoubleSetting("graphics", "hbao_max_radius_pixels", 50.0);
	hbaoMaxRadiusPixels->addListener([&](double _value) { m_effects.m_hbao.m_maxRadiusPixels = (float)_value; });
	m_effects.m_hbao.m_maxRadiusPixels = (float)hbaoMaxRadiusPixels->get();

	hbaoAngleBias = settingsManager.getDoubleSetting("graphics", "hbao_angle_bias", glm::tan(glm::radians(30.0f)));
	hbaoAngleBias->addListener([&](double _value) { m_effects.m_hbao.m_angleBias = (float)_value; });
	m_effects.m_hbao.m_angleBias = (float)hbaoAngleBias->get();

	gtaoSteps = settingsManager.getIntSetting("graphics", "gtao_steps", 4);
	gtaoSteps->addListener([&](int _value) { m_effects.m_gtao.m_steps = _value; });
	m_effects.m_gtao.m_steps = gtaoSteps->get();

	gtaoStrength = settingsManager.getDoubleSetting("graphics", "gtao_strength", 0.5);
	gtaoStrength->addListener([&](double _value) { m_effects.m_gtao.m_strength = (float)_value; });
	m_effects.m_gtao.m_strength = (float)gtaoStrength->get();

	gtaoRadius = settingsManager.getDoubleSetting("graphics", "gtao_radius", 0.3);
	gtaoRadius->addListener([&](double _value) { m_effects.m_gtao.m_radius = (float)_value; });
	m_effects.m_gtao.m_radius = (float)gtaoRadius->get();

	gtaoMaxRadiusPixels = settingsManager.getDoubleSetting("graphics", "gtao_max_radius_pixels", 50.0);
	gtaoMaxRadiusPixels->addListener([&](double _value) { m_effects.m_gtao.m_maxRadiusPixels = (float)_value; });
	m_effects.m_gtao.m_maxRadiusPixels = (float)gtaoMaxRadiusPixels->get();

	motionBlur = settingsManager.getIntSetting("graphics", "motion_blur", 0);
	motionBlur->addListener([&](int _value) { m_effects.m_motionBlur = static_cast<MotionBlur>(_value); });
	m_effects.m_motionBlur = static_cast<MotionBlur>(motionBlur->get());

	bakeReflections = settingsManager.getBoolSetting("graphics", "bake_reflections", false);
	bakeIrradianceVolume = settingsManager.getBoolSetting("graphics", "bake_irradiance_volume", false);

	settingsManager.saveToIni();

	depthOfField = settingsManager.getIntSetting("graphics", "depth_of_field", 0);
	depthOfField->addListener([&](int _value) { m_effects.m_depthOfField = static_cast<DepthOfField>(_value); });
	m_effects.m_depthOfField = static_cast<DepthOfField>(depthOfField->get());
}

void RenderSystem::input(double _currentTime, double _timeDelta)
{
}

void RenderSystem::update(double _currentTime, double _timeDelta)
{
}

int irradianceSource = 1;
bool freeze;
bool anamorphicFlares = false;
bool godrays = false;

void RenderSystem::render()
{
	std::shared_ptr<Level> level = SystemManager::getInstance().getLevel();
	if (!level || level->m_cameras.empty())
	{
		// dont render anything if there is no active camera
		return;
	}

	m_effects.m_exposure = level->m_exposure * m_exposureMultiplier;
	m_effects.m_diffuseAmbientSource = DiffuseAmbientSource(irradianceSource);
	m_effects.m_anamorphicFlares.m_enabled = anamorphicFlares;
	m_effects.m_anamorphicFlares.m_color = glm::vec3(0.5f, 0.5f, 1.0f);
	m_effects.m_godrays = godrays;

	// calculate transformations
	if(!freeze)
	{
		// TODO: avoid this set
		std::set<const Entity *> transformedEntities;
		for (const auto &entityData : m_scene.getData())
		{
			if (!ContainerUtility::contains(transformedEntities, entityData->m_entity))
			{
				transformedEntities.insert(entityData->m_entity);
				entityData->m_transformationComponent->m_prevTransformation = entityData->m_transformationComponent->m_transformation;
				entityData->m_transformationComponent->m_transformation = glm::translate(entityData->m_transformationComponent->m_position)
					* glm::mat4_cast(entityData->m_transformationComponent->m_rotation)
					* glm::scale(glm::vec3(entityData->m_transformationComponent->m_scale));
			}
		}
	}

	Environment &environment = level->m_environment;

	if (environment.m_useAtmosphere && !environment.m_isAtmosphereValid)
	{
		environment.m_environmentMap = m_graphicsFramework->render(environment.m_atmosphereParams);
		environment.m_isAtmosphereValid = true;
	}

	if (level->m_loaded &&
		((!m_bakedReflections && bakeReflections->get()) ||
		(!m_bakedIrradianceVolume && bakeIrradianceVolume->get())))
	{
		m_graphicsFramework->bake(m_scene, level, 2, !m_bakedReflections && bakeReflections->get(), !m_bakedIrradianceVolume && bakeIrradianceVolume->get());

		if (bakeReflections->get())
		{
			for (auto probe : level->m_environment.m_environmentProbes)
			{
				probe->saveToFile();
			}
			m_bakedReflections = true;
		}

		if (bakeIrradianceVolume->get())
		{
			level->m_environment.m_irradianceVolume->saveToFile(level->m_filepath + "probes.dds");
			m_bakedIrradianceVolume = true;
		}
	}
	m_graphicsFramework->render(level->m_cameras[level->m_activeCameraIndex], m_scene, level, m_effects);
}

void RenderSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(m_entityManager.getComponentBitField(_entity)))
	{
		if (ContainerUtility::contains(m_managedEntities, _entity))
		{
			m_scene.update(_entity);
		}
		else
		{
			m_managedEntities.push_back(_entity);
			m_scene.add(_entity);
		}
		m_scene.sort();
	}
}

void RenderSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (ContainerUtility::contains(m_managedEntities, _entity))
	{
		if (validate(m_entityManager.getComponentBitField(_entity)))
		{
			m_scene.update(_entity);
			m_scene.sort();
		}
		else
		{
			ContainerUtility::remove(m_managedEntities, _entity);
			m_scene.remove(_entity);
		}
	}
}

void RenderSystem::onDestruction(const Entity *_entity)
{
	if (ContainerUtility::contains(m_managedEntities, _entity))
	{
		m_scene.remove(_entity);
	}
}

std::shared_ptr<Window> RenderSystem::getWindow()
{
	return m_window;
}

std::shared_ptr<Camera> RenderSystem::getActiveCamera()
{
	std::shared_ptr<Level> level = SystemManager::getInstance().getLevel();
	if (!level || level->m_cameras.empty())
	{
		return nullptr;
	}
	return level->m_cameras[level->m_activeCameraIndex];
}

unsigned int RenderSystem::getFinishedFrameTexture()
{
	return m_graphicsFramework->getFinishedFrameTexture();
}

float RenderSystem::getExposureMultiplier()
{
	return m_exposureMultiplier;
}

void RenderSystem::setExposureMultiplier(float _multiplier)
{
	m_exposureMultiplier = _multiplier;
}

bool RenderSystem::validate(std::uint64_t _bitMap)
{
	for (std::uint64_t configuration : m_validBitMaps)
	{
		if ((configuration & _bitMap) == configuration)
		{
			return true;
		}
	}
	return false;
}
