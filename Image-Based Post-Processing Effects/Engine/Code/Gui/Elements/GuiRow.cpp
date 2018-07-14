#include "GuiRow.h"
#include "..\Gui.h"
#include "Utilities\ContainerUtility.h"

GuiRow::GuiRow(GuiWindow *window, const std::string &id, const std::string &height)
	: IIdentifiable(id), window(window), hasLayoutCode(false), hidden(false)
{
	setHeight(height);
	updateRatio();
}


GuiRow::~GuiRow()
{
	for (GuiElement *element : elements)
	{
		delete element;
	}
}

void GuiRow::addElement(GuiElement *element)
{
	if (!element)
		return;

	if (!ContainerUtility::contains(elements, element))
	{
		elements.push_back(element);
	}
	updateColumnCount();
}

void GuiRow::removeElement(GuiElement *element)
{
	ContainerUtility::remove(elements, element);
	updateColumnCount();
}


void GuiRow::updateColumnCount()
{
	columns = 0;

	for (GuiElement *element : elements)
	{
		columns += element->getColumns();
	}
	updateRatio();
}

void GuiRow::updateRatio()
{
	ratio.clear();
	hasRatio = !ratioValues.empty();

	if (hasRatio)
	{
		ratio = ratioValues;
		while (ratio.size() < columns)
		{
			ratio.push_back(1.0f);
		}
		while (ratio.size() > columns)
		{
			ratio.pop_back();
		}
		float sum = 0.0f;
		for (float r : ratio)
		{
			sum += r;
		}
		for (float &r : ratio)
		{
			r /= sum;
		}
	}
}

void GuiRow::layout(nk_context *ctx)
{
	if (!hidden)
	{
		applyStyle(ctx);
		if (hasLayoutCode)
		{
			layoutCodeBlock(ctx);
		}
		else
		{
			if (hasRatio)
			{
				nk_layout_row(ctx, NK_DYNAMIC, h, (int)columns, ratio.data());
			}
			else
			{
				nk_layout_row_dynamic(ctx, h, (int)columns);
			}
			for (GuiElement *element : elements)
			{
				element->layout(ctx);
			}
		}
		revertStyle(ctx);
	}
}

void GuiRow::guiEventNotification(GuiEvent &_event)
{
	if (!_event.source)
	{
		_event.source = this;
	}
	window->guiEventNotification(_event);
}

void GuiRow::updateDimensions()
{
	h = Gui::parseDimension(height);

	for (GuiElement *element : elements)
	{
		element->updateDimensions();
	}
}

void GuiRow::setLayoutCodeBlock(LayoutCodeBlock &&code)
{
	layoutCodeBlock = code;
	hasLayoutCode = true;
}

void GuiRow::clearLayoutCodeBlock()
{
	layoutCodeBlock = nullptr;
	hasLayoutCode = false;
}

bool GuiRow::hasLayoutCodeBlock() const
{
	return hasLayoutCode;
}

GuiWindow * GuiRow::getWindow() const
{
	return window;
}

void GuiRow::setHeight(const std::string &height)
{
	GuiUtil::copy(this->height, height);
	updateDimensions();
}

float GuiRow::getHeight() const
{
	return h;
}

void GuiRow::setHidden(bool hidden)
{
	this->hidden = hidden;
}

bool GuiRow::isHidden() const
{
	return hidden;
}

void GuiRow::setRatios(std::vector<float> ratios)
{
	ratioValues = ratios;
	updateRatio();
}

void GuiRow::setRatios(std::vector<std::string> ratios)
{
	ratioValues.clear();
	for (std::string r : ratios)
	{
		ratioValues.push_back(Gui::parseDimension(r));
	}
	updateRatio();
}

void GuiRow::clearRatios()
{
	ratioValues.clear();
	updateRatio();
}
