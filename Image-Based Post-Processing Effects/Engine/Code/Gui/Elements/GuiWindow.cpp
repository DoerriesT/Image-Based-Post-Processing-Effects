#include "GuiWindow.h"
#include "..\Gui.h"
#include "Utilities\ContainerUtility.h"
#include "..\GuiLayout.h"
#include "..\GuiUtility.h"
#include "..\Style\GuiStyleSheet.h"

using GuiUtil::copy;
using Utility::empty;

GuiWindow::GuiWindow(GuiLayout *layout, const std::string &id)
	: IIdentifiable(id), parentLayout(layout), parentWindow(nullptr),
	hidden(false), disabled(false), flags(NK_WINDOW_BORDER | NK_WINDOW_TITLE)
{
	setTitle(id);
	bounds = nk_rect(100, 100, 400, 400);
}

GuiWindow::~GuiWindow()
{
	for (GuiRow *row : rows)
	{
		delete row;
	}
}

void GuiWindow::addRow(GuiRow *row)
{
	if (!row)
		return;

	if (!ContainerUtility::contains(rows, row))
	{
		assert(parentLayout->isUnique(row));
		rows.push_back(row);
	}
}

void GuiWindow::removeRow(GuiRow *row)
{
	ContainerUtility::remove(rows, row);
}


void GuiWindow::layout(struct nk_context *ctx)
{
	applyStyle(ctx);

	struct nk_rect b = bounds;
	if (hidden)
	{
		b.h = 0;
		b.w = 0;
	}
	if (nk_begin_titled(ctx, id, title, b, flags))
	{
		if (!hidden)
		{
			for (GuiRow *row : rows)
			{
				row->layout(ctx);
			}
		}
	}
	nk_end(ctx);

	revertStyle(ctx);
}

void GuiWindow::setCentered(bool centered)
{
	this->centered = centered;
	updateDimensions();
}

void GuiWindow::setPosition(const std::string &x, const std::string &y)
{
	copy(this->x, x);
	copy(this->y, y);
	updateDimensions();
}

void GuiWindow::setSize(const std::string &w, const std::string &h)
{
	copy(this->width, w);
	copy(this->height, h);
	updateDimensions();
}

void GuiWindow::setX(const std::string &x)
{
	copy(this->x, x);
	updateDimensions();
}

void GuiWindow::setY(const std::string &y)
{
	copy(this->y, y);
	updateDimensions();
}

void GuiWindow::setWidth(const std::string &w)
{
	copy(this->width, w);
	updateDimensions();
}

void GuiWindow::setHeight(const std::string &h)
{
	copy(this->height, h);
	updateDimensions();
}

const struct nk_rect GuiWindow::getBounds() const
{
	return bounds;
}

bool GuiWindow::isWindowArea(float x, float y)
{
	if (hidden)
	{
		return false;
	}

	return (x >= bounds.x &&
		y >= bounds.y &&
		x <= bounds.x + bounds.w &&
		y <= bounds.y + bounds.h);
}

void GuiWindow::set3DPosition(float x, float y, float z)
{
	setPosition("-2000", "-2000");
	position3D = glm::vec4(x, y, z, 1.0f);
	parentLayout->add3DWindow(this);
}

void GuiWindow::clear3DPosition()
{
	parentLayout->remove3DWindow(this);
}

void GuiWindow::setTitle(const std::string &title)
{
	copy(this->title, title);
}

void GuiWindow::setBordered(bool bordered)
{
	setFlag(NK_WINDOW_BORDER, bordered);
}

void GuiWindow::setFlag(nk_flags flag, bool enabled)
{
	if (enabled)
	{
		flags |= flag;
	}
	else
	{
		flags &= ~flag;
	}
}

bool GuiWindow::getFlag(nk_flags flag) const
{
	return (flags & flag) != 0;
}

void GuiWindow::setHidden(bool hidden)
{
	if (this->hidden != hidden)
	{
		parentLayout->clearInput();
		this->hidden = hidden;
		//nk_context *ctx = parentLayout->getContext();
		//setFlag(NK_WINDOW_HIDDEN, hidden);
		//nk_window_show(ctx, id, hidden ? NK_HIDDEN : NK_SHOWN);

		if (parentWindow)
		{
			parentWindow->setDisabled(!hidden);
		}
	}
}

bool GuiWindow::isHidden() const
{
	return hidden;
}

void GuiWindow::focus()
{
	nk_context *ctx = parentLayout->getContext();
	nk_window_set_focus(ctx, id);
}

void GuiWindow::setScrollbarHidden(bool scrollbarHidden)
{
	setFlag(NK_WINDOW_NO_SCROLLBAR, scrollbarHidden);
}

void GuiWindow::setDisabled(bool disabled)
{
	this->disabled = disabled;
	updateDisabled();
}

bool GuiWindow::isDisabled() const
{
	return disabled;
}

void GuiWindow::updateDisabled()
{
	const bool b = disabled || parentLayout->getDisabled();
	setFlag(NK_WINDOW_ROM | NK_WINDOW_BACKGROUND, b);
	setFlag(NK_WINDOW_REMOVE_ROM, !b);
}

void GuiWindow::setParent(GuiWindow *window)
{
	parentWindow = window;
	if (parentWindow)
	{
		parentWindow->setDisabled(!hidden);
	}
}

GuiWindow* GuiWindow::getParent() const
{
	return parentWindow;
}

GuiLayout* GuiWindow::getParentLayout() const
{
	return parentLayout;
}

void GuiWindow::updateDimensions()
{
	if (!empty(x))
		bounds.x = Gui::parseDimension(x);

	if (!empty(y))
		bounds.y = Gui::parseDimension(y);

	if (!empty(width))
		bounds.w = Gui::parseDimension(width);

	if (!empty(height))
		bounds.h = Gui::parseDimension(height);

	if (centered)
	{
		bounds.x = Gui::dimensions.windowWidth / 2 - bounds.w / 2 - bounds.x;
		bounds.y = Gui::dimensions.windowHeight / 2 - bounds.h / 2 - bounds.y;
	}

	for (GuiRow *row : rows)
	{
		row->updateDimensions();
	}
}

void GuiWindow::guiEventNotification(GuiEvent &_event)
{
	if (!_event.source)
	{
		_event.source = this;
	}
	parentLayout->guiEventNotification(_event);
}




