#define NK_IMPLEMENTATION
#include "Application.h"
#include "SponzaLevel.h"
#include <EntityComponentSystem\SystemManager.h>
#include <EntityComponentSystem\EntityManager.h>
#include <Engine.h>
#include <Window\Window.h>
#include <random>
#include <EasingFunctions.h>
#include <Input\UserInput.h>
#include <Input\Gamepad.h>
#include <Graphics\Effects.h>
#include <sstream>
#include <Utilities/Utility.h>
#include <fstream>

#define GETTER_FUNC_DEF(name, type) void TW_CALL name##GetCallback(void *value, void *clientData)	\
									{																\
										App::Application *app = (App::Application *)clientData;		\
										*(type *)value = app->##name##->get();					\
									}

#define SETTER_FUNC_DEF(name, type) void TW_CALL name##SetCallback(const void *value, void *clientData)	\
									{																	\
										App::Application *app = (App::Application *)clientData;			\
										app->##name##->set(*(type *)value);								\
										SettingsManager::getInstance().saveToIni();						\
									}

#define COMBINED_FUNC_DEF(name, type) GETTER_FUNC_DEF(name, type) SETTER_FUNC_DEF(name, type)
#define GETTER_FUNC_PTR(name) name##GetCallback
#define SETTER_FUNC_PTR(name) name##SetCallback

extern bool godrays;
bool renderLightProbes = false;
extern GBufferDisplayMode displayMode;
extern bool gtaoMultiBounce;
extern bool freeze;
extern bool anamorphicFlares;
extern float fNumber;
extern bool constantVelocity;


namespace App
{
	COMBINED_FUNC_DEF(shadowQuality, int)
		COMBINED_FUNC_DEF(anisotropicFiltering, int)
		COMBINED_FUNC_DEF(bloomEnabled, bool)
		COMBINED_FUNC_DEF(fxaaEnabled, bool)
		COMBINED_FUNC_DEF(smaaEnabled, bool)
		COMBINED_FUNC_DEF(smaaTemporalAA, bool)
		COMBINED_FUNC_DEF(lensFlaresEnabled, bool)
		COMBINED_FUNC_DEF(ambientOcclusion, int)
		COMBINED_FUNC_DEF(vsync, bool)
		COMBINED_FUNC_DEF(windowWidth, int)
		COMBINED_FUNC_DEF(windowHeight, int)
		COMBINED_FUNC_DEF(windowMode, int)
		COMBINED_FUNC_DEF(motionBlur, int)
		COMBINED_FUNC_DEF(depthOfField, int)
		COMBINED_FUNC_DEF(ssaoKernelSize, int)
		COMBINED_FUNC_DEF(ssaoRadius, double)
		COMBINED_FUNC_DEF(ssaoStrength, double)
		COMBINED_FUNC_DEF(hbaoDirections, int)
		COMBINED_FUNC_DEF(hbaoSteps, int)
		COMBINED_FUNC_DEF(hbaoStrength, double)
		COMBINED_FUNC_DEF(hbaoRadius, double)
		COMBINED_FUNC_DEF(hbaoMaxRadiusPixels, double)
		COMBINED_FUNC_DEF(hbaoAngleBias, double)
		COMBINED_FUNC_DEF(lensDirtEnabled, bool)
		COMBINED_FUNC_DEF(lensDirtStrength, double)
		COMBINED_FUNC_DEF(gtaoSteps, int)
		COMBINED_FUNC_DEF(gtaoStrength, double)
		COMBINED_FUNC_DEF(gtaoRadius, double)
		COMBINED_FUNC_DEF(gtaoMaxRadiusPixels, double)

		void TW_CALL windowResolutionGetCallback(void *value, void *clientData)
	{
		*(int *)value = (int)Engine::getInstance()->getWindow()->getSelectedResolutionIndex();
	}

	void TW_CALL windowResolutionSetCallback(const void *value, void *clientData)
	{
		Window *window = Engine::getInstance()->getWindow();
		auto resolution = window->getSupportedResolutions()[*(int *)value];

		App::Application *app = (App::Application *)clientData;
		app->windowWidth->set((int)resolution.first);
		app->windowHeight->set((int)resolution.second);
		SettingsManager::getInstance().saveToIni();
	}

	unsigned int scene;

	void TW_CALL sceneGetCallback(void *value, void *clientData)
	{
		*(unsigned int *)value = scene;
	}

	void TW_CALL sceneSetCallback(const void *value, void *clientData)
	{
		Camera &camera = *(Camera *)clientData;
		scene = *((unsigned int *)value);

		glm::vec3 pos;
		glm::quat rot;

		switch (scene)
		{
		case 0:
			pos = glm::vec3(12.0f, 1.8f, 0.0f);
			rot = glm::quat(glm::vec3(0.0f, glm::radians(-90.0f), 0.0f));
			break;
		case 1:
			pos = { -9.38771534f, 0.529580772f,  -1.64638960f };
			rot.x = -0.0124382321f;
			rot.y = 0.703372002f;
			rot.z = -0.0123098688f;
			rot.w = 0.710606515f;
			break;
		case 2:
			pos = { -9.57731056f, 0.258460432f,  -0.0429794118f };
			rot.x = 0.0531200469f;
			rot.y = 0.703272939f;
			rot.z = 0.0528445318f;
			rot.w = 0.706960320f;
			break;
		case 3:
			pos = { 13.4150219f, 1.95901549f,  -0.898027897f };
			rot.x = -0.0170103684f;
			rot.y = 0.973960757f;
			rot.z = 0.0780826211f;
			rot.w = -0.212165594f;
			break;
		case 4:
			pos = { 10.1570253f, 1.65887976f,  1.64646232f };
			rot.x = -0.119959101f;
			rot.y = 0.551599503f;
			rot.z = 0.0805459917f;
			rot.w = -0.821498811f;
			break;
		case 5:
			pos = { -0.964327455f, 1.59974909f,  -0.0586786345f };
			rot.x = 0.0649230108f;
			rot.y = 0.708267391f;
			rot.z = 0.0657015070f;
			rot.w = 0.699875534f;
			break;
		case 6:
			pos = { -1.87112761f, 4.62619543f, 5.61673927f };
			rot.x = 0.116875365f;
			rot.y = 0.491376519f;
			rot.z = 0.0667413920f;
			rot.w = 0.860485256f;
			break;
		default:
			assert(false);
			break;
		}

		camera.setPosition(pos);
		camera.setRotation(rot);
	}

	void TW_CALL mouseSmoothFactorGetCallback(void *value, void *clientData)
	{
		*(float *)value = ((CameraController *)clientData)->getSmoothFactor();
	}

	void TW_CALL mouseSmoothFactorSetCallback(const void *value, void *clientData)
	{
		((CameraController *)clientData)->setSmoothFactor(*(float *)value);
	}

	void TW_CALL carMovementGetCallback(void *value, void *clientData)
	{
		*(bool *)value = EntityManager::getInstance().getComponent<MovementPathComponent>((const Entity *)clientData);
	}

	void TW_CALL carMovementSetCallback(const void *value, void *clientData)
	{
		if (*(bool *)value)
		{
			std::vector<PathSegment> pathSegments;
			pathSegments.push_back(PathSegment(
				glm::vec3(-9.0, 0.0, 0.0),	// start pos
				glm::vec3(9.0, 0.0, 0.0),	// end pos
				glm::vec3(1.0f, 0.0f, 0.0f),						// start tangent
				glm::vec3(1.0f, 0.0f, 0.0f),						// end tangent
				1.0,														// duration
				linear));													// easing function
			pathSegments.push_back(PathSegment(
				glm::vec3(9.0, 0.0, 0.0),
				glm::vec3(-9.0, 0.0, 0.0),
				glm::vec3(-1.0f, 0.0f, 0.0),
				glm::vec3(-1.0f, 0.0f, 0.0),
				1.0,
				linear));

			EntityManager::getInstance().addComponent<MovementPathComponent>((const Entity *)clientData, pathSegments, Engine::getTime(), true);
		}
		else
		{
			EntityManager::getInstance().removeComponent<MovementPathComponent>((const Entity *)clientData);
			EntityManager::getInstance().getComponent<TransformationComponent>((const Entity *)clientData)->m_position = glm::vec3(6.0, 0.0, 0.0);
		}
	}

	void TW_CALL benchmarkButton(void *clientData)
	{
		bool &benchmarkEnabled = *((bool *)clientData);

		if (!benchmarkEnabled)
		{
			benchmarkEnabled = true;
		}
	}

	Application::Application()
		:m_cameraController(),
		guiVisible(true)
	{
	}

	void Application::init()
	{
		// load settings
		{
			SettingsManager &settingsManager = SettingsManager::getInstance();

			uiSizeOffset = settingsManager.getIntSetting("graphics", "gui_size", -1);
			shadowQuality = settingsManager.getIntSetting("graphics", "shadow_quality", 0);
			anisotropicFiltering = settingsManager.getIntSetting("graphics", "anisotropic_filtering", 1);
			bloomEnabled = settingsManager.getBoolSetting("graphics", "bloom_enabled", false);
			fxaaEnabled = settingsManager.getBoolSetting("graphics", "fxaa_enabled", false);
			smaaEnabled = settingsManager.getBoolSetting("graphics", "smaa_enabled", false);
			smaaTemporalAA = settingsManager.getBoolSetting("graphics", "smaa_temporal_aa", false);
			lensFlaresEnabled = settingsManager.getBoolSetting("graphics", "lens_flares_enabled", false);
			ambientOcclusion = settingsManager.getIntSetting("graphics", "ambient_occlusion", 0);
			vsync = settingsManager.getBoolSetting("graphics", "vsync", false);
			windowWidth = settingsManager.getIntSetting("graphics", "window_width", 1080);
			windowHeight = settingsManager.getIntSetting("graphics", "window_height", 720);
			windowMode = settingsManager.getIntSetting("graphics", "window_mode", 0);
			motionBlur = settingsManager.getIntSetting("graphics", "motion_blur", 0);
			depthOfField = settingsManager.getIntSetting("graphics", "depth_of_field", 0);
			ssaoKernelSize = settingsManager.getIntSetting("graphics", "ssao_kernel_size", 16);
			ssaoRadius = settingsManager.getDoubleSetting("graphics", "ssao_radius", 0.5);
			ssaoStrength = settingsManager.getDoubleSetting("graphics", "ssao_strength", 1.0);
			hbaoDirections = settingsManager.getIntSetting("graphics", "hbao_directions", 4);
			hbaoSteps = settingsManager.getIntSetting("graphics", "hbao_steps", 4);
			hbaoStrength = settingsManager.getDoubleSetting("graphics", "hbao_strength", 0.5);
			hbaoRadius = settingsManager.getDoubleSetting("graphics", "hbao_radius", 0.3);
			hbaoMaxRadiusPixels = settingsManager.getDoubleSetting("graphics", "hbao_max_radius_pixels", 50.0);
			hbaoAngleBias = settingsManager.getDoubleSetting("graphics", "hbao_angle_bias", glm::tan(glm::radians(30.0f)));
			gtaoSteps = settingsManager.getIntSetting("graphics", "gtao_steps", 4);
			gtaoStrength = settingsManager.getDoubleSetting("graphics", "gtao_strength", 0.5);
			gtaoRadius = settingsManager.getDoubleSetting("graphics", "gtao_radius", 0.3);
			gtaoMaxRadiusPixels = settingsManager.getDoubleSetting("graphics", "gtao_max_radius_pixels", 50.0);
			screenSpaceReflectionsEnabled = settingsManager.getBoolSetting("graphics", "screen_space_reflections_enabled", false);
			lensDirtEnabled = settingsManager.getBoolSetting("graphics", "lens_dirt_enabled", false);
			lensDirtStrength = settingsManager.getDoubleSetting("graphics", "lens_dirt_strength", 2.0);

			settingsManager.saveToIni();
		}


		m_level = loadSponzaLevel();
		SystemManager::getInstance().setLevel(m_level);
		m_cameraController.setCamera(m_level->m_cameras[m_level->m_activeCameraIndex]);
		m_cameraController.setSmoothFactor(0.85f);

		TwInit(TW_OPENGL_CORE, NULL); // for core profile
		TwWindowSize(Engine::getInstance()->getWindow()->getWidth(), Engine::getInstance()->getWindow()->getHeight());
		m_settingsTweakBar = TwNewBar("Settings");
		m_timingsTweakBar = TwNewBar("Timings");
		TwDefine("Settings refresh=0.49");

		// timings
		{
			TwAddVarRO(m_timingsTweakBar, "FPS", TW_TYPE_STDSTRING, &fpsStr, "group=Frame_Timings");
			TwAddVarRO(m_timingsTweakBar, "FPS Average", TW_TYPE_STDSTRING, &fpsAvgStr, "group=Frame_Timings");
			TwAddVarRO(m_timingsTweakBar, "FPS Worst", TW_TYPE_STDSTRING, &fpsWorstStr, "group=Frame_Timings");
			TwAddVarRO(m_timingsTweakBar, "Frame Time", TW_TYPE_STDSTRING, &frameTimeStr, "group=Frame_Timings");
			TwAddVarRO(m_timingsTweakBar, "Frame Time Average", TW_TYPE_STDSTRING, &frameTimeAvgStr, "group=Frame_Timings");
			TwAddVarRO(m_timingsTweakBar, "Frame Time Worst", TW_TYPE_STDSTRING, &frameTimeWorstStr, "group=Frame_Timings");

			// mb
			TwAddVarRO(m_timingsTweakBar, "Simple MB Velocity Correction", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_velocityCorrectionComputeTime, "group=Simple_Motion_Blur_Timings");
			TwAddVarRO(m_timingsTweakBar, "Simple MB Render", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_motionBlurRenderTime, "group=Simple_Motion_Blur_Timings");
			TwAddVarRO(m_timingsTweakBar, "Simple MB Total", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_simpleMbSum, "group=Simple_Motion_Blur_Timings");

			TwAddVarRO(m_timingsTweakBar, "Single Direction MB Velocity Correction", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_velocityCorrectionComputeTime, "group=Single_Direction_Motion_Blur_Timings");
			TwAddVarRO(m_timingsTweakBar, "Single Direction MB Velocity Tile Max", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_velocityTileMaxRenderTime, "group=Single_Direction_Motion_Blur_Timings");
			TwAddVarRO(m_timingsTweakBar, "Single Direction MB Velocity Neighbor Tile Max", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_velocityNeighborTileMaxRenderTime, "group=Single_Direction_Motion_Blur_Timings");
			TwAddVarRO(m_timingsTweakBar, "Single Direction MB Render", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_motionBlurRenderTime, "group=Single_Direction_Motion_Blur_Timings");
			TwAddVarRO(m_timingsTweakBar, "Single Direction MB Total", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_singleDirectionMbSum, "group=Single_Direction_Motion_Blur_Timings");

			TwAddVarRO(m_timingsTweakBar, "Multi Direction MB Velocity Correction", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_velocityCorrectionComputeTime, "group=Multi_Direction_Motion_Blur_Timings");
			TwAddVarRO(m_timingsTweakBar, "Multi Direction MB Velocity Tile Max", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_velocityTileMaxRenderTime, "group=Multi_Direction_Motion_Blur_Timings");
			TwAddVarRO(m_timingsTweakBar, "Multi Direction MB Velocity Neighbor Tile Max", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_velocityNeighborTileMaxRenderTime, "group=Multi_Direction_Motion_Blur_Timings");
			TwAddVarRO(m_timingsTweakBar, "Multi Direction MB Render", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_motionBlurRenderTime, "group=Multi_Direction_Motion_Blur_Timings");
			TwAddVarRO(m_timingsTweakBar, "Multi Direction MB Total", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_multiDirectionMbSum, "group=Multi_Direction_Motion_Blur_Timings");

			// dof
			TwAddVarRO(m_timingsTweakBar, "Simple DoF CoC Compute", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_cocComputeTime, "group=Simple_Depth_of_Field_Timings");
			TwAddVarRO(m_timingsTweakBar, "Simple DoF CoC Blur", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_simpleDofCocBlurComputeTime, "group=Simple_Depth_of_Field_Timings");
			TwAddVarRO(m_timingsTweakBar, "Simple DoF Blur", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_simpleDofBlurComputeTime, "group=Simple_Depth_of_Field_Timings");
			TwAddVarRO(m_timingsTweakBar, "Simple DoF Composite", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_simpleDofCompositeComputeTime, "group=Simple_Depth_of_Field_Timings");
			TwAddVarRO(m_timingsTweakBar, "Simple DoF Total", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_simpleDofSum, "group=Simple_Depth_of_Field_Timings");

			TwAddVarRO(m_timingsTweakBar, "Sprite DoF CoC Compute", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_cocComputeTime, "group=Sprite_Depth_of_Field_Timings");
			TwAddVarRO(m_timingsTweakBar, "Sprite DoF Render", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_spriteDofRenderTime, "group=Sprite_Depth_of_Field_Timings");
			TwAddVarRO(m_timingsTweakBar, "Sprite DoF Composite", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_spriteDofCompositeComputeTime, "group=Sprite_Depth_of_Field_Timings");
			TwAddVarRO(m_timingsTweakBar, "Sprite DoF Total", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_spriteDofSum, "group=Sprite_Depth_of_Field_Timings");

			TwAddVarRO(m_timingsTweakBar, "Tiled DoF CoC Compute", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_cocComputeTime, "group=Tiled_Depth_of_Field_Timings");
			TwAddVarRO(m_timingsTweakBar, "Tiled DoF Coc Tile Max", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_cocTileMaxRenderTime, "group=Tiled_Depth_of_Field_Timings");
			TwAddVarRO(m_timingsTweakBar, "Tiled DoF Coc Neighbor Tile Max", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_cocNeighborTileMaxRenderTime, "group=Tiled_Depth_of_Field_Timings");
			TwAddVarRO(m_timingsTweakBar, "Tiled DoF Downsample", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_seperateDofDownsampleComputeTime, "group=Tiled_Depth_of_Field_Timings");
			TwAddVarRO(m_timingsTweakBar, "Tiled DoF Blur", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_seperateDofBlurComputeTime, "group=Tiled_Depth_of_Field_Timings");
			TwAddVarRO(m_timingsTweakBar, "Tiled DoF Fill", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_seperateDofFillComputeTime, "group=Tiled_Depth_of_Field_Timings");
			TwAddVarRO(m_timingsTweakBar, "Tiled DoF Composite", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_seperateDofCompositeComputeTime, "group=Tiled_Depth_of_Field_Timings");
			TwAddVarRO(m_timingsTweakBar, "Tiled DoF Total", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_tiledDofSum, "group=Tiled_Depth_of_Field_Timings");

			// ssao
			TwAddVarRO(m_timingsTweakBar, "SSAO (Original)", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_originalSsaoRenderTime, "group=SSAO_Original_Timings");
			TwAddVarRO(m_timingsTweakBar, "SSAO (Original) Blur", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_bilateralBlurRenderTime, "group=SSAO_Original_Timings");
			TwAddVarRO(m_timingsTweakBar, "SSAO (Original) Total", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_ssaoOriginalSum, "group=SSAO_Original_Timings");

			TwAddVarRO(m_timingsTweakBar, "SSAO", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_ssaoRenderTime, "group=SSAO_Timings");
			TwAddVarRO(m_timingsTweakBar, "SSAO Blur", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_bilateralBlurRenderTime, "group=SSAO_Timings");
			TwAddVarRO(m_timingsTweakBar, "SSAO Total", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_ssaoSum, "group=SSAO_Timings");

			TwAddVarRO(m_timingsTweakBar, "HBAO", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_hbaoRenderTime, "group=HBAO_Timings");
			TwAddVarRO(m_timingsTweakBar, "HBAO Blur", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_bilateralBlurRenderTime, "group=HBAO_Timings");
			TwAddVarRO(m_timingsTweakBar, "HBAO Total", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_hbaoSum, "group=HBAO_Timings");

			TwAddVarRO(m_timingsTweakBar, "GTAO Render", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_gtaoRenderTime, "group=GTAO_Timings");
			TwAddVarRO(m_timingsTweakBar, "GTAO Spatial", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_gtaoSpatialDenoiseTime, "group=GTAO_Timings");
			TwAddVarRO(m_timingsTweakBar, "GTAO Temporal", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_gtaoTemporalDenoiseTime, "group=GTAO_Timings");
			TwAddVarRO(m_timingsTweakBar, "GTAO Total", TW_TYPE_DOUBLE, &m_currentFrameTimings.m_gtaoSum, "group=GTAO_Timings");
		}

		{
			// mouse
			{
				TwAddVarCB(m_settingsTweakBar, "Smoothing Factor", TW_TYPE_FLOAT, SETTER_FUNC_PTR(mouseSmoothFactor), GETTER_FUNC_PTR(mouseSmoothFactor), &m_cameraController, "group=Mouse min=0.0 max=1.0 step=0.01");
			}

			// window
			{
				// window mode
				{
					TwEnumVal windowOptions[] = { { (int)WindowMode::WINDOWED, "Windowed" },{ (int)WindowMode::BORDERLESS_FULLSCREEN, "Borderless Fullscreen" },{ (int)WindowMode::FULLSCREEN, "Fullscreen" } };
					TwType WindowModeTwType = TwDefineEnum("WindowModeType", windowOptions, 3);
					TwAddVarCB(m_settingsTweakBar, "Window Mode", WindowModeTwType, SETTER_FUNC_PTR(windowMode), GETTER_FUNC_PTR(windowMode), this, "group=Window");
				}

				// window resolution
				{
					std::vector<TwEnumVal> resolutionOptions;
					Window *window = Engine::getInstance()->getWindow();
					auto resolutions = window->getSupportedResolutions();
					size_t resolutionsCount = resolutions.size();
					assert(!resolutions.empty());

					for (size_t i = 0; i < resolutionsCount; ++i)
					{
						const unsigned int w = resolutions[i].first;
						const unsigned int h = resolutions[i].second;
						m_resolutionOptionStrings.push_back(std::to_string(w) + " x " + std::to_string(h));
					}

					// only call c_str() after vector is completely filled (don't touch it after this point or the char pointers will invalidate)
					// TODO: find better solution
					for (size_t i = 0; i < m_resolutionOptionStrings.size(); ++i)
					{
						const char *str = m_resolutionOptionStrings[i].c_str();
						resolutionOptions.push_back({ (int)i, str });
					}

					TwType ResolutionTwType = TwDefineEnum("ResolutionType", resolutionOptions.data(), resolutionOptions.size());
					TwAddVarCB(m_settingsTweakBar, "Window Resolution", ResolutionTwType, SETTER_FUNC_PTR(windowResolution), GETTER_FUNC_PTR(windowResolution), this, "group=Window");
				}

				// vsync
				TwAddVarCB(m_settingsTweakBar, "V-Sync", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(vsync), GETTER_FUNC_PTR(vsync), this, "group=Window");
			}

			// af
			{
				// af
				{
					int aa = Engine::getInstance()->getMaxAnisotropicFiltering();
					std::vector<TwEnumVal> afOptions;
					afOptions.push_back({ 1, "Off" });

					for (int i = 2; i <= aa; i *= 2)
					{
						// strings need to reside somewhere -> c_str()
						m_afOptionStrings.push_back("x" + std::to_string(i));
					}

					// only call c_str() after vector is completely filled (don't touch it after this point or the char pointers will invalidate)
					// TODO: find better solution
					for (int i = 2, j = 0; i <= aa; i *= 2, ++j)
					{
						const char *str = m_afOptionStrings[j].c_str();
						afOptions.push_back({ i, str });
					}

					TwType DofTwType = TwDefineEnum("AfType", afOptions.data(), afOptions.size());
					TwAddVarCB(m_settingsTweakBar, "Anisotropic Filtering", DofTwType, SETTER_FUNC_PTR(anisotropicFiltering), GETTER_FUNC_PTR(anisotropicFiltering), this, "group=Anisotropic_Filtering");
				}
			}

			// anti aliasing
			{
				TwAddVarCB(m_settingsTweakBar, "FXAA", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(fxaaEnabled), GETTER_FUNC_PTR(fxaaEnabled), this, "group=Anti-Aliasing");
				TwAddVarCB(m_settingsTweakBar, "SMAA", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(smaaEnabled), GETTER_FUNC_PTR(smaaEnabled), this, "group=Anti-Aliasing");
				TwAddVarCB(m_settingsTweakBar, "SMAA Temporal AA", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(smaaTemporalAA), GETTER_FUNC_PTR(smaaTemporalAA), this, "group=Anti-Aliasing");
			}

			// depth of field
			{
				TwEnumVal dofOptions[] = { { (int)DepthOfField::OFF, "Off" },{ (int)DepthOfField::SIMPLE, "Simple" },{ (int)DepthOfField::SPRITE_BASED, "Sprite Based" },{ (int)DepthOfField::TILE_BASED, "Tile-Based" } };
				TwType DofTwType = TwDefineEnum("DepthOfFieldType", dofOptions, 4);
				TwAddVarCB(m_settingsTweakBar, "Depth of Field", DofTwType, SETTER_FUNC_PTR(depthOfField), GETTER_FUNC_PTR(depthOfField), this, "group=Depth_of_Field");
				TwAddVarRW(m_settingsTweakBar, "f-Number", TW_TYPE_FLOAT, &fNumber, "group=Depth_of_Field min=1.4 max=16.0 step=0.1");
			}

			// motion blur
			{
				TwEnumVal mbOptions[] = { { (int)MotionBlur::OFF, "Off" },{ (int)MotionBlur::SIMPLE, "Simple" },{ (int)MotionBlur::TILE_BASED_SINGLE, "Tile-Based Single Direction" },{ (int)MotionBlur::TILE_BASED_MULTI, "Tile-Based Multi Direction" } };
				TwType MbTwType = TwDefineEnum("MotionBlurType", mbOptions, 4);
				TwAddVarCB(m_settingsTweakBar, "Motion Blur", MbTwType, SETTER_FUNC_PTR(motionBlur), GETTER_FUNC_PTR(motionBlur), this, "group=Motion_Blur");
			}

			// ambient occlusion
			{
				TwEnumVal aoOptions[] = { { (int)AmbientOcclusion::OFF, "Off" }, { (int)AmbientOcclusion::SSAO_ORIGINAL, "SSAO (Original)" }, { (int)AmbientOcclusion::SSAO, "SSAO" }, { (int)AmbientOcclusion::HBAO, "HBAO" }, {(int)AmbientOcclusion::GTAO, "GTAO"} };
				TwType AoTwType = TwDefineEnum("AoType", aoOptions, 5);
				TwAddVarCB(m_settingsTweakBar, "Ambient Occlusion", AoTwType, SETTER_FUNC_PTR(ambientOcclusion), GETTER_FUNC_PTR(ambientOcclusion), this, "group=Ambient_Occlusion");

				TwAddVarCB(m_settingsTweakBar, "SSAO Kernel Size", TW_TYPE_INT32, SETTER_FUNC_PTR(ssaoKernelSize), GETTER_FUNC_PTR(ssaoKernelSize), this, "group=Ambient_Occlusion min=1 max=64");
				TwAddVarCB(m_settingsTweakBar, "SSAO Radius", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(ssaoRadius), GETTER_FUNC_PTR(ssaoRadius), this, "group=Ambient_Occlusion min=0.1 max=10.0 step=0.1");
				TwAddVarCB(m_settingsTweakBar, "SSAO Strength", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(ssaoStrength), GETTER_FUNC_PTR(ssaoStrength), this, "group=Ambient_Occlusion min=0.1 max=10.0 step=0.1");
				TwAddVarCB(m_settingsTweakBar, "HBAO Directions", TW_TYPE_INT32, SETTER_FUNC_PTR(hbaoDirections), GETTER_FUNC_PTR(hbaoDirections), this, "group=Ambient_Occlusion min=1 max=32");
				TwAddVarCB(m_settingsTweakBar, "HBAO Steps", TW_TYPE_INT32, SETTER_FUNC_PTR(hbaoSteps), GETTER_FUNC_PTR(hbaoSteps), this, "group=Ambient_Occlusion min=1 max=32");
				TwAddVarCB(m_settingsTweakBar, "HBAO Strength", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(hbaoStrength), GETTER_FUNC_PTR(hbaoStrength), this, "group=Ambient_Occlusion min=0.1 max=10.0 step=0.1");
				TwAddVarCB(m_settingsTweakBar, "HBAO Radius", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(hbaoRadius), GETTER_FUNC_PTR(hbaoRadius), this, "group=Ambient_Occlusion min=0.1 max=10.0 step=0.1");
				TwAddVarCB(m_settingsTweakBar, "HBAO Max Radius Pixels", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(hbaoMaxRadiusPixels), GETTER_FUNC_PTR(hbaoMaxRadiusPixels), this, "group=Ambient_Occlusion min=1 max=256");
				TwAddVarCB(m_settingsTweakBar, "HBAO Angle Bias", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(hbaoAngleBias), GETTER_FUNC_PTR(hbaoAngleBias), this, "group=Ambient_Occlusion min=0.0 max=1.5 step=0.01");
				TwAddVarCB(m_settingsTweakBar, "GTAO Steps", TW_TYPE_INT32, SETTER_FUNC_PTR(gtaoSteps), GETTER_FUNC_PTR(gtaoSteps), this, "group=Ambient_Occlusion min=1 max=32");
				TwAddVarCB(m_settingsTweakBar, "GTAO Strength", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(gtaoStrength), GETTER_FUNC_PTR(gtaoStrength), this, "group=Ambient_Occlusion min=0.1 max=10.0 step=0.1");
				TwAddVarCB(m_settingsTweakBar, "GTAO Radius", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(gtaoRadius), GETTER_FUNC_PTR(gtaoRadius), this, "group=Ambient_Occlusion min=0.1 max=10.0 step=0.1");
				TwAddVarCB(m_settingsTweakBar, "GTAO Max Radius Pixels", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(gtaoMaxRadiusPixels), GETTER_FUNC_PTR(gtaoMaxRadiusPixels), this, "group=Ambient_Occlusion min=1 max=256");
				TwAddVarRW(m_settingsTweakBar, "GTAO Multi Bounce", TW_TYPE_BOOLCPP, &gtaoMultiBounce, "group=Ambient_Occlusion");
			}

			// lens
			{
				TwAddVarCB(m_settingsTweakBar, "Lens Flares", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(lensFlaresEnabled), GETTER_FUNC_PTR(lensFlaresEnabled), this, "group=Lens");
				TwAddVarRW(m_settingsTweakBar, "Anamorphic Flares", TW_TYPE_BOOLCPP, &anamorphicFlares, "group=Lens");
				TwAddVarCB(m_settingsTweakBar, "Bloom", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(bloomEnabled), GETTER_FUNC_PTR(bloomEnabled), this, "group=Lens ");
				TwAddVarCB(m_settingsTweakBar, "Lens Dirt", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(lensDirtEnabled), GETTER_FUNC_PTR(lensDirtEnabled), this, "group=Lens ");
				TwAddVarCB(m_settingsTweakBar, "Lens Dirt Strength", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(lensDirtStrength), GETTER_FUNC_PTR(lensDirtStrength), this, "group=Lens min=0.0 max=10.0 step=0.1");
			}

			// misc
			{
				TwEnumVal displayOptions[] = {
					{ (int)GBufferDisplayMode::SHADED, "Shaded" },
				{ (int)GBufferDisplayMode::ALBEDO, "Albedo" },
				{ (int)GBufferDisplayMode::NORMAL, "Normal" },
				{ (int)GBufferDisplayMode::MATERIAL, "Material" },
				{ (int)GBufferDisplayMode::DEPTH, "Depth" },
				{ (int)GBufferDisplayMode::VELOCITY, "Velocity" },
				{ (int)GBufferDisplayMode::AMBIENT_OCCLUSION, "Ambient Occlusion" }
				};
				TwType DisplayTwType = TwDefineEnum("DisplayType", displayOptions, 7);
				TwAddVarRW(m_settingsTweakBar, "Display Mode", DisplayTwType, &displayMode, "group=Misc");

				TwAddVarRW(m_settingsTweakBar, "God Rays", TW_TYPE_BOOLCPP, &godrays, "group=Misc");
				TwAddVarRW(m_settingsTweakBar, "Show Light Probes", TW_TYPE_BOOLCPP, &renderLightProbes, "group=Misc");

				TwAddVarCB(m_settingsTweakBar, "Car Movement", TW_TYPE_BOOLCPP, carMovementSetCallback, carMovementGetCallback, (void *)m_level->m_entityMap["car"], "group=Misc");
			}

			// benchmark
			{
				TwAddButton(m_settingsTweakBar, "Start Benchmark", benchmarkButton, &benchmarkIsRunning, "group=Benchmark");
				TwAddVarRW(m_settingsTweakBar, "Constant Velocity", TW_TYPE_BOOLCPP, &constantVelocity, "group=Benchmark");
				TwAddVarCB(m_settingsTweakBar, "Scene", TW_TYPE_UINT32, sceneSetCallback, sceneGetCallback, m_level->m_cameras[0].get(), "group=Benchmark min=0 max=6");
			}
			

		}

		Engine::getInstance()->getWindow()->addInputListener(this);
		Engine::getInstance()->getWindow()->addResizeListener(this);
	}



	void Application::input(double time, double timeDelta)
	{
		if (!benchmarkIsRunning)
		{
			m_cameraController.input(time, timeDelta);
			freeze = UserInput::getInstance().isKeyPressed(InputKey::SPACE);
		}
		else
		{
			freeze = false;
		}
	}

	void Application::update(double time, double timeDelta)
	{
		UserInput &userInput = UserInput::getInstance();

		static double lastPressed = time;
		if (time - lastPressed > 0.1 && userInput.isKeyPressed(InputKey::LEFT_SHIFT) && userInput.isKeyPressed(InputKey::ENTER))
		{
			lastPressed = time;
			guiVisible = !guiVisible;
		}
	}

	void Application::render()
	{
		// timer query results
		extern double gtaoRenderTime;
		extern double gtaoSpatialDenoiseTime;
		extern double gtaoTemporalDenoiseTime;
		extern double hbaoRenderTime;
		extern double bilateralBlurRenderTime;
		extern double originalSsaoRenderTime;
		extern double ssaoRenderTime;
		extern double cocComputeTime;
		extern double seperateDofBlurComputeTime;
		extern double seperateDofCompositeComputeTime;
		extern double seperateDofFillComputeTime;
		extern double simpleDofCocBlurComputeTime;
		extern double simpleDofCompositeComputeTime;
		extern double simpleDofFillComputeTime;
		extern double spriteDofCompositeComputeTime;
		extern double cocNeighborTileMaxRenderTime;
		extern double cocTileMaxRenderTime;
		extern double spriteDofRenderTime;
		extern double simpleDofBlurComputeTime;
		extern double seperateDofDownsampleComputeTime;
		extern double velocityCorrectionComputeTime;
		extern double velocityTileMaxRenderTime;
		extern double velocityNeighborTileMaxRenderTime;
		extern double motionBlurRenderTime;

		memset(&m_currentFrameTimings, 0, sizeof(m_currentFrameTimings));
		m_currentFrameTimings.m_gtaoRenderTime = gtaoRenderTime;
		m_currentFrameTimings.m_gtaoSpatialDenoiseTime = gtaoSpatialDenoiseTime;
		m_currentFrameTimings.m_gtaoTemporalDenoiseTime = gtaoTemporalDenoiseTime;
		m_currentFrameTimings.m_hbaoRenderTime = hbaoRenderTime;
		m_currentFrameTimings.m_bilateralBlurRenderTime = bilateralBlurRenderTime;
		m_currentFrameTimings.m_originalSsaoRenderTime = originalSsaoRenderTime;
		m_currentFrameTimings.m_ssaoRenderTime = ssaoRenderTime;
		m_currentFrameTimings.m_cocComputeTime = cocComputeTime;
		m_currentFrameTimings.m_seperateDofBlurComputeTime = seperateDofBlurComputeTime;
		m_currentFrameTimings.m_seperateDofCompositeComputeTime = seperateDofCompositeComputeTime;
		m_currentFrameTimings.m_seperateDofFillComputeTime = seperateDofFillComputeTime;
		m_currentFrameTimings.m_simpleDofCocBlurComputeTime = simpleDofCocBlurComputeTime;
		m_currentFrameTimings.m_simpleDofCompositeComputeTime = simpleDofCompositeComputeTime;
		m_currentFrameTimings.m_simpleDofFillComputeTime = simpleDofFillComputeTime;
		m_currentFrameTimings.m_spriteDofCompositeComputeTime = spriteDofCompositeComputeTime;
		m_currentFrameTimings.m_cocNeighborTileMaxRenderTime = cocNeighborTileMaxRenderTime;
		m_currentFrameTimings.m_cocTileMaxRenderTime = cocTileMaxRenderTime;
		m_currentFrameTimings.m_spriteDofRenderTime = spriteDofRenderTime;
		m_currentFrameTimings.m_simpleDofBlurComputeTime = simpleDofBlurComputeTime;
		m_currentFrameTimings.m_seperateDofDownsampleComputeTime = seperateDofDownsampleComputeTime;
		m_currentFrameTimings.m_velocityCorrectionComputeTime = velocityCorrectionComputeTime;
		m_currentFrameTimings.m_velocityTileMaxRenderTime = velocityTileMaxRenderTime;
		m_currentFrameTimings.m_velocityNeighborTileMaxRenderTime = velocityNeighborTileMaxRenderTime;
		m_currentFrameTimings.m_motionBlurRenderTime = motionBlurRenderTime;
		m_currentFrameTimings.m_gtaoSum = (gtaoRenderTime + gtaoSpatialDenoiseTime + gtaoTemporalDenoiseTime) * (ambientOcclusion->get() == int(AmbientOcclusion::GTAO));
		m_currentFrameTimings.m_hbaoSum = (hbaoRenderTime + bilateralBlurRenderTime) * (ambientOcclusion->get() == int(AmbientOcclusion::HBAO));
		m_currentFrameTimings.m_ssaoSum = (ssaoRenderTime + bilateralBlurRenderTime) * (ambientOcclusion->get() == int(AmbientOcclusion::SSAO));
		m_currentFrameTimings.m_ssaoOriginalSum = (originalSsaoRenderTime + bilateralBlurRenderTime) * (ambientOcclusion->get() == int(AmbientOcclusion::SSAO_ORIGINAL));
		m_currentFrameTimings.m_simpleDofSum = (cocComputeTime + simpleDofCocBlurComputeTime + simpleDofBlurComputeTime + simpleDofFillComputeTime + simpleDofCompositeComputeTime)
			* (depthOfField->get() == int(DepthOfField::SIMPLE));
		m_currentFrameTimings.m_spriteDofSum = (cocComputeTime + spriteDofRenderTime + spriteDofCompositeComputeTime) * (depthOfField->get() == int(DepthOfField::SPRITE_BASED));
		m_currentFrameTimings.m_tiledDofSum = (cocComputeTime + cocTileMaxRenderTime + cocNeighborTileMaxRenderTime + seperateDofDownsampleComputeTime + seperateDofBlurComputeTime
			+ seperateDofFillComputeTime + seperateDofCompositeComputeTime)
			* (depthOfField->get() == int(DepthOfField::TILE_BASED));
		m_currentFrameTimings.m_simpleMbSum = (velocityCorrectionComputeTime + motionBlurRenderTime) * (motionBlur->get() == int(MotionBlur::SIMPLE));
		m_currentFrameTimings.m_singleDirectionMbSum = (velocityCorrectionComputeTime + velocityTileMaxRenderTime + velocityNeighborTileMaxRenderTime + motionBlurRenderTime) * (motionBlur->get() == int(MotionBlur::TILE_BASED_SINGLE));
		m_currentFrameTimings.m_multiDirectionMbSum = (velocityCorrectionComputeTime + velocityTileMaxRenderTime + velocityNeighborTileMaxRenderTime + motionBlurRenderTime) * (motionBlur->get() == int(MotionBlur::TILE_BASED_MULTI));

		const unsigned int MAX_BENCHMARK_FRAMES = 200;
		const unsigned int MAX_BENCHMARK_PASSES = 4;

		// benchmark is currently running
		if (benchmarkIsRunning && benchmarkFrameCount < MAX_BENCHMARK_FRAMES)
		{
			// first frame of current pass
			if (benchmarkFrameCount == 0)
			{
				memset(&benchmarkTimings, 0, sizeof(benchmarkTimings));

				// first pass; remember previous settings
				if (benchmarkPass == 0)
				{
					previousMb = motionBlur->get();
					previousDof = depthOfField->get();
					previousSsao = ambientOcclusion->get();
				}

				switch (benchmarkPass)
				{
				case 0:
				case 1:
				case 2:
					motionBlur->set(benchmarkPass + 1);
					depthOfField->set(benchmarkPass + 1);
					ambientOcclusion->set(benchmarkPass + 1);
					break;
				case 3:
					ambientOcclusion->set(4);
					break;
				default:
					break;
				}

				std::cout << "starting pass " << benchmarkPass << std::endl;
				std::cout << "mb " << motionBlur->get() << std::endl;
				std::cout << "dof " << depthOfField->get() << std::endl;
				std::cout << "ssao " << ambientOcclusion->get() << std::endl;
			}
			++benchmarkFrameCount;
			benchmarkTimings += m_currentFrameTimings;
		}
		// we reached the end of a pass
		else if (benchmarkIsRunning && benchmarkFrameCount == MAX_BENCHMARK_FRAMES && benchmarkPass < MAX_BENCHMARK_PASSES)
		{
			benchmarkTimings *= (1.0 / MAX_BENCHMARK_FRAMES);

			std::stringstream ss;
			ss << "\n\nPass " << benchmarkPass << std::endl
				<< "Average Timings over " << MAX_BENCHMARK_FRAMES << " frames\n" << std::endl;

			switch (benchmarkPass)
			{
			case 0:
				ss << "\nMotion Blur:" << std::endl

					<< "Simple Motion Blur Total:									" << benchmarkTimings.m_simpleMbSum << std::endl
					<< "Simple Motion Blur Velocity Correction:						" << benchmarkTimings.m_velocityCorrectionComputeTime << std::endl
					<< "Simple Motion Blur Render:									" << benchmarkTimings.m_motionBlurRenderTime << std::endl

					<< "\nDepth of Field:" << std::endl

					<< "Simple Depth of Field Total:								" << benchmarkTimings.m_simpleDofSum << std::endl
					<< "Simple Depth of Field CoC Computation:						" << benchmarkTimings.m_cocComputeTime << std::endl
					<< "Simple Depth of Field CoC Blur:								" << benchmarkTimings.m_simpleDofCocBlurComputeTime << std::endl
					<< "Simple Depth of Field Blur:									" << benchmarkTimings.m_simpleDofBlurComputeTime << std::endl
					<< "Simple Depth of Field Composite:							" << benchmarkTimings.m_simpleDofCompositeComputeTime << std::endl

					<< "\nScreen Space Ambient Occlusion:" << std::endl

					<< "SSAO (Original) Total:										" << benchmarkTimings.m_ssaoOriginalSum << std::endl
					<< "SSAO (Original) Render:										" << benchmarkTimings.m_originalSsaoRenderTime << std::endl
					<< "SSAO (Original) Blur:										" << benchmarkTimings.m_bilateralBlurRenderTime << std::endl;
				break;
			case 1:
				ss << "\nMotion Blur:" << std::endl

					<< "Single Direction Motion Blur Total:							" << benchmarkTimings.m_singleDirectionMbSum << std::endl
					<< "Single Direction Motion Blur Velocity Correction:			" << benchmarkTimings.m_velocityCorrectionComputeTime << std::endl
					<< "Single Direction Motion Blur Velocity Tile Max:				" << benchmarkTimings.m_velocityTileMaxRenderTime << std::endl
					<< "Single Direction Motion Blur Velocity Neighbor Tile Max:	" << benchmarkTimings.m_velocityNeighborTileMaxRenderTime << std::endl
					<< "Single Direction Motion Blur Render:						" << benchmarkTimings.m_motionBlurRenderTime << std::endl

					<< "\nDepth of Field:" << std::endl

					<< "Sprite Depth of Field Total:								" << benchmarkTimings.m_spriteDofSum << std::endl
					<< "Sprite Depth of Field CoC Computation:						" << benchmarkTimings.m_cocComputeTime << std::endl
					<< "Sprite Depth of Field Render:								" << benchmarkTimings.m_spriteDofRenderTime << std::endl
					<< "Sprite Depth of Field Composite:							" << benchmarkTimings.m_spriteDofCompositeComputeTime << std::endl

					<< "\nScreen Space Ambient Occlusion:" << std::endl

					<< "SSAO Total:													" << benchmarkTimings.m_ssaoSum << std::endl
					<< "SSAO Render:												" << benchmarkTimings.m_ssaoRenderTime << std::endl
					<< "SSAO Blur:													" << benchmarkTimings.m_bilateralBlurRenderTime << std::endl;
				break;
			case 2:
				ss << "\nMotion Blur:" << std::endl

					<< "Multi Direction Motion Blur Total:							" << benchmarkTimings.m_multiDirectionMbSum << std::endl
					<< "Multi Direction Motion Blur Velocity Correction:			" << benchmarkTimings.m_velocityCorrectionComputeTime << std::endl
					<< "Multi Direction Motion Blur Velocity Tile Max:				" << benchmarkTimings.m_velocityTileMaxRenderTime << std::endl
					<< "Multi Direction Motion Blur Velocity Neighbor Tile Max:		" << benchmarkTimings.m_velocityNeighborTileMaxRenderTime << std::endl
					<< "Multi Direction Motion Blur Render:							" << benchmarkTimings.m_motionBlurRenderTime << std::endl

					<< "\nDepth of Field:" << std::endl

					<< "Tiled Depth of Field Total:									" << benchmarkTimings.m_tiledDofSum << std::endl
					<< "Tiled Depth of Field CoC Computation:						" << benchmarkTimings.m_cocComputeTime << std::endl
					<< "Tiled Depth of Field CoC Tile Max:							" << benchmarkTimings.m_cocTileMaxRenderTime << std::endl
					<< "Tiled Depth of Field CoC Neighbor Tile Max:					" << benchmarkTimings.m_cocNeighborTileMaxRenderTime << std::endl
					<< "Tiled Depth of Field Downsample:							" << benchmarkTimings.m_seperateDofDownsampleComputeTime << std::endl
					<< "Tiled Depth of Field Blur:									" << benchmarkTimings.m_seperateDofBlurComputeTime << std::endl
					<< "Tiled Depth of Field Fill:									" << benchmarkTimings.m_seperateDofFillComputeTime << std::endl
					<< "Tiled Depth of Field Composite:								" << benchmarkTimings.m_seperateDofCompositeComputeTime << std::endl

					<< "\nScreen Space Ambient Occlusion:" << std::endl

					<< "HBAO Total:													" << benchmarkTimings.m_hbaoSum << std::endl
					<< "HBAO Render:												" << benchmarkTimings.m_hbaoRenderTime << std::endl
					<< "HBAO Blur:													" << benchmarkTimings.m_bilateralBlurRenderTime << std::endl;
				break;
			case 3:
				ss << "\nScreen Space Ambient Occlusion:" << std::endl

					<< "GTAO Total:													" << benchmarkTimings.m_gtaoSum << std::endl
					<< "GTAO Render:												" << benchmarkTimings.m_gtaoRenderTime << std::endl
					<< "GTAO Spatial Denoise:										" << benchmarkTimings.m_gtaoSpatialDenoiseTime << std::endl
					<< "GTAO Temporal Denoise:										" << benchmarkTimings.m_gtaoTemporalDenoiseTime << std::endl;
				break;
			default:
				assert(false);
				break;
			}

			if (benchmarkPass == 0)
			{
				benchmarkFilepath = "Benchmark" + Utility::getFormatedTime() + ".txt";
			}
			
			std::ofstream outfile(benchmarkFilepath, std::ios::app);
			outfile << ss.str();
			outfile.flush();
			outfile.close();

			std::cout << "wrote pass " << benchmarkPass << std::endl;

			++benchmarkPass;
			benchmarkFrameCount = 0;
		}
		// end benchmark; reset settings
		else if (benchmarkIsRunning && benchmarkFrameCount == MAX_BENCHMARK_FRAMES && benchmarkPass == MAX_BENCHMARK_PASSES)
		{
			benchmarkIsRunning = false;
			benchmarkFrameCount = 0;
			benchmarkPass = 0;
			motionBlur->set(previousMb);
			depthOfField->set(previousDof);
			ambientOcclusion->set(previousSsao);
		}


		static double lastMeasure = Engine::getTime();
		static double frameTimeSum = 0.0;
		static double fpsSum;
		static double worstFrameTime = 0.0;
		static double worstFps = std::numeric_limits<double>::max();
		static double frameTimeAvg = 0.0;
		static double fpsAvg = 0.0;
		static long long countedFrames = 0;

		double time = Engine::getTime();
		double timeDelta = Engine::getTimeDelta();
		double fps = Engine::getFps();

		if (time - lastMeasure >= 1.0)
		{
			frameTimeAvg = frameTimeSum / countedFrames;
			fpsAvg = fpsSum / countedFrames;
			lastMeasure = time;
			frameTimeSum = 0.0;
			fpsSum = 0.0;
			worstFrameTime = 0.0;
			worstFps = std::numeric_limits<double>::max();
			countedFrames = 0;
		}

		++countedFrames;
		frameTimeSum += timeDelta;
		fpsSum += fps;
		worstFrameTime = glm::max(worstFrameTime, timeDelta);
		worstFps = glm::min(worstFps, fps);

		fpsStr = std::to_string(fps).substr(0, 6);
		fpsAvgStr = std::to_string(fpsAvg).substr(0, 6);
		fpsWorstStr = std::to_string(worstFps).substr(0, 6);
		frameTimeStr = std::to_string(timeDelta * 1000.0).substr(0, 6);
		frameTimeAvgStr = std::to_string(frameTimeAvg * 1000.0).substr(0, 6);
		frameTimeWorstStr = std::to_string(worstFrameTime * 1000.0).substr(0, 6);

		if (guiVisible)
		{
			TwDraw();
		}
	}

	void Application::onKey(int _key, int _action)
	{
		TwEventKeyGLFW(_key, _action);
	}

	void Application::onChar(int _charKey)
	{
		TwEventCharGLFW(_charKey, 1);
	}

	void Application::onMouseButton(int _mouseButton, int _action)
	{
		TwEventMouseButtonGLFW(_mouseButton, _action);
	}

	void Application::onMouseMove(double _x, double _y)
	{
		TwEventMousePosGLFW((int)_x, (int)_y);
	}

	void Application::onMouseEnter(bool _entered)
	{
	}

	void Application::onMouseScroll(double _xOffset, double _yOffset)
	{
		scrollOffset += _yOffset;
		TwEventMouseWheelGLFW((int)scrollOffset);
	}

	void Application::gamepadUpdate(const std::vector<Gamepad>* _gamepads)
	{
	}

	void Application::onResize(unsigned int width, unsigned int height)
	{
		TwWindowSize(width, height);
	}
}