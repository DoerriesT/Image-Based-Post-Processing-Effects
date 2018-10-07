#include "GuiElement.h"
#include "..\GuiUtility.h"
#include "..\Gui.h"
#include "Utilities\ContainerUtility.h"
#include <algorithm>

using GuiUtil::copy;

GuiElement::GuiElement(GuiRow *row, const std::string &id) : IIdentifiable(id), row(row)
{
}

GuiElement::~GuiElement()
{
}

void GuiElement::layout(nk_context *ctx)
{
	applyStyle(ctx);
	elementLayout(ctx);
	revertStyle(ctx);
}

GuiRow* GuiElement::getParentRow() const
{
	return row;
}

void GuiElement::updateDimensions()
{
}

void GuiElement::guiEventNotification(struct GuiEvent &_event)
{
	if (!_event.source)
	{
		_event.source = this;
	}
	row->guiEventNotification(_event);
}

GuiButton::GuiButton(GuiRow *row, const std::string &id, const std::string &text) : GuiElement(row, id)
{
	setText(text);
}

void GuiButton::setText(const std::string &text)
{
	copy(this->text, text);
}

GuiLabel::GuiLabel(GuiRow *row, const std::string &id, const std::string &text, const nk_text_alignment align)
	: GuiElement(row, id), align(align), usesFontOffset(false)
{
	setText(text);
}

void GuiButton::elementLayout(nk_context *ctx)
{
	if (nk_button_label(ctx, text))
	{
		Gui::clickSound();
		GuiEvent _event(GuiEventType::Clicked);
		guiEventNotification(_event);
	}
}

void GuiLabel::elementLayout(nk_context *ctx)
{
	if (usesFontOffset)
	{
		nk_style_push_font(ctx, Gui::getInstance()->getFont(fontOffset));
		nk_label(ctx, text, align);
		nk_style_pop_font(ctx);
	}
	else
	{
		nk_label(ctx, text, align);
	}
}


void GuiLabel::setFontOffset(int offset)
{
	if (offset == 0)
	{
		unsetFontOffset();
	}
	else
	{
		this->fontOffset = offset > 0 ? 1 : -1;
		usesFontOffset = true;
	}

}

void GuiLabel::unsetFontOffset()
{
	usesFontOffset = false;
}

void GuiLabel::setText(const std::string &text)
{
	copy(this->text, text);
}

GuiTextField::GuiTextField(GuiRow *row, const std::string &id, const std::string &text, GuiTextFieldType type)
	: GuiTextField(row, id, text, type, 256, NK_EDIT_SIMPLE)
{
}

GuiTextField::GuiTextField(GuiRow *row, const  std::string &id, const std::string &text, GuiTextFieldType type, int maxTextLength, nk_flags flags) : GuiElement(row, id), flags(flags | NK_EDIT_SIG_ENTER | NK_EDIT_SELECTABLE)
{
	this->maxTextLength = maxTextLength;
	this->text = new char[maxTextLength];

	setText(text);
	setType(type);
}

GuiTextField::~GuiTextField()
{
	delete[] text;
}

void GuiTextField::setType(GuiTextFieldType type)
{
	switch (type)
	{
		case GuiTextFieldType::Text:
			filter = nk_filter_default;
			break;

		case GuiTextFieldType::Float:
			filter = nk_filter_float;
			break;

		case GuiTextFieldType::Integer:
			filter = nk_filter_decimal;
			break;
	}
}

int GuiTextField::getTextLength() const
{
	return textLength;
}

void GuiTextField::setText(const std::string &text)
{
	const char* ctext = text.c_str();
	textLength = nk_strlen(ctext);

	strncpy_s(this->text, maxTextLength, ctext, textLength);
}

void GuiTextField::elementLayout(nk_context *ctx)
{
	const int len = textLength;
	nk_flags result = nk_edit_string(ctx, flags, text, &textLength, maxTextLength, filter);

	if (len != textLength)
	{
		guiEventNotification(GuiEvent(GuiEventType::ValueChanged));
	}

	if (result & NK_EDIT_COMMITED)
	{
		Gui::clickSound();
		guiEventNotification(GuiEvent(GuiEventType::TextCommited));
	}
	if (result & NK_EDIT_ACTIVATED)
	{
		Gui::setKeyboardFocus(this);
	}
	if (result & NK_EDIT_DEACTIVATED)
	{
		Gui::unsetKeyboardFocus(this);
		guiEventNotification(GuiEvent(GuiEventType::TextCommited));
	}
}

std::string GuiTextField::getText() const
{
	return std::string(text, textLength);
}

int GuiTextField::getInteger() const
{
	int value = 0;
	try
	{
		value = stoi(getText());
	}
	catch (...)
	{
	}
	return value;
}

float GuiTextField::getFloat() const
{
	float value = 0;
	try
	{
		value = stof(getText());
	}
	catch (...)
	{
	}
	return value;
}

GuiGroup::GuiGroup(GuiRow *row, const std::string &id)
	: GuiElement(row, id), flags(0)
{
	scroll.x = 0;
	scroll.y = 0;
}

GuiGroup::~GuiGroup()
{
}

void GuiGroup::elementLayout(nk_context * ctx)
{
	if (nk_group_scrolled_begin(ctx, &scroll, title, flags))
	{
		for (GuiRow *row : rows)
		{
			row->layout(ctx);
		}
		nk_group_scrolled_end(ctx);
	}
}

void GuiGroup::setTitle(const std::string &title)
{
	copy(this->title, title);
}

void GuiGroup::addRow(GuiRow *row)
{
	if (!row)
		return;

	if (!ContainerUtility::contains(rows, row))
	{
		assert(this->row->getWindow()->getParentLayout()->isUnique(row));
		rows.push_back(row);
	}
}
void GuiGroup::removeRow(GuiRow *row)
{
	ContainerUtility::remove(rows, row);
}

void GuiGroup::setBordered(bool bordered)
{
	if (bordered)
	{
		flags |= NK_WINDOW_BORDER;
	}
	else
	{
		flags &= ~NK_WINDOW_BORDER;
	}
}
void GuiGroup::setTitled(bool titled)
{
	if (titled)
	{
		flags |= NK_WINDOW_TITLE;
	}
	else
	{
		flags &= ~NK_WINDOW_TITLE;
	}
}



void GuiGroup::updateDimensions()
{
	for (GuiRow *row : rows)
	{
		row->updateDimensions();
	}
}

GuiComboBox::GuiComboBox(GuiRow *row, const std::string &id)
	: GuiElement(row, id), items(nullptr), itemCount(0)
{
}

GuiComboBox::~GuiComboBox()
{
	clearItems();
}

void GuiComboBox::elementLayout(nk_context *ctx)
{
	if (items)
	{
		const auto bounds = nk_widget_bounds(ctx);
		int newSelectedItem = nk_combo(ctx, (const char**)items, (int)itemCount, (int)selectedItem, (int)bounds.h, nk_vec2(bounds.w, bounds.h*7));

		if (newSelectedItem != selectedItem)
		{
			selectedItem = newSelectedItem;

			GuiEvent _event(GuiEventType::ValueChanged);
			guiEventNotification(_event);
		}
	}
}

void GuiComboBox::setItems(const std::vector<std::string> &items)
{
	clearItems();

	itemCount = items.size();

	this->items = new char*[itemCount];

	//copy all char* into own array
	for (size_t i = 0; i<itemCount; ++i)
	{
		const char *str = items[i].c_str();

		size_t len = strlen(str) + 1;
		this->items[i] = new char[len];
		strcpy_s(this->items[i], len, str);
	}
}

size_t GuiComboBox::getItemCount() const
{
	return itemCount;
}

const char* GuiComboBox::getItemName(const size_t &item) const
{
	assert(item<itemCount);
	return items[item];
}

const char* GuiComboBox::getSelectedItemName() const
{
	return getItemName(selectedItem);
}

size_t GuiComboBox::getSelectedItem()
{
	return selectedItem;
}

void GuiComboBox::setSelectedItem(const size_t &item)
{
	selectedItem = item % itemCount;
}

void GuiComboBox::clearItems()
{
	selectedItem = 0;
	for (size_t i = 0; i < itemCount; ++i)
	{
		delete[] items[i];
	}
	delete[] items;
	itemCount = 0;
}

GuiSpacing::GuiSpacing(GuiRow *row, const std::string &id, int columns)
	: GuiElement(row, id), columns(columns)
{
}

void GuiSpacing::elementLayout(nk_context *ctx)
{
	nk_spacing(ctx, columns);
}

void GuiSpacing::setColumns(int columns)
{
	this->columns = columns;
	row->updateColumnCount();
}

GuiSlider::GuiSlider(GuiRow *row, const std::string &id)
	: GuiElement(row, id), min(0.0f), value(0.5f), max(1.0f), step(0.01f)
{
}

GuiSlider::~GuiSlider()
{
}

void GuiSlider::elementLayout(nk_context *ctx)
{
	if (nk_slider_float(ctx, min, &value, max, step))
	{
		GuiEvent _event(GuiEventType::ValueChanged);
		guiEventNotification(_event);
	}
}

void GuiSlider::setMin(float min)
{
	this->min = min;
}

void GuiSlider::setMax(float max)
{
	this->max = max;
}

void GuiSlider::setValue(float value)
{
	this->value = value;
}

void GuiSlider::setStep(float step)
{
	this->step = step;
}

float GuiSlider::getMin() const
{
	return min;
}

float GuiSlider::getMax() const
{
	return max;
}

float GuiSlider::getValue() const
{
	return value;
}

float GuiSlider::getStep() const
{
	return step;
}

GuiMultilineLabel::GuiMultilineLabel(GuiRow *row, const std::string &id, const std::string &text, const nk_text_alignment align)
	: GuiElement(row, id), align(align)
{
	std::string t = text;
	std::replace(t.begin(), t.end(), '#', '\n');
	setText(t);
}

void GuiMultilineLabel::elementLayout(nk_context *ctx)
{
	const float rowHeight = nk_widget_height(ctx);

	bool firstRow = true;
	for (const std::string &line : lines)
	{
		if (firstRow)
		{
			firstRow = false;
		}
		else
		{
			nk_layout_row_dynamic(ctx, rowHeight, 1);
		}
		nk_label(ctx, line.c_str(), align);
	}
}

void GuiMultilineLabel::setText(const std::string &text)
{
	lines = Utility::split(text, "\n");
}

GuiToggle::GuiToggle(GuiRow *row, const std::string &id, const std::string &text, const std::string &group, GuiToggleType type, bool checked)
	: GuiElement(row, id), group(group), type(type), checked(checked), grouped(!group.empty())
{
	setText(text);
	row->getWindow()->getParentLayout()->addToggle(this);
}

GuiToggle::~GuiToggle()
{
	row->getWindow()->getParentLayout()->removeToggle(this);
}

void GuiToggle::elementLayout(nk_context *ctx)
{
	if (type == GuiToggleType::CheckBox)
	{
		int result = nk_check_label(ctx, text, checked);
		if (result != checked)
		{
			if (grouped)
			{
				row->getWindow()->getParentLayout()->clearToggleGroup(group);
			}
			checked = result;
			guiEventNotification(GuiEvent(GuiEventType::ValueChanged));
		}
	}
	else // GuiToggleType::RadioButton
	{
		int result = nk_option_label(ctx, text, checked);
		if (result != checked)
		{
			if (grouped)
			{
				if (result)
				{
					row->getWindow()->getParentLayout()->clearToggleGroup(group);
					checked = true;
				}
			}
			else
			{
				checked = result;
			}
			guiEventNotification(GuiEvent(GuiEventType::ValueChanged));
		}
	}
}

void GuiToggle::setText(const std::string &text)
{
	copy(this->text, text);
}

void GuiToggle::setChecked(bool checked)
{
	this->checked = checked ? nk_true : nk_false;
}

bool GuiToggle::isChecked() const
{
	return checked == nk_true;
}

GuiWrapLabel::GuiWrapLabel(GuiRow *row, const std::string &id, const std::string &text)
	: GuiLabel(row, id, text)
{
}

void GuiWrapLabel::elementLayout(nk_context *ctx)
{
	nk_label_wrap(ctx, text);
}
