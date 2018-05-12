#include "GuiStyleBlock.h"
#include "..\..\Utilities\Utility.h"

GuiStyleBlock::GuiStyleBlock()
{
}

GuiStyleBlock::~GuiStyleBlock()
{
}

void GuiStyleBlock::apply(nk_style &style)
{
	for (GuiStyleProperty* property : properties)
	{
		property->apply(style);
	}
}

void GuiStyleBlock::revert(nk_style &style) const
{
	// reversed iteration
	for (auto it = properties.crbegin(); it!= properties.crend(); ++it)
	{
		const GuiStyleProperty *property = *it;
		property->revert(style);
	}
}

void GuiStyleBlock::addProperty(GuiStyleProperty *p)
{
	remove(properties, p);
	properties.push_back(p);
}
