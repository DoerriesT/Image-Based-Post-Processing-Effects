#include "IStyleable.h"
#include "..\Gui.h"

IStyleable::IStyleable()
	: styleBlock(nullptr)
{
}

IStyleable::~IStyleable()
{
	clearStyleClass();
}

void IStyleable::setStyleClass(const std::string &name)
{
	if (className != name)
	{
		if (name.empty())
		{
			clearStyleClass();
		}
		else
		{
			className = name;
			updateStyleBlock();
			Gui::getStyleSheet().addListener(this);
		}
	}
}

void IStyleable::clearStyleClass()
{
	if (styleBlock)
	{
		Gui::getStyleSheet().removeListener(this);
	}
	className = "";
	styleBlock = nullptr;
}

void IStyleable::updateStyleBlock()
{
	styleBlock = Gui::getStyleClassBlock(className);
}

GuiStyleBlock* IStyleable::getStyleBlock() const
{
	return styleBlock;
}

