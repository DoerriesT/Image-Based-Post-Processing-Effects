#include "GuiStyleSheet.h"
#include "Utilities\ContainerUtility.h"
#include "..\Gui.h"

GuiStyleSheet::GuiStyleSheet()
{
}

GuiStyleSheet::~GuiStyleSheet()
{
}

void GuiStyleSheet::addClassProperty(const std::string &name, GuiStyleProperty *prop)
{
	if (!ContainerUtility::contains(classes, name))
	{
		classes[name] = GuiStyleBlock();
	}
	classes[name].addProperty(prop);
	notifyListener();
}

GuiStyleBlock* GuiStyleSheet::getClassBlock(const std::string &name)
{
	if (name.empty())
	{
		return nullptr;
	}

	if (!ContainerUtility::contains(classes, name))
	{
		classes[name] = GuiStyleBlock();
		notifyListener();
	}

	return &classes[name];
}

void GuiStyleSheet::addListener(IStyleable *l)
{
	if (!ContainerUtility::contains(listener, l))
	{
		listener.push_back(l);
	}
}

void GuiStyleSheet::removeListener(IStyleable *l)
{
	ContainerUtility::remove(listener, l);
}

GuiStyleSheet& GuiStyleSheet::operator=(const GuiStyleSheet &other)
{
	clear();
	return *this += other;
}

GuiStyleSheet& GuiStyleSheet::operator+=(const GuiStyleSheet &other)
{
	for (auto block : other.classes)
	{
		for (GuiStyleProperty *p : block.second.properties)
		{
			addClassProperty(block.first, p);
		}
	}
	notifyListener();
	return *this;
}

void GuiStyleSheet::notifyListener()
{
	for (IStyleable *s : listener)
	{
		s->updateStyleBlock();
	}
}

void GuiStyleSheet::clear()
{
	for (auto iter = classes.begin(); iter != classes.end(); ++iter)
	{
		std::string key = iter->first;
		GuiStyleBlock &block = classes[key];

		for (GuiStyleProperty *p : block.properties)
		{
			delete p;
		}
		block.properties.clear();
	}
}