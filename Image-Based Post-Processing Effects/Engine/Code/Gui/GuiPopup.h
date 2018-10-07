#pragma once
#include <string>
#include <vector>
#include "IIdentifiable.h"
#include "Elements\GuiElement.h"

class GuiPopup : public IIdentifiable
{
	friend class GuiLayout;
public:
	int64_t userData;
	bool errorPopup;

	enum GuiPopupDefaultButtons
	{
		OK = 0,
		OK_CANCEL = 1,

		YES = 0,
		NO = 1,
		YESNO_CANCEL = 2,
	};

	static GuiPopup createCustomPopup(const std::string &id, const std::vector<std::string> &buttons, const std::string &message, const std::string &title = "")
	{
		return GuiPopup(id, title, { message }, buttons);
	}

	static GuiPopup createOkCancelComboBoxPopup(const std::string &id, const std::vector<std::string> &messages, const std::vector<std::vector<std::string>> &comboOptions, const std::vector<size_t> &comboSelections = {}, const std::string &title = "")
	{
		return createCustomComboBoxPopup(id, { "Ok", "Cancel" }, messages, comboOptions, comboSelections, title);
	}

	static GuiPopup createCustomComboBoxPopup(const std::string &id, const std::vector<std::string> &buttons, const std::vector<std::string> &messages, const std::vector<std::vector<std::string>> &comboOptions, const std::vector<size_t> &comboSelections = {}, const std::string &title = "")
	{
		GuiPopup popup(id, title, messages, buttons);

		popup.comboOptions = comboOptions;
		popup.comboSelections = comboSelections;

		// make options and selections the same size
		while (popup.comboSelections.size() < popup.comboOptions.size())
		{
			popup.comboSelections.push_back(0);
		}
		while (popup.comboSelections.size() > popup.comboOptions.size())
		{
			popup.comboSelections.pop_back();
		}
		assert(comboOptions.size() <= 4);
		return popup;
	}

	static GuiPopup createCustomInputPopup(const std::string &id, const std::vector<std::string> &buttons, const std::vector<std::string> &messages, const std::vector<std::string> &inputDefaults, const std::vector<GuiTextFieldType> &inputTypes = {}, const std::string &title = "")
	{
		return GuiPopup(id, title, messages, buttons, inputDefaults, inputTypes);
	}

	static GuiPopup createYesNoPopup(const std::string &id, const std::string &message, const std::string &title = "Question")
	{
		return GuiPopup(id, title, { message }, { "Yes", "No" });
	}

	static GuiPopup createYesNoCancelPopup(const std::string &id, const std::string &message, const std::string &title = "Question")
	{
		return GuiPopup(id, title, { message }, { "Yes", "No", "Cancel" });
	}

	static GuiPopup createErrorPopup(const std::string &id, const std::string &message, const std::string &title = "Error")
	{
		GuiPopup popup(id, title, { message }, { "Ok" });
		popup.errorPopup = true;
		return popup;
	}

	static GuiPopup createOkPopup(const std::string &id, const std::string &message, const std::string &title = "Info")
	{
		return GuiPopup(id, title, { message }, { "Ok" });
	}

	static GuiPopup createOkCancelPopup(const std::string &id, const std::string &message, const std::string &title = "Info")
	{
		return GuiPopup(id, title, { message }, { "Ok" , "Cancel" });
	}

	static GuiPopup createOkCancelInputPopup(const std::string &id, const std::string &message, const std::string &inputDefault = "", const std::string &title = "Info")
	{
		GuiPopup popup(id, title, { message }, { "Ok" , "Cancel" }, { inputDefault }, { GuiTextFieldType::Text });
		return popup;
	}

	static GuiPopup createOkInputPopup(const std::string &id, const std::string &message, const std::string &inputDefault = "", const std::string &title = "Info")
	{
		GuiPopup popup(id, title, { message }, { "Ok" }, { inputDefault }, { GuiTextFieldType::Text });
		return popup;
	}


	std::string getTextField(size_t pos = 0) const
	{
		if (pos >= getTextFieldCount())
		{
			return "";
		}
		else
		{
			return textfields.at(pos);
		}
	}

	std::vector<std::string> getTextFields() const
	{
		return textfields;
	}

	size_t getTextFieldCount() const
	{
		return textfields.size();
	}

	bool isInputPopup() const
	{
		return !textfields.empty();
	}

	std::vector<std::vector<std::string>> getComboOptions() const
	{
		return comboOptions;
	}

	std::vector<size_t> getComboSelections() const
	{
		return comboSelections;
	}

	size_t getComboSelection(size_t pos = 0) const
	{
		return comboSelections[pos];
	}

	size_t getComboCount() const
	{
		return comboSelections.size();
	}

private:
	std::string title;
	std::vector<std::string> messages;
	std::vector<std::string> buttons;
	std::vector<std::string> textfields;
	std::vector<GuiTextFieldType> textfieldTypes;

	std::vector<std::vector<std::string>> comboOptions;
	std::vector<size_t> comboSelections;


	GuiPopup(const std::string &id, const std::string &title,
			 const std::vector<std::string> &messages,
			 const std::vector<std::string> &buttons,
			 const std::vector<std::string> &textfields = {},
			 const std::vector<GuiTextFieldType> &textfieldTypes = {})
		: IIdentifiable(id), 
		title(title), 
		messages(messages), 
		buttons(buttons),
		textfields(textfields), 
		textfieldTypes(textfieldTypes),
		errorPopup(false),
		userData()
	{
		assert(messages.size() <= 4);
		assert(textfields.size() <= 4);

		while (this->textfieldTypes.size() < textfields.size())
		{
			this->textfieldTypes.push_back(GuiTextFieldType::Text);
		}
	}
};