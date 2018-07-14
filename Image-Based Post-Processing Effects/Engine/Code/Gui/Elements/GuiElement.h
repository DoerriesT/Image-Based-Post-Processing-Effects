#pragma once

#include "..\nuklearInclude.h"
#include "..\GuiEvent.h"
#include <string>
#include "..\..\Utilities\Utility.h"
#include "..\IIdentifiable.h"
#include "..\IDimensionable.h"
#include "..\Style\IStyleable.h"

class GuiRow;
class GuiLayout;

enum GuiElementType
{
	GuiElementLabelType,
	GuiElementMultilineLabelType,
	GuiElementButtonType,
	GuiElementGroupType,
	GuiElementTextFieldType,
	GuiElementComboBoxType,
	GuiElementSpacingType,
	GuiElementSliderType,
	GuiElementCheckBoxType,
	GuiElementRadioButtonListType,
};

enum class GuiToggleType
{
	RadioButton,
	CheckBox
};

enum class GuiTextFieldType
{
	Text,
	Integer,
	Float
};

class GuiElement : public IGuiEventListener, public IIdentifiable, public IStyleable, private IDimensionable
{
	friend GuiLayout;
protected:
	GuiRow *row;
	virtual void elementLayout(nk_context *ctx) = 0;

public:
	GuiElement(GuiRow *row, const std::string &id);
	virtual ~GuiElement();
	void layout(nk_context *ctx);
	virtual GuiElementType getType() = 0;
	GuiRow* getParentRow() const;

	virtual void updateDimensions() override;
	virtual void guiEventNotification(struct GuiEvent &_event) override;

	virtual size_t getColumns() const
	{
		return 1;
	}

	template<typename T>
	bool getElementById(const char *id, T *&resultElement);
};


class GuiSpacing : public GuiElement
{
private:
	int columns;

public:
	GuiSpacing(GuiRow *row, const std::string &id, int columns);
	virtual void elementLayout(nk_context *ctx) override;

	virtual GuiElementType getType() override
	{
		return GuiElementSpacingType;
	};

	void setColumns(int columns);

	virtual size_t getColumns() const override
	{
		return columns;
	}
};

class GuiButton : public GuiElement
{
private:
	char text[MAX_TEXT_LEN];

public:
	GuiButton(GuiRow *row, const std::string &id, const std::string &text);
	void setText(const std::string &text);

	virtual void elementLayout(nk_context *ctx) override;


	virtual GuiElementType getType() override
	{
		return GuiElementButtonType;
	};
};

class GuiLabel : public GuiElement
{
protected:
	char text[MAX_TEXT_LEN];
	nk_text_alignment align;
	bool usesFontOffset;
	int fontOffset;

public:
	GuiLabel(GuiRow *row, const std::string &id, const std::string &text, const nk_text_alignment align = NK_TEXT_CENTERED);
	virtual void elementLayout(nk_context *ctx) override;

	void setFontOffset(int offset);
	void unsetFontOffset();

	void setText(const std::string &text);

	virtual GuiElementType getType() override
	{
		return GuiElementLabelType;
	};
};


class GuiWrapLabel : public GuiLabel
{
public:
	GuiWrapLabel(GuiRow *row, const std::string &id, const std::string &text);
	virtual void elementLayout(nk_context *ctx) override;
};

class GuiMultilineLabel : public GuiElement
{
private:
	std::vector<std::string> lines;
	nk_text_alignment align;

public:
	GuiMultilineLabel(GuiRow *row, const std::string &id, const std::string &text, const nk_text_alignment align);
	virtual void elementLayout(nk_context *ctx) override;

	void setText(const std::string &text);

	virtual GuiElementType getType() override
	{
		return GuiElementMultilineLabelType;
	};
};

class GuiTextField : public GuiElement
{
private:
	char *text;
	nk_flags flags;
	int textLength;
	int maxTextLength;
	nk_plugin_filter filter;

public:
	GuiTextField(GuiRow *row, const std::string &id, const std::string &text, GuiTextFieldType type);
	GuiTextField(GuiRow *row, const std::string &id, const std::string &text, GuiTextFieldType type, int maxTextLength, nk_flags flags);
	~GuiTextField();
	virtual void elementLayout(nk_context *ctx) override;

	void setType(GuiTextFieldType type);
	int getTextLength() const;
	void setText(const std::string &text);
	std::string getText() const;
	int getInteger() const;
	float getFloat() const;

	virtual GuiElementType getType() override
	{
		return GuiElementTextFieldType;
	};
};

class GuiComboBox : public GuiElement
{
private:
	char **items;
	size_t selectedItem;
	size_t itemCount;

public:
	GuiComboBox(GuiRow *row, const std::string &id);
	~GuiComboBox();
	virtual void elementLayout(nk_context *ctx) override;

	void setItems(const std::vector<std::string> &items);
	size_t getItemCount() const;
	const char* getItemName(const size_t &item) const;
	const char* getSelectedItemName() const;
	size_t getSelectedItem();

	void setSelectedItem(const size_t &item);
	void clearItems();

	virtual GuiElementType getType() override
	{
		return GuiElementComboBoxType;
	};
};

class GuiGroup : public GuiElement
{
private:
	std::vector<GuiRow*> rows;
	char title[MAX_TEXT_LEN];

public:
	nk_scroll scroll;
	nk_flags flags;

	GuiGroup(GuiRow *row, const std::string &id);
	~GuiGroup();
	virtual void elementLayout(nk_context *ctx) override;

	void setTitle(const std::string &title);
	void addRow(GuiRow *row);
	void removeRow(GuiRow *row);
	void setBordered(bool bordered);
	void setTitled(bool titled);

	virtual void updateDimensions() override;

	virtual GuiElementType getType() override
	{
		return GuiElementGroupType;
	};

	template<typename T>
	bool getElementById(const char *id, T *&resultElement);
};

class GuiSlider : public GuiElement
{
private:
	float min, max, value, step;

public:
	GuiSlider(GuiRow *row, const std::string &id);
	~GuiSlider();
	virtual void elementLayout(nk_context *ctx) override;

	void setMin(float min);
	void setMax(float max);
	void setValue(float value);
	void setStep(float step);

	float getMin() const;
	float getMax() const;
	float getValue() const;
	float getStep() const;

	virtual GuiElementType getType() override
	{
		return GuiElementSliderType;
	};
};

class GuiToggle : public GuiElement
{
private:
	int checked;
	bool grouped;
	char text[MAX_TEXT_LEN];
	const GuiToggleType type;

public:
	GuiToggle(GuiRow *row, const std::string &id, const std::string &text, const std::string &group, GuiToggleType type, bool checked);
	~GuiToggle();
	virtual void elementLayout(nk_context *ctx) override;

	void setText(const std::string &text);
	void setChecked(bool checked);
	bool isChecked() const;

	const std::string group;

	virtual GuiElementType getType() override
	{
		return GuiElementCheckBoxType;
	};
};


template<typename T>
bool GuiElement::getElementById(const char *id, T *&resultElement)
{
	if (Utility::equals(this->id, id))
	{
		if (resultElement = dynamic_cast<T*>(this))
		{
			return true;
		}
	}

	if (getType() == GuiElementGroupType)
	{
		GuiGroup *group = (GuiGroup*)this;
		return group->getElementById(id, resultElement);
	}

	return false;
}

template<typename T>
inline bool GuiGroup::getElementById(const char *id, T *& resultElement)
{
	for (GuiRow *row : rows)
	{
		if (row->getElementById(id, resultElement))
		{
			return true;
		}
	}
	return false;
}
