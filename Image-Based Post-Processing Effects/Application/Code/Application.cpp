#define NK_IMPLEMENTATION
#include "Application.h"
#include "Level.h"
#include <EntityComponentSystem\SystemManager.h>
#include <Gui\GuiJSONParser.h>
#include <Gui\nuklearInclude.h>
#include <Gui\GuiLayout.h>
#include <Gui\GuiEvent.h>
#include <Engine.h>
#include <Window.h>
#include <random>
#include <EasingFunctions.h>

namespace App
{
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
			lensFlaresEnabled = settingsManager.getBoolSetting("graphics", "lens_flares_enabled", false);
			ambientOcclusion = settingsManager.getIntSetting("graphics", "ambient_occlusion", 0);
			vsync = settingsManager.getBoolSetting("graphics", "vsync", false);
			windowWidth = settingsManager.getIntSetting("graphics", "window_width", 1080);
			windowHeight = settingsManager.getIntSetting("graphics", "window_height", 720);
			windowMode = settingsManager.getIntSetting("graphics", "window_mode", 0);
			motionBlur = settingsManager.getIntSetting("graphics", "motion_blur", 0);
			ssaoKernelSize = settingsManager.getIntSetting("graphics", "ssao_kernel_size", 16);
			ssaoRadius = settingsManager.getDoubleSetting("graphics", "ssao_radius", 0.5);
			ssaoStrength = settingsManager.getDoubleSetting("graphics", "ssao_strength", 1.0);
			hbaoDirections = settingsManager.getIntSetting("graphics", "hbao_directions", 4);
			hbaoSteps = settingsManager.getIntSetting("graphics", "hbao_steps", 4);
			hbaoStrength = settingsManager.getDoubleSetting("graphics", "hbao_strength", 0.5);
			hbaoRadius = settingsManager.getDoubleSetting("graphics", "hbao_radius", 0.3);
			hbaoMaxRadiusPixels = settingsManager.getDoubleSetting("graphics", "hbao_max_radius_pixels", 50.0);
			hbaoAngleBias = settingsManager.getDoubleSetting("graphics", "hbao_angle_bias", glm::tan(glm::radians(30.0f)));

			settingsManager.saveToIni();
		}


		level = loadLevel();
		SystemManager::getInstance().setLevel(level);
		cameraController.setCamera(level->cameras[level->activeCameraIndex]);

		gui.init();
		GuiStyleSheet style = GuiJSONParser::parseStyleJSONFile("Resources/Gui/stylesheet.sts");
		gui.setStyleSheet(style);

		optionsGui = std::unique_ptr<GuiLayout>(GuiJSONParser::parseLayoutJSONFile("Resources/Gui/settings.lyt"));
		optionsGui->init();
		optionsGui->setEventListener(this);
		optionsGui->setDisabled(false);

		gui.setLayout(optionsGui.get());

		//SettingsManager::getInstance().saveToIni();

		initGuiData();

		optionsGui->getElementById<GuiWindow>("settings_window")->setFlag(NK_WINDOW_MINIMIZABLE, true);

		// physics
		{
			btBroadphaseInterface* broadphase = new btDbvtBroadphase();

			btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
			btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

			btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

			dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

			dynamicsWorld->setGravity(btVector3(0, -10, 0));


			btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);

			btCollisionShape* fallShape = new btSphereShape(1);


			btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
			btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
			btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
			groundRigidBody->setRestitution(1.0);
			dynamicsWorld->addRigidBody(groundRigidBody);

			for (int i = 0; i < 2000; ++i)
			{
				const Entity *sphere = level->entityMap["sphere" + std::to_string(i)];
				glm::vec3 pos = EntityManager::getInstance().getComponent<TransformationComponent>(sphere)->position;
				btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(pos.x, pos.y, pos.z)));
				btScalar mass = 1;
				btVector3 fallInertia(0, 0, 0);
				fallShape->calculateLocalInertia(mass, fallInertia);
				btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);

				fallRigidBody[i] = new btRigidBody(fallRigidBodyCI);
				fallRigidBody[i]->setRestitution(1.0);
				dynamicsWorld->addRigidBody(fallRigidBody[i]);
			}

		}
	}

	void Application::input(double currentTime, double timeDelta)
	{
		cameraController.input(currentTime, timeDelta);
		gui.input();
	}

	void Application::update(double currentTime, double timeDelta)
	{
		dynamicsWorld->stepSimulation(timeDelta, 10);

		btTransform trans;
		for (int i = 0; i < 2000; ++i)
		{
			fallRigidBody[i]->getMotionState()->getWorldTransform(trans);
			const Entity *sphere = level->entityMap["sphere"+std::to_string(i)];
			auto *tc = EntityManager::getInstance().getComponent<TransformationComponent>(sphere);
			tc->position = glm::vec3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ());
			tc->rotation = glm::quat(trans.getRotation().getW(), trans.getRotation().getX(), trans.getRotation().getY(), trans.getRotation().getZ());
		}
		
	}

	void Application::render()
	{
		optionsGui->getElementById<GuiLabel>("fps_label")->setText(std::to_string(Engine::getCurrentFps()));
		optionsGui->getElementById<GuiLabel>("frame_time_label")->setText(std::to_string(Engine::getCurrentTimeDelta() * 1000.0));
		gui.render();
	}

	void Application::guiEventNotification(GuiEvent & _event)
	{
		const char *id = _event.source->getId();

		SettingsManager &settingsManager = SettingsManager::getInstance();
		const int ON = 0, OFF = 1;

		GuiComboBox *box;
		GuiToggle *checkbox;
		GuiSlider *slider;

		if (optionsGui->getElementById("window_mode_box", box) && _event.source == box)
		{
			windowMode->set((int)box->getSelectedItem());
		}
		if (optionsGui->getElementById("resolution_box", box) && _event.source == box)
		{
			auto window = Engine::getInstance()->getWindow();
			auto resolutions = window->getSupportedResolutions();
			auto resolution = resolutions[box->getSelectedItem()];

			windowWidth->set((int)resolution.first);
			windowHeight->set((int)resolution.second);

		}
		if (optionsGui->getElementById("vsync_checkbox", checkbox) && _event.source == checkbox)
		{
			vsync->set(checkbox->isChecked());
		}
		if (optionsGui->getElementById("anti_aliasing_checkbox", checkbox) && _event.source == checkbox)
		{
			fxaaEnabled->set(checkbox->isChecked());
		}
		if (optionsGui->getElementById("anisotropic_filtering_box", box) && _event.source == box)
		{
			int aa = (int)glm::pow(2, box->getSelectedItem());
			anisotropicFiltering->set(aa);
		}
		if (optionsGui->getElementById("shadow_quality_box", box) && _event.source == box)
		{
			shadowQuality->set((int)box->getSelectedItem());
		}
		if (optionsGui->getElementById("lens_flare_checkbox", checkbox) && _event.source == checkbox)
		{
			lensFlaresEnabled->set(checkbox->isChecked());
		}
		if (optionsGui->getElementById("bloom_checkbox", checkbox) && _event.source == checkbox)
		{
			bloomEnabled->set(checkbox->isChecked());
		}
		if (optionsGui->getElementById("ambient_occlusion_box", box) && _event.source == box)
		{
			ambientOcclusion->set(static_cast<int>(box->getSelectedItem()));
		}
		if (optionsGui->getElementById("motion_blur_box", box) && _event.source == box)
		{
			motionBlur->set(box->getSelectedItem());
		}
		if (optionsGui->getElementById("bounce_checkbox", checkbox) && _event.source == checkbox)
		{
			bool bounce = checkbox->isChecked();
			EntityManager &entityManager = EntityManager::getInstance();
			std::default_random_engine e;
			std::uniform_real_distribution<float> dist(0.0f, 1.0f);
			std::uniform_real_distribution<float> dist1(1.0f, 3.0f);

			for (int i = 0; i < 10; ++i)
			{
				for (int j = 0; j < 10; ++j)
				{
					const Entity *entity = level->entityMap.at("teapot" + std::to_string(i * 10 + j));

					assert(entity);

					glm::vec3 position = glm::vec3(i - 5.0f, 0.0f, j - 5.0f) * 4.0f;

					if (bounce)
					{
						glm::vec3 bouncePos = position + glm::vec3(0.0f, 10.0f, 0.0f);
						float speed = dist1(e) * 0.5f;

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
						entityManager.addComponent<MovementPathComponent>(entity, pathSegments, Engine::getCurrentTime(), true);
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
		if (optionsGui->getElementById("colors_checkbox", checkbox) && _event.source == checkbox)
		{
			bool colors = checkbox->isChecked();
			EntityManager &entityManager = EntityManager::getInstance();
			std::default_random_engine e;
			std::uniform_real_distribution<float> dist(0.0f, 1.0f);
			std::uniform_real_distribution<float> dist1(1.0f, 3.0f);

			for (int i = 0; i < 10; ++i)
			{
				for (int j = 0; j < 10; ++j)
				{
					const Entity *entity = level->entityMap.at("teapot" + std::to_string(i * 10 + j));

					assert(entity);

					glm::vec3 color(1.0);

					if (colors)
					{
						color = glm::vec3(dist(e), dist(e), dist(e));
					}
					entityManager.getComponent<ModelComponent>(entity)->model[0].second.setAlbedo(glm::vec4(color, 1.0));
				}
			}
		}
		if (optionsGui->getElementById("ssao_kernel_size_slider", slider) && _event.source == slider)
		{
			ssaoKernelSize->set((int)slider->getValue());
			optionsGui->getElementById<GuiLabel>("ssao_kernel_size_label")->setText(std::to_string(ssaoKernelSize->get()));
		}
		if (optionsGui->getElementById("ssao_radius_slider", slider) && _event.source == slider)
		{
			ssaoRadius->set(slider->getValue());
			optionsGui->getElementById<GuiLabel>("ssao_radius_label")->setText(std::to_string(ssaoRadius->get()));
		}
		if (optionsGui->getElementById("ssao_strength_slider", slider) && _event.source == slider)
		{
			ssaoStrength->set(slider->getValue());
			optionsGui->getElementById<GuiLabel>("ssao_strength_label")->setText(std::to_string(ssaoStrength->get()));
		}
		if (optionsGui->getElementById("hbao_directions_slider", slider) && _event.source == slider)
		{
			hbaoDirections->set((int)slider->getValue());
			optionsGui->getElementById<GuiLabel>("hbao_directions_label")->setText(std::to_string(hbaoDirections->get()));
		}
		if (optionsGui->getElementById("hbao_steps_slider", slider) && _event.source == slider)
		{
			hbaoSteps->set((int)slider->getValue());
			optionsGui->getElementById<GuiLabel>("hbao_steps_label")->setText(std::to_string(hbaoSteps->get()));
		}
		if (optionsGui->getElementById("hbao_strength_slider", slider) && _event.source == slider)
		{
			hbaoStrength->set(slider->getValue());
			optionsGui->getElementById<GuiLabel>("hbao_strength_label")->setText(std::to_string(hbaoStrength->get()));
		}
		if (optionsGui->getElementById("hbao_radius_slider", slider) && _event.source == slider)
		{
			hbaoRadius->set(slider->getValue());
			optionsGui->getElementById<GuiLabel>("hbao_radius_label")->setText(std::to_string(hbaoRadius->get()));
		}
		if (optionsGui->getElementById("hbao_max_radius_pixels_slider", slider) && _event.source == slider)
		{
			hbaoMaxRadiusPixels->set(slider->getValue());
			optionsGui->getElementById<GuiLabel>("hbao_max_radius_pixels_label")->setText(std::to_string(hbaoMaxRadiusPixels->get()));
		}
		if (optionsGui->getElementById("hbao_angle_bias_slider", slider) && _event.source == slider)
		{
			hbaoAngleBias->set(slider->getValue());
			optionsGui->getElementById<GuiLabel>("hbao_angle_bias_label")->setText(std::to_string(hbaoAngleBias->get()));
		}

		settingsManager.saveToIni();
	}


	void Application::initGuiData()
	{
		SettingsManager &settingsManager = SettingsManager::getInstance();
		const int ON = 0, OFF = 1;


		GuiComboBox *box;
		GuiToggle *checkbox;
		GuiSlider *slider;

		if (optionsGui->getElementById("window_mode_box", box))
		{
			box->setSelectedItem(static_cast<size_t>(windowMode->get()));
		}
		if (optionsGui->getElementById("resolution_box", box))
		{
			auto window = Engine::getInstance()->getWindow();
			auto resolutions = window->getSupportedResolutions();
			size_t resolutionsCount = resolutions.size();
			auto selectedResolution = window->getSelectedResolution();
			assert(!resolutions.empty());

			std::vector<std::string> resolutionStrings;
			size_t selectedIndex = 0;
			for (size_t i = 0; i < resolutionsCount; ++i)
			{
				const unsigned int w = resolutions[i].first;
				const unsigned int h = resolutions[i].second;
				resolutionStrings.push_back(std::to_string(w) + " x " + std::to_string(h));
				if (w == selectedResolution.first && h == selectedResolution.second)
				{
					selectedIndex = i;
				}
			}

			box->setItems(resolutionStrings);
			box->setSelectedItem(selectedIndex);
		}
		if (optionsGui->getElementById("vsync_checkbox", checkbox))
		{
			checkbox->setChecked(vsync->get());
		}
		if (optionsGui->getElementById("anti_aliasing_checkbox", checkbox))
		{
			checkbox->setChecked(fxaaEnabled->get());
		}
		if (optionsGui->getElementById("anisotropic_filtering_box", box))
		{
			int aa = Engine::getInstance()->getMaxAnisotropicFiltering();
			std::vector<std::string> aaStrings;
			aaStrings.push_back("Off");

			for (int i = 2; i <= aa; i *= 2)
			{
				aaStrings.push_back("x" + std::to_string(i));
			}

			int af = anisotropicFiltering->get();
			size_t index = (size_t)log2(af);
			box->setItems(aaStrings);
			box->setSelectedItem(index);
		}
		if (optionsGui->getElementById("shadow_quality_box", box))
		{
			box->setSelectedItem(static_cast<size_t>(shadowQuality->get()));
		}
		if (optionsGui->getElementById("lens_flare_checkbox", checkbox))
		{
			checkbox->setChecked(lensFlaresEnabled->get());
		}
		if (optionsGui->getElementById("bloom_checkbox", checkbox))
		{
			checkbox->setChecked(bloomEnabled->get());
		}
		if (optionsGui->getElementById("ambient_occlusion_checkbox", checkbox))
		{
			checkbox->setChecked(ambientOcclusion->get());
		}
		if (optionsGui->getElementById("motion_blur_box", box))
		{
			box->setSelectedItem(static_cast<size_t>(motionBlur->get()));
		}
		if (optionsGui->getElementById("ambient_occlusion_box", box))
		{
			box->setSelectedItem(static_cast<size_t>(ambientOcclusion->get()));
		}
		if (optionsGui->getElementById("ssao_kernel_size_slider", slider))
		{
			slider->setValue(static_cast<float>(ssaoKernelSize->get()));
		}
		if (optionsGui->getElementById("ssao_radius_slider", slider))
		{
			slider->setValue(static_cast<float>(ssaoRadius->get()));
		}
		if (optionsGui->getElementById("hbao_directions_slider", slider))
		{
			slider->setValue(static_cast<float>(hbaoDirections->get()));
		}
		if (optionsGui->getElementById("hbao_steps_slider", slider))
		{
			slider->setValue(static_cast<float>(hbaoSteps->get()));
		}
		if (optionsGui->getElementById("hbao_strength_slider", slider))
		{
			slider->setValue(static_cast<float>(hbaoStrength->get()));
		}
		if (optionsGui->getElementById("hbao_radius_slider", slider))
		{
			slider->setValue(static_cast<float>(hbaoRadius->get()));
		}
		if (optionsGui->getElementById("hbao_max_radius_pixels_slider", slider))
		{
			slider->setValue(static_cast<float>(hbaoMaxRadiusPixels->get()));
		}
		if (optionsGui->getElementById("hbao_angle_bias_slider", slider))
		{
			slider->setValue(static_cast<float>(hbaoAngleBias->get()));
		}
		optionsGui->getElementById<GuiLabel>("ssao_radius_label")->setText(std::to_string(ssaoRadius->get()));
		optionsGui->getElementById<GuiLabel>("ssao_kernel_size_label")->setText(std::to_string(ssaoKernelSize->get()));
		optionsGui->getElementById<GuiLabel>("ssao_strength_label")->setText(std::to_string(ssaoStrength->get()));
		optionsGui->getElementById<GuiLabel>("hbao_directions_label")->setText(std::to_string(hbaoDirections->get()));
		optionsGui->getElementById<GuiLabel>("hbao_steps_label")->setText(std::to_string(hbaoSteps->get()));
		optionsGui->getElementById<GuiLabel>("hbao_strength_label")->setText(std::to_string(hbaoStrength->get()));
		optionsGui->getElementById<GuiLabel>("hbao_radius_label")->setText(std::to_string(hbaoRadius->get()));
		optionsGui->getElementById<GuiLabel>("hbao_max_radius_pixels_label")->setText(std::to_string(hbaoMaxRadiusPixels->get()));
		optionsGui->getElementById<GuiLabel>("hbao_angle_bias_label")->setText(std::to_string(hbaoAngleBias->get()));

	}
}