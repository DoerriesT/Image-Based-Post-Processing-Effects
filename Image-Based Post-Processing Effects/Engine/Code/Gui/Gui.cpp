#include "Gui.h"
#include "nuklearInclude.h"
#include <iostream>
#include <rapidjson\rapidjson.h>
#include <rapidjson\document.h>
#include "GuiEvent.h"
#include "UserInput.h"
#include "overview.h"
#include "Elements\GuiWindow.h"
#include "..\EntityComponentSystem\Systems\RenderSystem.h"
#include "..\EntityComponentSystem\SystemManager.h"
#include "Window.h"

const float Gui::fontSizes[] = { 10, 18, 25, 32, 50 };
const float Gui::rowHeights[] = { 12, 22, 30, 38, 60 };
GuiDimensions Gui::dimensions;

Gui* Gui::instance = nullptr;
GuiElement* Gui::keyboardFocus = nullptr;

Gui::Gui()
	: renderer(), inputHandler(), fontManager(),
	currentLayout(nullptr), showOverview(false)
{
	assert(!instance);
	instance = this;

	setStyleClass("*");

	soundEntity = EntityManager::getInstance().createEntity();
}


Gui::~Gui()
{
	clearStyleClass();
	styleSheet.clear();

	EntityManager::getInstance().destroyEntity(soundEntity);
}

void Gui::init()
{
	SettingsManager &settingsManager = SettingsManager::getInstance();
	windowWidth = settingsManager.getIntSetting("graphics", "window_width", 1080);
	windowWidth->addListener([&](int _value) { updateDimensions(); });

	windowHeight = settingsManager.getIntSetting("graphics", "window_height", 720);
	windowHeight->addListener([&](int _value) { updateDimensions(); });

	windowMode = settingsManager.getIntSetting("graphics", "window_mode", 0);
	windowMode->addListener([&](int _value) { updateDimensions(); });

	uiSizeOffset = SettingsManager::getInstance().getIntSetting("graphics", "gui_size", 0);
	uiSizeOffset->addListener([this](int ) { updateDimensions(); });

	renderer.init();
	inputHandler.init();
	fontManager.init(&renderer);

	overviewCtx = new nk_context();
	nk_init_default(overviewCtx, fontManager.getFont(size));
}


void Gui::setLayout(GuiLayout *layout)
{
	if (currentLayout == layout)
		return;

	input();

	currentLayout = layout;

	if (currentLayout)
	{
		currentLayout->clearInput(); // clear input
		applyStyle(currentLayout->getContext());
		updateDimensions(); //apply dimension settings to layout
	}
}

GuiLayout* Gui::getLayout() const
{
	return currentLayout;
}

void Gui::updateDimensions()
{
	auto window = SystemManager::getInstance().getSystem<RenderSystem>()->getWindow();
	dimensions.windowWidth = (float)window->getWidth();
	dimensions.windowHeight = (float)window->getHeight();

	/* calculate new gui size */
	int s = (int)dimensions.windowHeight / 333 - 1;	
	s += uiSizeOffset->get();
	if (s <= 0)
	{
		size = GUI_SIZE_SMALL;
	}
	else if (s >= 2)
	{
		size = GUI_SIZE_LARGE;
	}
	else
	{
		size = GUI_SIZE_NORMAL;
	}

	/* update everything */

	dimensions.rowHeight = rowHeights[size];
	dimensions.rowSpacing = 10;
	dimensions.fontHeight = fontSizes[size];

	renderer.setDimensions(dimensions.windowWidth, dimensions.windowHeight);

	if (currentLayout)
	{
		nk_user_font *font = fontManager.getFont(size);
		nk_style_set_font(currentLayout->getContext(), font);
		currentLayout->updateDimensions();
		currentLayout->clearInput();
	}

	notifyDimensionListeners();
}

void Gui::render()
{
	nk_context *ctx = nullptr;
	if (currentLayout)
	{
		ctx = currentLayout->layout();
		if (styleBlock)
		{
			styleBlock->apply(ctx->style);
		}
		if (showOverview)
		{
			nuklearOverview(ctx);
		}
	}
	renderer.render(ctx);
}

void Gui::input()
{
	if (currentLayout)
	{
		inputHandler.input(currentLayout->getContext());
	}
}

void Gui::update()
{
	if (currentLayout)
	{
		currentLayout->update3DWindows();
	}
}

float parseVars(const char *text)
{
	if (strlen(text) == 1)
	{
		switch (text[0])
		{
		case 'w': return Gui::dimensions.windowWidth;
		case 'h': return Gui::dimensions.windowHeight;
		case 'r': return Gui::dimensions.rowHeight;
		case 's': return Gui::dimensions.rowSpacing;
		case 'f': return Gui::dimensions.fontHeight;
		}
	}

	try
	{
		return std::stof(text);
	}
	catch (std::invalid_argument)
	{
		printf("!!! ERROR: Gui parseVars %s is not a number !!!", text);
	}
	return 1.0f;
}

float parseMult(char *text)
{
	char *token = NULL, *context = NULL;
	float result = 1.0;

	token = strtok_s(text, "*", &context);
	while (token != NULL)
	{
		result *= parseVars(token);
		token = strtok_s(NULL, "*", &context);
	}

	return result;
}

float parseAdd(char *text)
{
	char *token = NULL, *context = NULL;
	float result = 0.0;

	token = strtok_s(text, "+", &context);
	while (token != NULL)
	{
		result += parseMult(token);
		token = strtok_s(NULL, "+", &context);
	}

	return result;
}

float Gui::parseDimension(const char *text)
{
	size_t size = strlen(text) + 1;
	char* tmp = new char[size];
	strcpy_s(tmp, size, text);

	float result = parseAdd(tmp);
	delete[] tmp;
	return result;
}

float Gui::parseDimension(const std::string &text)
{
	return parseDimension(text.c_str());
}

void Gui::toggleNuklearOverviewExample()
{
	showOverview = !showOverview;
}

bool Gui::isGuiArea(float x, float y)
{
	const GuiLayout *layout = instance->currentLayout;
	if (layout)
	{
		for (GuiWindow *window : layout->windows)
		{
			if (window->isWindowArea(x, y))
			{
				return true;
			}
		}

		if (layout->popupWindow && layout->popupWindow->isWindowArea(x, y))
		{
			return true;
		}
	}
	return false;
}

void Gui::setKeyboardFocus(GuiElement *element)
{
	keyboardFocus = element;
}

void Gui::unsetKeyboardFocus(GuiElement *element)
{
	if (element == keyboardFocus)
	{
		keyboardFocus = nullptr;
	}
}

bool Gui::hasKeyboardFocus()
{
	return keyboardFocus != nullptr;
}

void Gui::setStyleSheet(const GuiStyleSheet &styleSheet)
{
	this->styleSheet = styleSheet;
	updateStyleBlock();
}
void Gui::addStyleSheet(const GuiStyleSheet &styleSheet)
{
	this->styleSheet += styleSheet;
	updateStyleBlock();
}

GuiStyleBlock* Gui::getStyleClassBlock(const std::string &name)
{
	return instance->styleSheet.getClassBlock(name);
}

void Gui::clickSound()
{
	playGuiSound("Resources/Sounds/click.ogg", 0.5f);
}

void Gui::errorSound()
{
	playGuiSound("Resources/Sounds/error.ogg", 0.5f);
}

void Gui::notificationSound()
{
	playGuiSound("Resources/Sounds/notification.ogg", 0.3f);
}

void Gui::addDimensionListener(IDimensionable *listener)
{
	if (!contains(dimensionListeners, listener))
	{
		dimensionListeners.push_back(listener);
	}
}

void Gui::removeDimensionListener(IDimensionable *listener)
{
	remove(dimensionListeners, listener);
}

void Gui::notifyDimensionListeners()
{
	for (auto listener : dimensionListeners)
	{
		listener->updateDimensions();
	}
}

void Gui::playGuiSound(const std::string &file, float volume)
{
	EntityManager &eManager = EntityManager::getInstance();
	eManager.addComponent<SoundComponent>(instance->soundEntity, file, SoundType::UI, volume, false, false);
}

nk_user_font* Gui::getFont(int offset)
{
	return fontManager.getFont((GuiSize)((int)size + offset));
}