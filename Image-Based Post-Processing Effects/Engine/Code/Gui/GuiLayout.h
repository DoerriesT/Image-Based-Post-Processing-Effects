#pragma once
#include <vector>
#include "nuklearInclude.h"
#include "Elements\GuiWindow.h"
#include "GuiEvent.h"
#include "GuiUtility.h"
#include "Style\GuiStyleSheet.h"
#include "..\Utilities\Utility.h"
#include "IIdentifiable.h"
#include "IDimensionable.h"

struct nk_context;
struct nk_color;
class Gui;
class GuiPopup;

class GuiLayout : public IGuiEventListener, public IIdentifiable, public IStyleable, private IDimensionable
{
	friend Gui;
public:
	GuiLayout(const std::string &id);
	~GuiLayout();

	void init();
	nk_context *layout();
	nk_context *getContext();
	void setEventListener(IGuiEventListener * listener);
	void addWindow(GuiWindow *window);
	void removeWindow(GuiWindow *window);
	virtual void updateDimensions() override;
	void clearInput();

	virtual void guiEventNotification(GuiEvent &_event) override;
	void add3DWindow(GuiWindow *window);
	void remove3DWindow(GuiWindow *window);
	void update3DWindows(bool forceUpdate = false);

	template<typename T>
	bool getElementById(const char *id, T *&resultElement);

	template<typename T>
	T* getElementById(const char *id);

	template<typename T>
	T* getElementById_s(const char *id);

	void setDisabled(bool disabled);
	bool getDisabled() const;

	void displayPopup(const GuiPopup &popup);
	void removePopup();
	void removePopup(const std::string &id);

	void addToggle(GuiToggle *toggle);
	void removeToggle(GuiToggle *toggle);
	void clearToggleGroup(const std::string &group);

	template<typename T>
	bool isUnique(T *obj);

private:
	std::vector<GuiWindow*> windows;
	nk_context *ctx;
	IGuiEventListener *eventListener;
	bool disabled;

	std::vector<GuiWindow*> windows3D;
	GuiWindow *popupWindow;
	std::vector<GuiPopup> popups;

	void updatePopup();

	std::map<std::string, std::vector<GuiToggle*>> toggleMap;
};

template<typename T>
bool GuiLayout::getElementById(const char *id, T *&resultElement)
{
	for (GuiWindow *window : windows)
	{
		if (window->getElementById(id, resultElement))
		{
			return true;
		}
	}
	return false;
}

template<typename T>
inline T* GuiLayout::getElementById_s(const char *id)
{
	T* element = getElementById<T>(id);
	if (!element)
	{
		printf("ERROR: gui element \"%s\" not found\n", id);
		assert(false && "element not found, check stdout for more info.");
	}
	return element;
}

template<typename T>
inline T* GuiLayout::getElementById(const char *id)
{
	T* element;
	if (getElementById<T>(id, element))
	{
		return element;
	}
	return nullptr;
}


template<typename T>
inline bool GuiLayout::isUnique(T *obj)
{
	if (!std::is_base_of<IIdentifiable, T>::value)
	{
		printf("ERROR: not an IIdentifiable\n");
		return false;
	}

	const char *id = obj->getId();
	if (!Util::empty(id) && getElementById<T>(id))
	{
		printf("ERROR: The ID \"%s\" is already used for the same type in this layout (\"%s\")\n", id, this->id);
		return false;
	}

	return true;
}