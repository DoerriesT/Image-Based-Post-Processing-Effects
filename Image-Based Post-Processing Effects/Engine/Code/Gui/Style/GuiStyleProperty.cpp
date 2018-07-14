#include "GuiStyleProperty.h"
#include <string>
#include "..\nuklearInclude.h"
#include "..\..\Utilities\Utility.h"

struct nk_vec2 parseSize(const char *value)
{
	struct nk_vec2 size;
	size_t len = strlen(value) + 1;
	char* tmp = new char[len];
	strcpy_s(tmp, len, value);

	char *token, *next_token;
	try
	{
		if (token = strtok_s(tmp, ",", &next_token))
			size.x = std::stof(token);
		if (token = strtok_s(NULL, ",", &next_token))
			size.y = std::stof(token);
	}
	catch (std::exception&)
	{
		printf("GUI PARSE ERROR: %s is not a size\n", value);
		size.x = 0.0f;
		size.y = 0.0f;
	}
	delete[] tmp;

	return size;
}

nk_color parseColor(const char *value)
{
	nk_color color;

	size_t size = strlen(value) + 1;
	char* tmp = new char[size];
	strcpy_s(tmp, size, value);
	char *token, *next_token;

	try
	{
		if (token = strtok_s(tmp, ",", &next_token))
			color.r = std::stoi(token);
		if (token = strtok_s(NULL, ",", &next_token))
			color.g = std::stoi(token);
		if (token = strtok_s(NULL, ",", &next_token))
			color.b = std::stoi(token);
		if (token = strtok_s(NULL, ",", &next_token))
			color.a = std::stoi(token);
	}
	catch (std::exception&)
	{
		printf("GUI PARSE ERROR: %s is not a color\n", value);
		color.r = 0;
		color.g = 0;
		color.b = 0;
		color.a = 0;
	}

	delete[] tmp;
	return color;
}

nk_style_item parseStyleItem(const char *value)
{
	if (Utility::equalsIgnoreCase(value, "hide"))
	{
		return nk_style_item_hide();
	}
	else
	{
		nk_color color = parseColor(value);
		return nk_style_item_color(color);
	}
}


GuiStyleProperty::GuiStyleProperty(size_t offset) : offset(offset)
{
}

GuiStyleProperty::~GuiStyleProperty()
{
}


void GuiStyleFloatProperty::apply(nk_style &style)
{
	float *p = (float*)getPointer(style);
	prevValue = *p;
	*p = value;
}

void GuiStyleFloatProperty::revert(nk_style &style) const
{
	float *p = (float*)getPointer(style);
	*p = prevValue;
}

void GuiStyleColorProperty::apply(nk_style &style)
{
	nk_color *p = (nk_color*)getPointer(style);
	prevColor = *p;
	*p = color;
}

void GuiStyleColorProperty::revert(nk_style &style) const
{
	nk_color *p = (nk_color*)getPointer(style);
	*p = prevColor;
}

void GuiStyleItemProperty::apply(nk_style &style)
{
	nk_style_item *p = (nk_style_item*)getPointer(style);
	prevItem = *p;
	*p = item;
}

void GuiStyleItemProperty::revert(nk_style &style) const
{
	nk_style_item *p = (nk_style_item*)getPointer(style);
	*p = prevItem;
}

void GuiStyleSizeProperty::apply(nk_style &style)
{
	struct nk_vec2 *p = (struct nk_vec2*)getPointer(style);
	prevSize = *p;
	*p = size;
}

void GuiStyleSizeProperty::revert(nk_style &style) const
{
	struct nk_vec2 *p = (struct nk_vec2*)getPointer(style);
	*p = prevSize;
}

void GuiStyleUIntProperty::apply(nk_style &style)
{
	uint32_t* s = (uint32_t*)getPointer(style);
	prevValue = *s;
	*s = value;
}

void GuiStyleUIntProperty::revert(nk_style &style) const
{
	uint32_t* s = (uint32_t*)getPointer(style);
	*s = prevValue;
}

GuiStyleFloatProperty::GuiStyleFloatProperty(size_t offset, const char* value)
	: GuiStyleProperty(offset), value(std::stof(value))
{
}

GuiStyleColorProperty::GuiStyleColorProperty(size_t offset, const char *value)
	: GuiStyleProperty(offset), color(parseColor(value))
{
}

GuiStyleItemProperty::GuiStyleItemProperty(size_t offset, const char *value)
	: GuiStyleProperty(offset), item(parseStyleItem(value))
{
}

GuiStyleSizeProperty::GuiStyleSizeProperty(size_t offset, const char *value)
	: GuiStyleProperty(offset), size(parseSize(value))
{
}

GuiStyleUIntProperty::GuiStyleUIntProperty(size_t offset, const char *value)
	: GuiStyleProperty(offset), value(std::stoi(value))
{
}

