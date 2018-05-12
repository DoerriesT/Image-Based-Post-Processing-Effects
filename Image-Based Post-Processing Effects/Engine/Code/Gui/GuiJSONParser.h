#pragma once
#include "Elements\GuiWindow.h"
#include "Elements\GuiRow.h"
#include "Elements\GuiElement.h"
#include "Style\GuiStyleBlock.h"
#include "Style\GuiStyleSheet.h"
#include "GuiLayout.h"
#include "..\Utilities\JSONUtility.h"

enum GuiStylePropertyType
{
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeUInt
};

class GuiJSONParser
{
public:
	static GuiWindow* parseWindowJSON(GuiLayout *layout, const JSON::Object &windowJSONObject);
	static GuiRow* parseRowJSON(GuiWindow *window, const JSON::Object &rowJSONObject);
	static GuiElement* parseElementJSON(GuiRow *row, const JSON::Object &elementJSONObject);

	static GuiLayout* parseLayoutJSONFile(const char *filename);
	static GuiLayout* parseLayoutJSON(const char *jsonString);

	static GuiStyleSheet parseStyleJSONFile(const char *filename);
	static GuiStyleSheet parseStyleJSON(const char *jsonString);


	static char* stylePropertyNames[];
	static size_t stylePropertyOffsets[];
	static GuiStylePropertyType stylePropertyTypes[];
	static size_t stylePropertyNamesCount;
	static GuiStyleProperty *createStyleProperty(const char *key, const char *value);
};

