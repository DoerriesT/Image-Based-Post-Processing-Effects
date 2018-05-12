#pragma once
#include "IIdentifiable.h"

enum class GuiEventType
{
	Clicked,
	ValueChanged,
	TextCommited,
	Custom,
	Popup,
};

struct GuiEvent
{
	GuiEventType type;
	class GuiLayout *sourceLayout;
	class IIdentifiable *source;
	int64_t userData;

	GuiEvent(GuiEventType type) : GuiEvent(type, nullptr, 0)
	{
	}
	GuiEvent(GuiEventType type, IIdentifiable *source) : GuiEvent(type, source, 0)
	{
	}
	GuiEvent(GuiEventType type, int userData) : GuiEvent(type, nullptr, userData)
	{
	}
	GuiEvent(GuiEventType type, IIdentifiable *source, int userData) : type(type), sourceLayout(nullptr), source(source), userData(userData)
	{
	}
};


class IGuiEventListener
{
public:
	virtual ~IGuiEventListener() = default;
	virtual void guiEventNotification(struct GuiEvent &_event) = 0;
};




