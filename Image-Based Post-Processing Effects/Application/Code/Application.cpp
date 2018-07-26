#define NK_IMPLEMENTATION
#include "Application.h"
#include "Levels.h"
#include <EntityComponentSystem\SystemManager.h>
#include <EntityComponentSystem\EntityManager.h>
#include <Engine.h>
#include <Window\Window.h>
#include <random>
#include <EasingFunctions.h>
#include <Input\UserInput.h>
#include <Input\Gamepad.h>
#include <Graphics\Effects.h>

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
		COMBINED_FUNC_DEF(screenSpaceReflectionsEnabled, bool)

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

	void TW_CALL colorsGetCallback(void *value, void *clientData)
	{
		*(bool *)value = ((App::Application *)clientData)->colors;
	}

	void TW_CALL colorsSetCallback(const void *value, void *clientData)
	{
		App::Application *app = ((App::Application *)clientData);
		app->colors = *(bool *)value;
		EntityManager &entityManager = EntityManager::getInstance();
		std::default_random_engine e;
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		std::uniform_real_distribution<float> dist1(1.0f, 3.0f);

		for (int i = 0; i < 10; ++i)
		{
			for (int j = 0; j < 10; ++j)
			{
				const Entity *entity = app->level->entityMap.at("teapot" + std::to_string(i * 10 + j));

				assert(entity);

				glm::vec3 color(1.0);

				if (app->colors)
				{
					color = glm::vec3(dist(e), dist(e), dist(e));
				}
				entityManager.getComponent<ModelComponent>(entity)->model[0].second.setAlbedo(glm::vec4(color, 1.0));
			}
		}
	}

	void TW_CALL bouncingGetCallback(void *value, void *clientData)
	{
		*(bool *)value = ((App::Application *)clientData)->bouncing;
	}

	void TW_CALL bouncingSetCallback(const void *value, void *clientData)
	{
		App::Application *app = ((App::Application *)clientData);
		app->bouncing = *(bool *)value;

		EntityManager &entityManager = EntityManager::getInstance();
		std::default_random_engine e;
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		std::uniform_real_distribution<float> dist1(1.0f, 3.0f);

		for (int i = 0; i < 10; ++i)
		{
			for (int j = 0; j < 10; ++j)
			{
				const Entity *entity = app->level->entityMap.at("teapot" + std::to_string(i * 10 + j));

				assert(entity);

				glm::vec3 position = glm::vec3(i - 5.0f, 0.0f, j - 5.0f) * 0.2f;

				if (app->bouncing)
				{
					glm::vec3 bouncePos = position + glm::vec3(0.0f, 3.0f, 0.0f);
					float speed = dist1(e);

					std::vector<PathSegment> pathSegments;
					pathSegments.push_back(PathSegment(
						position,	// start pos
						bouncePos,	// end pos
						glm::vec3(0.0f, 1.0f, 0.0f),						// start tangent
						glm::vec3(0.0f, 1.0f, 0.0f),						// end tangent
						speed,														// duration
						linear));													// easing function
					pathSegments.push_back(PathSegment(
						bouncePos,
						position,
						glm::vec3(0.0f, -1.0f, 0.0),
						glm::vec3(0.0f, -1.0f, 0.0),
						speed,
						linear));
					entityManager.addComponent<MovementPathComponent>(entity, pathSegments, Engine::getTime(), true);
					entityManager.addComponent<PerpetualRotationComponent>(entity, glm::vec3(dist(e), dist(e), dist(e)));
				}
				else
				{
					entityManager.removeComponent<MovementPathComponent>(entity);
					entityManager.removeComponent<PerpetualRotationComponent>(entity);
					TransformationComponent *tc = entityManager.getComponent<TransformationComponent>(entity);
					tc->position = position;
					tc->rotation = glm::quat(glm::vec3(0.0, glm::radians(40.0f), 0.0f));
				}

			}
		}
	}

	Application::Application()
		:cameraController()
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
			screenSpaceReflectionsEnabled = settingsManager.getBoolSetting("graphics", "screen_space_reflections_enabled", false);

			settingsManager.saveToIni();
		}


		level = loadSponzaLevel();
		SystemManager::getInstance().setLevel(level);
		cameraController.setCamera(level->cameras[level->activeCameraIndex]);

		TwInit(TW_OPENGL_CORE, NULL); // for core profile
		TwWindowSize(windowWidth->get(), windowHeight->get());
		settingsTweakBar = TwNewBar("Settings");
		TwDefine("Settings refresh=0.49");

		{
			// timings
			{
				TwAddVarRO(settingsTweakBar, "FPS", TW_TYPE_STDSTRING, &fpsStr, "group=Timings");
				TwAddVarRO(settingsTweakBar, "FPS Average", TW_TYPE_STDSTRING, &fpsAvgStr, "group=Timings");
				TwAddVarRO(settingsTweakBar, "FPS Worst", TW_TYPE_STDSTRING, &fpsWorstStr, "group=Timings");
				TwAddVarRO(settingsTweakBar, "Frame Time", TW_TYPE_STDSTRING, &frameTimeStr, "group=Timings");
				TwAddVarRO(settingsTweakBar, "Frame Time Average", TW_TYPE_STDSTRING, &frameTimeAvgStr, "group=Timings");
				TwAddVarRO(settingsTweakBar, "Frame Time Worst", TW_TYPE_STDSTRING, &frameTimeWorstStr, "group=Timings");
			}

			// window
			{
				// window mode
				{
					TwEnumVal windowOptions[] = { { (int)WindowMode::WINDOWED, "Windowed" },{ (int)WindowMode::BORDERLESS_FULLSCREEN, "Borderless Fullscreen" },{ (int)WindowMode::FULLSCREEN, "Fullscreen" } };
					TwType WindowModeTwType = TwDefineEnum("WindowModeType", windowOptions, 3);
					TwAddVarCB(settingsTweakBar, "Window Mode", WindowModeTwType, SETTER_FUNC_PTR(windowMode), GETTER_FUNC_PTR(windowMode), this, "group=Window");
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
						resolutionOptionStrings.push_back(std::to_string(w) + " x " + std::to_string(h));
					}

					// only call c_str() after vector is completely filled (don't touch it after this point or the char pointers will invalidate)
					// TODO: find better solution
					for (size_t i = 0; i < resolutionOptionStrings.size(); ++i)
					{
						const char *str = resolutionOptionStrings[i].c_str();
						resolutionOptions.push_back({ (int)i, str });
					}

					TwType ResolutionTwType = TwDefineEnum("ResolutionType", resolutionOptions.data(), resolutionOptions.size());
					TwAddVarCB(settingsTweakBar, "Window Resolution", ResolutionTwType, SETTER_FUNC_PTR(windowResolution), GETTER_FUNC_PTR(windowResolution), this, "group=Window");
				}

				// vsync
				TwAddVarCB(settingsTweakBar, "V-Sync", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(vsync), GETTER_FUNC_PTR(vsync), this, "group=Window");
			}

			// shadows
			{
				// shadow quality
				{
					TwEnumVal shadowOptions[] = { { (int)ShadowQuality::OFF, "Off" },{ (int)ShadowQuality::NORMAL, "Normal" } };
					TwType ShadowTwType = TwDefineEnum("ShadowType", shadowOptions, 2);
					TwAddVarCB(settingsTweakBar, "Shadow Quality", ShadowTwType, SETTER_FUNC_PTR(shadowQuality), GETTER_FUNC_PTR(shadowQuality), this, "group=Shadows");
				}
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
						afOptionStrings.push_back("x" + std::to_string(i));
					}

					// only call c_str() after vector is completely filled (don't touch it after this point or the char pointers will invalidate)
					// TODO: find better solution
					for (int i = 2, j = 0; i <= aa; i *= 2, ++j)
					{
						const char *str = afOptionStrings[j].c_str();
						afOptions.push_back({ i, str });
					}

					TwType DofTwType = TwDefineEnum("AfType", afOptions.data(), afOptions.size());
					TwAddVarCB(settingsTweakBar, "Anisotropic Filtering", DofTwType, SETTER_FUNC_PTR(anisotropicFiltering), GETTER_FUNC_PTR(anisotropicFiltering), this, "group=Anisotropic_Filtering");
				}
			}

			// anti aliasing
			{
				TwAddVarCB(settingsTweakBar, "FXAA", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(fxaaEnabled), GETTER_FUNC_PTR(fxaaEnabled), this, "group=Anti-Aliasing");
				TwAddVarCB(settingsTweakBar, "SMAA", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(smaaEnabled), GETTER_FUNC_PTR(smaaEnabled), this, "group=Anti-Aliasing");
				TwAddVarCB(settingsTweakBar, "SMAA Temporal AA", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(smaaTemporalAA), GETTER_FUNC_PTR(smaaTemporalAA), this, "group=Anti-Aliasing");
			}

			// lens
			{
				TwAddVarCB(settingsTweakBar, "Lens Flares", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(lensFlaresEnabled), GETTER_FUNC_PTR(lensFlaresEnabled), this, "group=Lens");
				TwAddVarCB(settingsTweakBar, "Bloom", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(bloomEnabled), GETTER_FUNC_PTR(bloomEnabled), this, "group=Lens ");
			}

			// reflections
			{
				TwAddVarCB(settingsTweakBar, "Screen Space Reflections", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(screenSpaceReflectionsEnabled), GETTER_FUNC_PTR(screenSpaceReflectionsEnabled), this, "group=Reflections");
			}

			// depth of field
			{
				TwEnumVal dofOptions[] = { { (int)DepthOfField::OFF, "Off" },{ (int)DepthOfField::SIMPLE, "Simple" },{ (int)DepthOfField::SPRITE_BASED, "Sprite Based" },{ (int)DepthOfField::TILE_BASED_SEPERATE, "Tile-Based Seperate" },{ (int)DepthOfField::TILE_BASED_COMBINED, "Tile-Based Combined" } };
				TwType DofTwType = TwDefineEnum("DepthOfFieldType", dofOptions, 5);
				TwAddVarCB(settingsTweakBar, "Depth of Field", DofTwType, SETTER_FUNC_PTR(depthOfField), GETTER_FUNC_PTR(depthOfField), this, "group=Depth_of_Field");
			}

			// motion blur
			{
				TwEnumVal mbOptions[] = { { (int)MotionBlur::OFF, "Off" },{ (int)MotionBlur::SIMPLE, "Simple" },{ (int)MotionBlur::TILE_BASED_SINGLE, "Tile-Based Single Direction" },{ (int)MotionBlur::TILE_BASED_MULTI, "Tile-Based Multi Direction" } };
				TwType MbTwType = TwDefineEnum("MotionBlurType", mbOptions, 4);
				TwAddVarCB(settingsTweakBar, "Motion Blur", MbTwType, SETTER_FUNC_PTR(motionBlur), GETTER_FUNC_PTR(motionBlur), this, "group=Motion_Blur");
			}

			// ambient occlusion
			{
				TwEnumVal aoOptions[] = { { (int)AmbientOcclusion::OFF, "Off" }, { (int)AmbientOcclusion::SSAO_ORIGINAL, "SSAO (Original)" }, { (int)AmbientOcclusion::SSAO, "SSAO" }, { (int)AmbientOcclusion::HBAO, "HBAO" } };
				TwType AoTwType = TwDefineEnum("AoType", aoOptions, 4);
				TwAddVarCB(settingsTweakBar, "Ambient Occlusion", AoTwType, SETTER_FUNC_PTR(ambientOcclusion), GETTER_FUNC_PTR(ambientOcclusion), this, "group=Ambient_Occlusion");

				TwAddVarCB(settingsTweakBar, "SSAO Kernel Size", TW_TYPE_INT32, SETTER_FUNC_PTR(ssaoKernelSize), GETTER_FUNC_PTR(ssaoKernelSize), this, "group=Ambient_Occlusion min=1 max=64");
				TwAddVarCB(settingsTweakBar, "SSAO Radius", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(ssaoRadius), GETTER_FUNC_PTR(ssaoRadius), this, "group=Ambient_Occlusion min=0.1 max=10.0 step=0.1");
				TwAddVarCB(settingsTweakBar, "SSAO Strength", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(ssaoStrength), GETTER_FUNC_PTR(ssaoStrength), this, "group=Ambient_Occlusion min=0.1 max=10.0 step=0.1");
				TwAddVarCB(settingsTweakBar, "HBAO Directions", TW_TYPE_INT32, SETTER_FUNC_PTR(hbaoDirections), GETTER_FUNC_PTR(hbaoDirections), this, "group=Ambient_Occlusion min=1 max=32");
				TwAddVarCB(settingsTweakBar, "HBAO Steps", TW_TYPE_INT32, SETTER_FUNC_PTR(hbaoSteps), GETTER_FUNC_PTR(hbaoSteps), this, "group=Ambient_Occlusion min=1 max=32");
				TwAddVarCB(settingsTweakBar, "HBAO Strength", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(hbaoStrength), GETTER_FUNC_PTR(hbaoStrength), this, "group=Ambient_Occlusion min=0.1 max=10.0 step=0.1");
				TwAddVarCB(settingsTweakBar, "HBAO Radius", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(hbaoRadius), GETTER_FUNC_PTR(hbaoRadius), this, "group=Ambient_Occlusion min=0.1 max=10.0 step=0.1");
				TwAddVarCB(settingsTweakBar, "HBAO Max Radius Pixels", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(hbaoMaxRadiusPixels), GETTER_FUNC_PTR(hbaoMaxRadiusPixels), this, "group=Ambient_Occlusion min=1 max=256");
				TwAddVarCB(settingsTweakBar, "HBAO Angle Bias", TW_TYPE_DOUBLE, SETTER_FUNC_PTR(hbaoAngleBias), GETTER_FUNC_PTR(hbaoAngleBias), this, "group=Ambient_Occlusion min=0.0 max=1.5 step=0.01");
			}

			// scene
			{
				TwAddVarCB(settingsTweakBar, "Colors", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(colors), GETTER_FUNC_PTR(colors), this, "group=Scene");
				TwAddVarCB(settingsTweakBar, "Bouncing", TW_TYPE_BOOLCPP, SETTER_FUNC_PTR(bouncing), GETTER_FUNC_PTR(bouncing), this, "group=Scene");
			}
		}

		Engine::getInstance()->getWindow()->addInputListener(this);
		Engine::getInstance()->getWindow()->addResizeListener(this);
	}

	void Application::input(double time, double timeDelta)
	{
		cameraController.input(time, timeDelta);
	}

	void Application::update(double time, double timeDelta)
	{
		Gamepad &gamepad = UserInput::getInstance().getGamepad();
		if (UserInput::getInstance().isKeyPressed(InputKey::SPACE) || gamepad.id != -1 && gamepad.leftTrigger > -1.0f)
		{
			level->lights.spotLights[2]->setDirection(level->cameras[level->activeCameraIndex]->getForwardDirection());
			level->lights.spotLights[2]->setPosition(level->cameras[level->activeCameraIndex]->getPosition());
		}
	}

	void Application::render()
	{
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

		TwDraw();
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
		TwEventMouseWheelGLFW((int)_yOffset);
	}

	void Application::gamepadUpdate(const std::vector<Gamepad>* _gamepads)
	{
	}

	void Application::onResize(unsigned int width, unsigned int height)
	{
		TwWindowSize(width, height);
	}
}