#include "..\DebugMacro.h"
#include "GuiJSONParser.h"
#include "Utilities\Utility.h"
#include "GuiUtility.h"
#include "GuiLayout.h"
#include "Style\GuiStyleProperty.h"
#include "nuklearInclude.h"
#include <algorithm>

using namespace JSON;
using rapidjson::SizeType;

GuiLayout* GuiJSONParser::parseLayoutJSONFile(const char *filename)
{
	GuiLayout *layout = parseLayoutJSON(Utility::readTextFile(filename).data());
	return layout;
}

GuiLayout* GuiJSONParser::parseLayoutJSON(const char *jsonString)
{
	rapidjson::Document document;
	document.Parse(jsonString);
	assert(document.IsObject());
	Object jsonObject = document.GetObject();

	std::string id;

	if (getString(jsonObject, "id", id))
	{
		GuiLayout *layout = new GuiLayout(id);

		layout->setStyleClass(getStringDefault(jsonObject, "class", ""));

		Value* array = getMemberValue(jsonObject, "windows");
		if (array && array->IsArray())
		{
			for (auto &obj : array->GetArray())
			{
				if (obj.IsObject())
				{
					layout->addWindow(parseWindowJSON(layout, obj.GetObject()));
				}
			}
		}
		return layout;
	}
	else
	{
		return nullptr;
	}

}

GuiStyleSheet GuiJSONParser::parseStyleJSONFile(const char *filename)
{
	GuiStyleSheet styleSheet = parseStyleJSON(Utility::readTextFile(filename).data());
	return styleSheet;
}

GuiStyleSheet GuiJSONParser::parseStyleJSON(const char *jsonString)
{
	rapidjson::Document document;
	document.Parse(jsonString);

	if (!document.IsObject())
	{
		return GuiStyleSheet();
	}
	Object obj = document.GetObject();

	GuiStyleSheet styleSheet;

	for (auto sheetItr = obj.MemberBegin(); sheetItr != obj.MemberEnd(); ++sheetItr)
	{
		if (!sheetItr->name.IsString() || !sheetItr->value.IsObject())
			continue;

		const char* className = sheetItr->name.GetString();
		Object classObj = sheetItr->value.GetObject();

		for (auto classItr = classObj.MemberBegin(); classItr != classObj.MemberEnd(); ++classItr)
		{
			if (!classItr->name.IsString() || !classItr->value.IsString())
				continue;

			const char* propertyName = classItr->name.GetString();
			const char* propertyValue = classItr->value.GetString();

			GuiStyleProperty *prop = createStyleProperty(propertyName, propertyValue);
			if (prop)
			{
				styleSheet.addClassProperty(className, prop);
			}
			else
			{
				printf("STYLE PARSE ERROR: %s : %s : %s\n", className, propertyName, propertyValue);
			}
		}
	}

	return styleSheet;
}




GuiWindow* GuiJSONParser::parseWindowJSON(GuiLayout *layout, const Object &windowJSONObject)
{
	std::string id;
	if (getString(windowJSONObject, "id", id))
	{
		GuiWindow *window = new GuiWindow(layout, id);
		std::string x, y, w, h;
		x = getStringDefault(windowJSONObject, "x", "0");
		y = getStringDefault(windowJSONObject, "y", "0");
		w = getStringDefault(windowJSONObject, "width", "0.5*w");
		h = getStringDefault(windowJSONObject, "height", "0.5*h");

		bool centered = getBoolDefault(windowJSONObject, "centered", true);

		window->setStyleClass(getStringDefault(windowJSONObject, "class", ""));

		window->setBordered(getBoolDefault(windowJSONObject, "bordered", true));
		window->setFlag(NK_WINDOW_MOVABLE, getBoolDefault(windowJSONObject, "movable", false));
		window->setFlag(NK_WINDOW_SCALABLE, getBoolDefault(windowJSONObject, "scalable", false));
		window->setFlag(NK_WINDOW_MINIMIZABLE, getBoolDefault(windowJSONObject, "minimizable", false));
		window->setFlag(NK_WINDOW_TITLE, getBoolDefault(windowJSONObject, "titled", true));

		std::string title;
		if (getString(windowJSONObject, "title", title))
		{
			window->setTitle(title);
		}

		window->setCentered(centered);
		window->setPosition(x, y);
		window->setSize(w, h);


		Value* rowJSONArray = getMemberValue(windowJSONObject, "rows");
		if (rowJSONArray && rowJSONArray->IsArray())
		{
			for (auto &obj : rowJSONArray->GetArray())
			{
				if (obj.IsObject())
				{
					GuiRow *row = parseRowJSON(window, obj.GetObject());
					window->addRow(row);
				}
			}
		}

		return window;
	}
	else
	{
		DBG_LOG("window is missing id");
		return nullptr;
	}
}



GuiStyleProperty * GuiJSONParser::createStyleProperty(const char *key, const char *value)
{
	GuiStyleProperty *prop = nullptr;

	for (uint32_t i = 0; i < stylePropertyNamesCount; ++i)
	{
		if (_stricmp(key, stylePropertyNames[i]) == 0) // equals ignore case 
		{
			size_t offset = stylePropertyOffsets[i];

			switch (stylePropertyTypes[i])
			{
				case GuiStylePropertyTypeColor:
					prop = new GuiStyleColorProperty(offset, value);
					break;
				case GuiStylePropertyTypeFloat:
					prop = new GuiStyleFloatProperty(offset, value);
					break;
				case GuiStylePropertyTypeSize:
					prop = new GuiStyleSizeProperty(offset, value);
					break;
				case GuiStylePropertyTypeItem:
					prop = new GuiStyleItemProperty(offset, value);
					break;
				case GuiStylePropertyTypeUInt:
					prop = new GuiStyleUIntProperty(offset, value);
					break;
			}
			break;
		}
	}

	return prop;
}

GuiRow* GuiJSONParser::parseRowJSON(GuiWindow *window, const Object &rowJSONObject)
{
	std::string height = getStringDefault(rowJSONObject, "height", "1*r");
	std::string id = getStringDefault(rowJSONObject, "id", "");
	std::string ratioStr = getStringDefault(rowJSONObject, "ratio", "");
	Utility::ltrim(ratioStr);
	Utility::rtrim(ratioStr);
	std::replace(ratioStr.begin(), ratioStr.end(), ':', ';');

	//create window
	GuiRow *row = new GuiRow(window, id, height);

	if (!ratioStr.empty())
	{
		
		row->setRatios(Utility::split(ratioStr, ";"));
	}

	row->setStyleClass(getStringDefault(rowJSONObject, "class", ""));

	Value *elementsJSONArray = getMemberValue(rowJSONObject, "elements");
	if (elementsJSONArray && elementsJSONArray->IsArray())
	{
		for (auto &obj : elementsJSONArray->GetArray())
		{
			if (obj.IsObject())
			{
				GuiElement *element = parseElementJSON(row, obj.GetObject());
				row->addElement(element);
			}
		}
	}
	return row;
}

GuiElement* GuiJSONParser::parseElementJSON(GuiRow *row, const Object &elementJSONObject)
{
	GuiElement *element = nullptr;

	std::string id = getStringDefault(elementJSONObject, "id", "");
	std::string type;

	if (getString(elementJSONObject, "type", type))
	{

		if (type == "button")
		{
			std::string text = getStringDefault(elementJSONObject, "text", "");
			element = new GuiButton(row, id, text);
		}
		else if (type == "spacing")
		{
			int columns = getIntDefault(elementJSONObject, "columns", 1);
			element = new GuiSpacing(row, id, columns);
		}
		else if (type == "label")
		{
			nk_text_alignment align = NK_TEXT_CENTERED;

			std::string text = getStringDefault(elementJSONObject, "text", "");

			std::string alignStr;
			if (getString(elementJSONObject, "align", alignStr))
			{
				if (alignStr == "left")
				{
					align = NK_TEXT_LEFT;
				}
				else if (alignStr == "right")
				{
					align = NK_TEXT_RIGHT;
				}
			}

			bool fontOffset = getIntDefault(elementJSONObject, "font_offset", 0);

			bool multiline = getBoolDefault(elementJSONObject, "multiline", false);
			if (multiline)
			{
				element = new GuiMultilineLabel(row, id, text, align);
				
			}
			else
			{
				GuiLabel *label = new GuiLabel(row, id, text, align);
				label->setFontOffset(fontOffset);
				element = label;
			}
		}
		else if (type == "textfield")
		{
			std::string text = getStringDefault(elementJSONObject, "text", "");
			std::string inputTypeStr = Utility::toLowerCase(getStringDefault(elementJSONObject, "input_type", ""));
			GuiTextFieldType inputType;
			if (inputTypeStr == "int" || inputTypeStr == "integer" || inputTypeStr == "decimal")
			{
				inputType = GuiTextFieldType::Integer;
			}
			else if (inputTypeStr == "float" || inputTypeStr == "double")
			{
				inputType = GuiTextFieldType::Float;
			}
			else
			{
				inputType = GuiTextFieldType::Text;
			}

			element = new GuiTextField(row, id, text, inputType);
		}
		else if (type == "combobox")
		{
			GuiComboBox *comboBox = new GuiComboBox(row, id);

			std::string itemsStr = getStringDefault(elementJSONObject, "items", "");
			std::vector<std::string> items = Utility::split(itemsStr, ";");
			comboBox->setItems(items);

			element = comboBox;
		}
		else if (type == "group")
		{
			GuiGroup *newGroup = new GuiGroup(row, id);

			Value *v = getMemberValue(elementJSONObject, "rows");
			if (v && v->IsArray())
			{
				for (auto &obj : v->GetArray())
				{
					if (obj.IsObject())
					{
						newGroup->addRow(parseRowJSON(row->getWindow(), obj.GetObject()));
					}
				}
			}

			std::string title = getStringDefault(elementJSONObject, "title", id);
			bool bordered = getBoolDefault(elementJSONObject, "bordered", false);
			bool titled = getBoolDefault(elementJSONObject, "titled", false);
			newGroup->setTitle(title);
			newGroup->setTitled(titled);
			newGroup->setBordered(bordered);

			element = newGroup;
		}
		else if (type == "slider")
		{
			GuiSlider *newSlider = new GuiSlider(row, id);

			newSlider->setMin(getFloatDefault(elementJSONObject, "min", 0.0f));
			newSlider->setMax(getFloatDefault(elementJSONObject, "max", 1.0f));
			newSlider->setValue(getFloatDefault(elementJSONObject, "value", 0.5f));
			newSlider->setStep(getFloatDefault(elementJSONObject, "step", 0.01f));

			element = newSlider;
		}
		else if (type == "checkbox" || type == "radiobutton")
		{
			std::string text = getStringDefault(elementJSONObject, "text", "");
			std::string group = getStringDefault(elementJSONObject, "group", "");
			bool checked = getBoolDefault(elementJSONObject, "checked", false);
			GuiToggleType tt = type == "checkbox" ? GuiToggleType::CheckBox : GuiToggleType::RadioButton;

			element = new GuiToggle(row, id, text, group, tt, checked);
		}
	}

	element->setStyleClass(getStringDefault(elementJSONObject, "class", ""));

	return element;
}


size_t GuiJSONParser::stylePropertyNamesCount = 242u;

char *GuiJSONParser::stylePropertyNames[] = {
	"text-color",
	"text-padding",
	"button-normal",
	"button-hover",
	"button-active",
	"button-border-color",
	"button-text-background",
	"button-text-normal",
	"button-text-hover",
	"button-text-active",
	"button-border",
	"button-rounding",
	"button-padding",
	"button-image-padding",
	"button-touch-padding",
	"contextual-button-normal",
	"contextual-button-hover",
	"contextual-button-active",
	"contextual-button-border-color",
	"contextual-button-text-background",
	"contextual-button-text-normal",
	"contextual-button-text-hover",
	"contextual-button-text-active",
	"contextual-button-border",
	"contextual-button-rounding",
	"contextual-button-padding",
	"contextual-button-image-padding",
	"contextual-button-touch-padding",
	"menu-button-normal",
	"menu-button-hover",
	"menu-button-active",
	"menu-button-border-color",
	"menu-button-text-background",
	"menu-button-text-normal",
	"menu-button-text-hover",
	"menu-button-text-active",
	"menu-button-border",
	"menu-button-rounding",
	"menu-button-padding",
	"menu-button-image-padding",
	"menu-button-touch-padding",
	"option-normal",
	"option-hover",
	"option-active",
	"option-border-color",
	"option-cursor-normal",
	"option-cursor-hover",
	"option-text-normal",
	"option-text-hover",
	"option-text-active",
	"option-text-background",
	"option-padding",
	"option-touch-padding",
	"option-spacing",
	"option-border",
	"checkbox-normal",
	"checkbox-hover",
	"checkbox-active",
	"checkbox-border-color",
	"checkbox-cursor-normal",
	"checkbox-cursor-hover",
	"checkbox-text-normal",
	"checkbox-text-hover",
	"checkbox-text-active",
	"checkbox-text-background",
	"checkbox-padding",
	"checkbox-touch-padding",
	"checkbox-spacing",
	"checkbox-border",
	"selectable-normal",
	"selectable-hover",
	"selectable-pressed",
	"selectable-normal-active",
	"selectable-hover-active",
	"selectable-pressed-active",
	"selectable-text-normal",
	"selectable-text-hover",
	"selectable-text-pressed",
	"selectable-text-normal-active",
	"selectable-text-hover-active",
	"selectable-text-pressed-active",
	"selectable-text-background",
	"selectable-rounding",
	"selectable-padding",
	"selectable-touch-padding",
	"selectable-image-padding",
	"slider-normal",
	"slider-hover",
	"slider-active",
	"slider-border-color",
	"slider-bar-normal",
	"slider-bar-hover",
	"slider-bar-active",
	"slider-bar-filled",
	"slider-cursor-normal",
	"slider-cursor-hover",
	"slider-cursor-active",
	"slider-border",
	"slider-rounding",
	"slider-bar-height",
	"slider-padding",
	"slider-spacing",
	"slider-cursor-size",
	"progress-normal",
	"progress-hover",
	"progress-active",
	"progress-border-color",
	"progress-cursor-normal",
	"progress-cursor-hover",
	"progress-cursor-active",
	"progress-cursor-border-color",
	"progress-rounding",
	"progress-border",
	"progress-cursor-border",
	"progress-cursor-rounding",
	"progress-padding",
	"property-normal",
	"property-hover",
	"property-active",
	"property-border-color",
	"property-label-normal",
	"property-label-hover",
	"property-label-active",
	"property-border",
	"property-rounding",
	"property-padding",
	"edit-normal",
	"edit-hover",
	"edit-active",
	"edit-border-color",
	"edit-cursor-normal",
	"edit-cursor-hover",
	"edit-cursor-text-normal",
	"edit-cursor-text-hover",
	"edit-text-normal",
	"edit-text-hover",
	"edit-text-active",
	"edit-selected-normal",
	"edit-selected-hover",
	"edit-selected-text-normal",
	"edit-selected-text-hover",
	"edit-border",
	"edit-rounding",
	"edit-cursor-size",
	"edit-scrollbar-size",
	"edit-padding",
	"edit-row-padding",
	"chart-background",
	"chart-border-color",
	"chart-selected-color",
	"chart-color",
	"chart-border",
	"chart-rounding",
	"chart-padding",
	"scrollh-normal",
	"scrollh-hover",
	"scrollh-active",
	"scrollh-border-color",
	"scrollh-cursor-normal",
	"scrollh-cursor-hover",
	"scrollh-cursor-active",
	"scrollh-cursor-border-color",
	"scrollh-border",
	"scrollh-rounding",
	"scrollh-border-cursor",
	"scrollh-rounding-cursor",
	"scrollh-padding",
	"scrollv-normal",
	"scrollv-hover",
	"scrollv-active",
	"scrollv-border-color",
	"scrollv-cursor-normal",
	"scrollv-cursor-hover",
	"scrollv-cursor-active",
	"scrollv-cursor-border-color",
	"scrollv-border",
	"scrollv-rounding",
	"scrollv-border-cursor",
	"scrollv-rounding-cursor",
	"scrollv-padding",
	"tab-background",
	"tab-border-color",
	"tab-text",
	"tab-border",
	"tab-rounding",
	"tab-indent",
	"tab-padding",
	"tab-spacing",
	"combo-normal",
	"combo-hover",
	"combo-active",
	"combo-border-color",
	"combo-label-normal",
	"combo-label-hover",
	"combo-label-active",
	"combo-symbol-normal",
	"combo-symbol-hover",
	"combo-symbol-active",
	"combo-border",
	"combo-rounding",
	"combo-content-padding",
	"combo-button-padding",
	"combo-spacing",
	"window-fixed-background",
	"window-background",
	"window-border-color",
	"window-popup-border-color",
	"window-combo-border-color",
	"window-contextual-border-color",
	"window-menu-border-color",
	"window-group-border-color",
	"window-tooltip-border-color",
	"window-scaler",
	"window-border",
	"window-combo-border",
	"window-contextual-border",
	"window-menu-border",
	"window-group-border",
	"window-tooltip-border",
	"window-popup-border",
	"window-min-row-height-padding",
	"window-rounding",
	"window-spacing",
	"window-scrollbar-size",
	"window-min-size",
	"window-padding",
	"window-group-padding",
	"window-popup-padding",
	"window-combo-padding",
	"window-contextual-padding",
	"window-menu-padding",
	"window-tooltip-padding",
	"window-header-normal",
	"window-header-hover",
	"window-header-active",
	"window-header-label-normal",
	"window-header-label-hover",
	"window-header-label-active",
	"window-header-padding",
	"window-header-label-padding",
	"window-header-spacing",
	"button-text-alignment"
};



size_t GuiJSONParser::stylePropertyOffsets[] = {
	double_offsetof(nk_style, text, color),
	double_offsetof(nk_style, text, padding),
	double_offsetof(nk_style, button, normal),
	double_offsetof(nk_style, button, hover),
	double_offsetof(nk_style, button, active),
	double_offsetof(nk_style, button, border_color),
	double_offsetof(nk_style, button, text_background),
	double_offsetof(nk_style, button, text_normal),
	double_offsetof(nk_style, button, text_hover),
	double_offsetof(nk_style, button, text_active),
	double_offsetof(nk_style, button, border),
	double_offsetof(nk_style, button, rounding),
	double_offsetof(nk_style, button, padding),
	double_offsetof(nk_style, button, image_padding),
	double_offsetof(nk_style, button, touch_padding),
	double_offsetof(nk_style, contextual_button, normal),
	double_offsetof(nk_style, contextual_button, hover),
	double_offsetof(nk_style, contextual_button, active),
	double_offsetof(nk_style, contextual_button, border_color),
	double_offsetof(nk_style, contextual_button, text_background),
	double_offsetof(nk_style, contextual_button, text_normal),
	double_offsetof(nk_style, contextual_button, text_hover),
	double_offsetof(nk_style, contextual_button, text_active),
	double_offsetof(nk_style, contextual_button, border),
	double_offsetof(nk_style, contextual_button, rounding),
	double_offsetof(nk_style, contextual_button, padding),
	double_offsetof(nk_style, contextual_button, image_padding),
	double_offsetof(nk_style, contextual_button, touch_padding),
	double_offsetof(nk_style, menu_button, normal),
	double_offsetof(nk_style, menu_button, hover),
	double_offsetof(nk_style, menu_button, active),
	double_offsetof(nk_style, menu_button, border_color),
	double_offsetof(nk_style, menu_button, text_background),
	double_offsetof(nk_style, menu_button, text_normal),
	double_offsetof(nk_style, menu_button, text_hover),
	double_offsetof(nk_style, menu_button, text_active),
	double_offsetof(nk_style, menu_button, border),
	double_offsetof(nk_style, menu_button, rounding),
	double_offsetof(nk_style, menu_button, padding),
	double_offsetof(nk_style, menu_button, image_padding),
	double_offsetof(nk_style, menu_button, touch_padding),
	double_offsetof(nk_style, option, normal),
	double_offsetof(nk_style, option, hover),
	double_offsetof(nk_style, option, active),
	double_offsetof(nk_style, option, border_color),
	double_offsetof(nk_style, option, cursor_normal),
	double_offsetof(nk_style, option, cursor_hover),
	double_offsetof(nk_style, option, text_normal),
	double_offsetof(nk_style, option, text_hover),
	double_offsetof(nk_style, option, text_active),
	double_offsetof(nk_style, option, text_background),
	double_offsetof(nk_style, option, padding),
	double_offsetof(nk_style, option, touch_padding),
	double_offsetof(nk_style, option, spacing),
	double_offsetof(nk_style, option, border),
	double_offsetof(nk_style, checkbox, normal),
	double_offsetof(nk_style, checkbox, hover),
	double_offsetof(nk_style, checkbox, active),
	double_offsetof(nk_style, checkbox, border_color),
	double_offsetof(nk_style, checkbox, cursor_normal),
	double_offsetof(nk_style, checkbox, cursor_hover),
	double_offsetof(nk_style, checkbox, text_normal),
	double_offsetof(nk_style, checkbox, text_hover),
	double_offsetof(nk_style, checkbox, text_active),
	double_offsetof(nk_style, checkbox, text_background),
	double_offsetof(nk_style, checkbox, padding),
	double_offsetof(nk_style, checkbox, touch_padding),
	double_offsetof(nk_style, checkbox, spacing),
	double_offsetof(nk_style, checkbox, border),
	double_offsetof(nk_style, selectable, normal),
	double_offsetof(nk_style, selectable, hover),
	double_offsetof(nk_style, selectable, pressed),
	double_offsetof(nk_style, selectable, normal_active),
	double_offsetof(nk_style, selectable, hover_active),
	double_offsetof(nk_style, selectable, pressed_active),
	double_offsetof(nk_style, selectable, text_normal),
	double_offsetof(nk_style, selectable, text_hover),
	double_offsetof(nk_style, selectable, text_pressed),
	double_offsetof(nk_style, selectable, text_normal_active),
	double_offsetof(nk_style, selectable, text_hover_active),
	double_offsetof(nk_style, selectable, text_pressed_active),
	double_offsetof(nk_style, selectable, text_background),
	double_offsetof(nk_style, selectable, rounding),
	double_offsetof(nk_style, selectable, padding),
	double_offsetof(nk_style, selectable, touch_padding),
	double_offsetof(nk_style, selectable, image_padding),
	double_offsetof(nk_style, slider, normal),
	double_offsetof(nk_style, slider, hover),
	double_offsetof(nk_style, slider, active),
	double_offsetof(nk_style, slider, border_color),
	double_offsetof(nk_style, slider, bar_normal),
	double_offsetof(nk_style, slider, bar_hover),
	double_offsetof(nk_style, slider, bar_active),
	double_offsetof(nk_style, slider, bar_filled),
	double_offsetof(nk_style, slider, cursor_normal),
	double_offsetof(nk_style, slider, cursor_hover),
	double_offsetof(nk_style, slider, cursor_active),
	double_offsetof(nk_style, slider, border),
	double_offsetof(nk_style, slider, rounding),
	double_offsetof(nk_style, slider, bar_height),
	double_offsetof(nk_style, slider, padding),
	double_offsetof(nk_style, slider, spacing),
	double_offsetof(nk_style, slider, cursor_size),
	double_offsetof(nk_style, progress, normal),
	double_offsetof(nk_style, progress, hover),
	double_offsetof(nk_style, progress, active),
	double_offsetof(nk_style, progress, border_color),
	double_offsetof(nk_style, progress, cursor_normal),
	double_offsetof(nk_style, progress, cursor_hover),
	double_offsetof(nk_style, progress, cursor_active),
	double_offsetof(nk_style, progress, cursor_border_color),
	double_offsetof(nk_style, progress, rounding),
	double_offsetof(nk_style, progress, border),
	double_offsetof(nk_style, progress, cursor_border),
	double_offsetof(nk_style, progress, cursor_rounding),
	double_offsetof(nk_style, progress, padding),
	double_offsetof(nk_style, property, normal),
	double_offsetof(nk_style, property, hover),
	double_offsetof(nk_style, property, active),
	double_offsetof(nk_style, property, border_color),
	double_offsetof(nk_style, property, label_normal),
	double_offsetof(nk_style, property, label_hover),
	double_offsetof(nk_style, property, label_active),
	double_offsetof(nk_style, property, border),
	double_offsetof(nk_style, property, rounding),
	double_offsetof(nk_style, property, padding),
	double_offsetof(nk_style, edit, normal),
	double_offsetof(nk_style, edit, hover),
	double_offsetof(nk_style, edit, active),
	double_offsetof(nk_style, edit, border_color),
	double_offsetof(nk_style, edit, cursor_normal),
	double_offsetof(nk_style, edit, cursor_hover),
	double_offsetof(nk_style, edit, cursor_text_normal),
	double_offsetof(nk_style, edit, cursor_text_hover),
	double_offsetof(nk_style, edit, text_normal),
	double_offsetof(nk_style, edit, text_hover),
	double_offsetof(nk_style, edit, text_active),
	double_offsetof(nk_style, edit, selected_normal),
	double_offsetof(nk_style, edit, selected_hover),
	double_offsetof(nk_style, edit, selected_text_normal),
	double_offsetof(nk_style, edit, selected_text_hover),
	double_offsetof(nk_style, edit, border),
	double_offsetof(nk_style, edit, rounding),
	double_offsetof(nk_style, edit, cursor_size),
	double_offsetof(nk_style, edit, scrollbar_size),
	double_offsetof(nk_style, edit, padding),
	double_offsetof(nk_style, edit, row_padding),
	double_offsetof(nk_style, chart, background),
	double_offsetof(nk_style, chart, border_color),
	double_offsetof(nk_style, chart, selected_color),
	double_offsetof(nk_style, chart, color),
	double_offsetof(nk_style, chart, border),
	double_offsetof(nk_style, chart, rounding),
	double_offsetof(nk_style, chart, padding),
	double_offsetof(nk_style, scrollh, normal),
	double_offsetof(nk_style, scrollh, hover),
	double_offsetof(nk_style, scrollh, active),
	double_offsetof(nk_style, scrollh, border_color),
	double_offsetof(nk_style, scrollh, cursor_normal),
	double_offsetof(nk_style, scrollh, cursor_hover),
	double_offsetof(nk_style, scrollh, cursor_active),
	double_offsetof(nk_style, scrollh, cursor_border_color),
	double_offsetof(nk_style, scrollh, border),
	double_offsetof(nk_style, scrollh, rounding),
	double_offsetof(nk_style, scrollh, border_cursor),
	double_offsetof(nk_style, scrollh, rounding_cursor),
	double_offsetof(nk_style, scrollh, padding),
	double_offsetof(nk_style, scrollv, normal),
	double_offsetof(nk_style, scrollv, hover),
	double_offsetof(nk_style, scrollv, active),
	double_offsetof(nk_style, scrollv, border_color),
	double_offsetof(nk_style, scrollv, cursor_normal),
	double_offsetof(nk_style, scrollv, cursor_hover),
	double_offsetof(nk_style, scrollv, cursor_active),
	double_offsetof(nk_style, scrollv, cursor_border_color),
	double_offsetof(nk_style, scrollv, border),
	double_offsetof(nk_style, scrollv, rounding),
	double_offsetof(nk_style, scrollv, border_cursor),
	double_offsetof(nk_style, scrollv, rounding_cursor),
	double_offsetof(nk_style, scrollv, padding),
	double_offsetof(nk_style, tab, background),
	double_offsetof(nk_style, tab, border_color),
	double_offsetof(nk_style, tab, text),
	double_offsetof(nk_style, tab, border),
	double_offsetof(nk_style, tab, rounding),
	double_offsetof(nk_style, tab, indent),
	double_offsetof(nk_style, tab, padding),
	double_offsetof(nk_style, tab, spacing),
	double_offsetof(nk_style, combo, normal),
	double_offsetof(nk_style, combo, hover),
	double_offsetof(nk_style, combo, active),
	double_offsetof(nk_style, combo, border_color),
	double_offsetof(nk_style, combo, label_normal),
	double_offsetof(nk_style, combo, label_hover),
	double_offsetof(nk_style, combo, label_active),
	double_offsetof(nk_style, combo, symbol_normal),
	double_offsetof(nk_style, combo, symbol_hover),
	double_offsetof(nk_style, combo, symbol_active),
	double_offsetof(nk_style, combo, border),
	double_offsetof(nk_style, combo, rounding),
	double_offsetof(nk_style, combo, content_padding),
	double_offsetof(nk_style, combo, button_padding),
	double_offsetof(nk_style, combo, spacing),
	double_offsetof(nk_style, window, fixed_background),
	double_offsetof(nk_style, window, background),
	double_offsetof(nk_style, window, border_color),
	double_offsetof(nk_style, window, popup_border_color),
	double_offsetof(nk_style, window, combo_border_color),
	double_offsetof(nk_style, window, contextual_border_color),
	double_offsetof(nk_style, window, menu_border_color),
	double_offsetof(nk_style, window, group_border_color),
	double_offsetof(nk_style, window, tooltip_border_color),
	double_offsetof(nk_style, window, scaler),
	double_offsetof(nk_style, window, border),
	double_offsetof(nk_style, window, combo_border),
	double_offsetof(nk_style, window, contextual_border),
	double_offsetof(nk_style, window, menu_border),
	double_offsetof(nk_style, window, group_border),
	double_offsetof(nk_style, window, tooltip_border),
	double_offsetof(nk_style, window, popup_border),
	double_offsetof(nk_style, window, min_row_height_padding),
	double_offsetof(nk_style, window, rounding),
	double_offsetof(nk_style, window, spacing),
	double_offsetof(nk_style, window, scrollbar_size),
	double_offsetof(nk_style, window, min_size),
	double_offsetof(nk_style, window, padding),
	double_offsetof(nk_style, window, group_padding),
	double_offsetof(nk_style, window, popup_padding),
	double_offsetof(nk_style, window, combo_padding),
	double_offsetof(nk_style, window, contextual_padding),
	double_offsetof(nk_style, window, menu_padding),
	double_offsetof(nk_style, window, tooltip_padding),
	triple_offsetof(nk_style, window, header, normal),
	triple_offsetof(nk_style, window, header, hover),
	triple_offsetof(nk_style, window, header, active),
	triple_offsetof(nk_style, window, header, label_normal),
	triple_offsetof(nk_style, window, header, label_hover),
	triple_offsetof(nk_style, window, header, label_active),
	triple_offsetof(nk_style, window, header, padding),
	triple_offsetof(nk_style, window, header, label_padding),
	triple_offsetof(nk_style, window, header, spacing),
	double_offsetof(nk_style, button, text_alignment)
};

GuiStylePropertyType GuiJSONParser::stylePropertyTypes[] = {
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeFloat,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeItem,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeColor,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeSize,
	GuiStylePropertyTypeUInt
};