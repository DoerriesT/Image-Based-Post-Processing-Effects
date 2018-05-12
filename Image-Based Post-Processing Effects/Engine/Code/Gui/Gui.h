#pragma once
#include "Systems/GuiRenderer.h"
#include "Systems/GuiFontManager.h"
#include "GuiLayout.h"
#include "Systems/GuiInputHandler.h"
#include <GLFW/glfw3.h>
#include "IIdentifiable.h"
#include "IDimensionable.h"
#include "nuklearInclude.h"
#include "GuiPopup.h"
#include "..\Settings.h"
#include "Style\GuiStyleSheet.h"
#include "..\EntityComponentSystem\EntityManager.h"
#include <Settings.h>

enum GuiSize : int
{
	GUI_SIZE_TINY = 0,
	GUI_SIZE_SMALL,
	GUI_SIZE_NORMAL,
	GUI_SIZE_LARGE,
	GUI_SIZE_HUGE,
	GUI_SIZE_COUNT
};

struct GuiDimensions
{
	float rowHeight;
	float rowSpacing;
	float windowWidth;
	float windowHeight;
	float fontHeight;
};

class Gui : public IStyleable, private IDimensionable
{
	friend GuiTextField;
public:
	Gui();
	~Gui();

	void init();
	void input();
	void update();
	void render();

	void setStyleSheet(const GuiStyleSheet &styleSheet);
	void addStyleSheet(const GuiStyleSheet &styleSheet);
	void setLayout(GuiLayout *layout);
	GuiLayout* getLayout() const;

	GuiSize size = GUI_SIZE_NORMAL;
	static const float fontSizes[];
	static const float rowHeights[];
	static GuiDimensions dimensions;
	virtual void updateDimensions();

	inline static Gui* getInstance();
	nk_user_font* getFont(int offset);
	static float parseDimension(const char *text);
	static float parseDimension(const std::string &text);

	static bool hasKeyboardFocus();
	
	void toggleNuklearOverviewExample();

	static bool isGuiArea(const float &x, const float &y);

	static inline const GuiRenderer& getRenderer();
	static inline GuiStyleSheet& getStyleSheet();
	static GuiStyleBlock* getStyleClassBlock(const std::string &name);

	static void clickSound();
	static void errorSound();
	static void notificationSound();

	void addDimensionListener(IDimensionable *listener);
	void removeDimensionListener(IDimensionable *listener);
	void notifyDimensionListeners();

private:
	static Gui *instance;
	GuiRenderer renderer;
	GuiInputHandler inputHandler;
	GuiFontManager fontManager;
	GuiStyleSheet styleSheet;
	const Entity *soundEntity;
	std::vector<IDimensionable*> dimensionListeners;

	GuiLayout *currentLayout;
	nk_context *overviewCtx;
	bool showOverview; //TODO: remove for release
	static GuiElement* keyboardFocus;
	static void setKeyboardFocus(GuiElement *element);
	static void unsetKeyboardFocus(GuiElement *element);
	static void playGuiSound(const std::string &file, float volume);

	std::shared_ptr<Setting<int>> windowWidth;
	std::shared_ptr<Setting<int>> windowHeight;
	std::shared_ptr<Setting<int>> windowMode;
	std::shared_ptr<Setting<int>> uiSizeOffset;
};

inline Gui *Gui::getInstance()
{
	return instance;
}

inline const GuiRenderer& Gui::getRenderer()
{
	return instance->renderer;
}

inline GuiStyleSheet& Gui::getStyleSheet()
{
	return instance->styleSheet;
}
