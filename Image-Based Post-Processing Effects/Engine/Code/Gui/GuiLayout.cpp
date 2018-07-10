#include "GuiLayout.h"
#include <cassert>
#include "GuiJSONParser.h"
#include "Gui.h"
#include "Utilities\Utility.h"
#include "EntityComponentSystem\SystemManager.h"
#include "EntityComponentSystem\Systems\RenderSystem.h"
#include "Window\Window.h"
#include "Graphics\Camera.h"

const char *POPUP_COMBO[] = { "POPUP_COMBO_0", "POPUP_COMBO_1", "POPUP_COMBO_2", "POPUP_COMBO_3" };
const char *POPUP_INPUT[] = { "POPUP_INPUT_0", "POPUP_INPUT_1", "POPUP_INPUT_2", "POPUP_INPUT_3" };
const char *POPUP_MESSAGE[] = { "POPUP_MESSAGE_0", "POPUP_MESSAGE_1", "POPUP_MESSAGE_2", "POPUP_MESSAGE_3" };

const char *POPUP_COMBO_ROW[] = { "POPUP_COMBO_ROW_0", "POPUP_COMBO_ROW_1", "POPUP_COMBO_ROW_2", "POPUP_COMBO_ROW_3" };
const char *POPUP_INPUT_ROW[] = { "POPUP_INPUT_ROW_0", "POPUP_INPUT_ROW_1", "POPUP_INPUT_ROW_2", "POPUP_INPUT_ROW_3" };
const char *POPUP_MESSAGE_ROW[] = { "POPUP_MESSAGE_ROW_0", "POPUP_MESSAGE_ROW_1", "POPUP_MESSAGE_ROW_2", "POPUP_MESSAGE_ROW_3" };

using GuiUtil::copy;

GuiLayout::GuiLayout(const std::string &id)
	: IIdentifiable(id), disabled(false), popupWindow(nullptr)
{
	ctx = new nk_context();
}

GuiLayout::~GuiLayout()
{
	nk_free(ctx);
	delete ctx;
	delete popupWindow;

	for (GuiWindow *window : windows)
	{
		delete window;
	}
}



void GuiLayout::init()
{
	nk_init_default(ctx, 0);

	// remove combo box arrow background
	ctx->style.combo.button.normal = nk_style_item_hide();
	ctx->style.combo.button.hover = nk_style_item_hide();
	ctx->style.combo.button.active = nk_style_item_hide();
}

nk_context* GuiLayout::layout()
{
	applyStyle(ctx);

	for (GuiWindow *window : windows)
	{
		window->layout(ctx);
	}

	//layout popups
	if (popupWindow)
	{
		popupWindow->layout(ctx);
	}

	return ctx;
}

nk_context* GuiLayout::getContext()
{
	return ctx;
}

void GuiLayout::setEventListener(IGuiEventListener *listener)
{
	eventListener = listener;
}

void GuiLayout::addWindow(GuiWindow *window)
{
	if (!window)
		return;

	clearInput();
	if (!contains(windows, window))
	{
		assert(isUnique(window));
		windows.push_back(window);
	}
}

void GuiLayout::removeWindow(GuiWindow *window)
{
	remove(windows, window);
}


void GuiLayout::updateDimensions()
{
	//TODO: use ctx->style for more accurate calcs
	for (GuiWindow *window : windows)
	{
		window->updateDimensions();
	}

	if (popupWindow)
		popupWindow->updateDimensions();


	update3DWindows(true);
}

void GuiLayout::guiEventNotification(GuiEvent &_event)
{
	if (_event.type == GuiEventType::Popup)
	{
		removePopup();
	}

	if (eventListener)
	{
		if (!_event.source)
		{
			_event.source = this;
		}
		_event.sourceLayout = this;
		eventListener->guiEventNotification(_event);
	}
}

void GuiLayout::setDisabled(bool disabled)
{
	this->disabled = disabled;

	for (GuiWindow *window : windows)
	{
		window->updateDisabled();
	}
}

bool GuiLayout::getDisabled() const
{
	return disabled;
}

void GuiLayout::displayPopup(const GuiPopup &popup)
{
	Gui::notificationSound();

	clearInput();
	popups.push_back(popup);
	updatePopup();
}

void GuiLayout::removePopup()
{
	popups.erase(popups.begin()); // remove front
	updatePopup();
}

void GuiLayout::removePopup(const std::string &id)
{
	popups.erase(std::remove_if(popups.begin(), popups.end(),
								[id](GuiPopup p)
	{
		return id == p.getId();
	}), popups.end());

	updatePopup();
}

void GuiLayout::addToggle(GuiToggle *toggle)
{
	toggleMap[toggle->group].push_back(toggle);
}

void GuiLayout::removeToggle(GuiToggle *toggle)
{
	remove(toggleMap[toggle->group], toggle);
}

void GuiLayout::clearToggleGroup(const std::string &group)
{
	for (GuiToggle *t : toggleMap[group])
	{
		t->setChecked(false);
	}
}

void GuiLayout::updatePopup()
{
	const bool hasPopup = !popups.empty();
	if (hasPopup)
	{
		if (!popupWindow)
		{
			popupWindow = new GuiWindow(this, "_POPUP_");
			popupWindow->setWidth("15*r+15*s");
			popupWindow->setPosition("0", "0");
			popupWindow->setFlag(NK_WINDOW_NO_SCROLLBAR, true);
			popupWindow->setCentered(true);

			for (int i = 0; i<4; ++i)
			{
				GuiRow *row;
				row = new GuiRow(popupWindow, POPUP_MESSAGE_ROW[i], "r");
				row->addElement(new GuiWrapLabel(row, POPUP_MESSAGE[i], ""));
				popupWindow->addRow(row);

				row = new GuiRow(popupWindow, POPUP_INPUT_ROW[i], "r");
				row->addElement(new GuiTextField(row, POPUP_INPUT[i], "", GuiTextFieldType::Text));
				popupWindow->addRow(row);

				row = new GuiRow(popupWindow, POPUP_COMBO_ROW[i], "r");
				row->addElement(new GuiComboBox(row, POPUP_COMBO[i]));
				popupWindow->addRow(row);
			}
			popupWindow->addRow(new GuiRow(popupWindow, "POPUP_BUTTON_ROW", "r"));
		}

		popupWindow->setHidden(false);
		popupWindow->focus();

		/* update popup window content */
		GuiPopup *popup = &popups.front();
		popupWindow->setTitle(popup->title);
		popupWindow->setStyleClass(popup->errorPopup ? "error-popup" : "");

		size_t messageCount = popup->messages.size();
		size_t textFieldCount = popup->getTextFieldCount();
		size_t comboBoxCount = popup->comboOptions.size();

		bool standardPopup = messageCount == 1 && comboBoxCount == 0 && !popup->isInputPopup();

		if (standardPopup)
		{
			popupWindow->setHeight("5*r+4*s");
		}
		else
		{
			std::string rows = std::to_string(2 + messageCount + textFieldCount + comboBoxCount);
			std::string space = std::to_string(3 + messageCount + textFieldCount + comboBoxCount);
			popupWindow->setHeight(rows+"*r+"+space+"*s");
		}

		// scale first message row
		GuiRow *row;
		if (popupWindow->getElementById(POPUP_MESSAGE_ROW[0], row))
		{
			if (standardPopup)
			{
				row->setHeight("3*r");
			}
			else
			{
				row->setHeight("r");
			}
		}

		// fill all popup rows
		for (size_t i = 0; i<4; ++i)
		{
			if (popupWindow->getElementById(POPUP_MESSAGE_ROW[i], row))
			{
				row->setHidden(i >= messageCount);
				GuiWrapLabel *label;
				if (i < messageCount && popupWindow->getElementById(POPUP_MESSAGE[i], label))
				{
					label->setText(popup->messages.at(i));
				}
			}
			if (popupWindow->getElementById(POPUP_INPUT_ROW[i], row))
			{
				row->setHidden(i >= textFieldCount);
				GuiTextField *textField;
				if (i < textFieldCount && popupWindow->getElementById(POPUP_INPUT[i], textField))
				{
					textField->setText(popup->getTextField(i));
					textField->setType(popup->textfieldTypes.at(i));
				}
			}
			if (popupWindow->getElementById(POPUP_COMBO_ROW[i], row))
			{
				row->setHidden(i >= comboBoxCount);
				GuiComboBox *comboBox;
				if (i < comboBoxCount && popupWindow->getElementById(POPUP_COMBO[i], comboBox))
				{
					comboBox->setItems(popup->comboOptions[i]);
					comboBox->setSelectedItem(popup->comboSelections[i]);
				}
			}
		}

		if (popupWindow->getElementById("POPUP_BUTTON_ROW", row))
		{
			row->setLayoutCodeBlock([this, row, popup](nk_context *ctx)
			{
				const size_t size = popup->buttons.size();
				nk_layout_row_dynamic(ctx, Gui::dimensions.rowHeight, (int)size);
				for (size_t i = 0; i < size; ++i)
				{
					if (nk_button_label(ctx, popup->buttons[i].c_str()))
					{
						size_t textFieldCount = popup->getTextFieldCount();
						size_t comboBoxCount = popup->comboOptions.size();
						for (size_t i = 0; i < textFieldCount; ++i)
						{
							GuiTextField *textField;
							if (popupWindow->getElementById(POPUP_INPUT[i], textField))
							{
								popup->textfields[i] = textField->getText();
							}
						}
						for (size_t e = 0; e < comboBoxCount; ++e)
						{
							GuiComboBox *comboBox;
							if (popupWindow->getElementById(POPUP_COMBO[e], comboBox))
							{								
								popup->comboSelections[e] = comboBox->getSelectedItem();
							}
						}
						GuiPopup popupCopy = *popup; // this is needed because the popup is deleted before it is served to the listeners
						row->guiEventNotification(GuiEvent(GuiEventType::Popup, &popupCopy, (int)i));
						break;
					}
				}
			});
		}
	}
	else if (popupWindow)
	{
		popupWindow->setHidden(true);
		GuiRow *row;
		if (popupWindow->getElementById("_POPUP_CONTENT_", row))
		{
			row->clearLayoutCodeBlock();
		}
	}

	setDisabled(hasPopup);
}

void GuiLayout::clearInput()
{
	memset(&ctx->input, 0, sizeof(nk_input));
}

void GuiLayout::add3DWindow(GuiWindow *window)
{
	if (!contains(windows3D, window))
	{
		windows3D.push_back(window);
	}
	update3DWindows(true);
}

void GuiLayout::remove3DWindow(GuiWindow *window)
{
	remove(windows3D, window);
	window->updateDimensions();
}

void GuiLayout::update3DWindows(bool forceUpdate)
{
	static glm::mat4 lastView;
	static glm::mat4 lastProj;

	RenderSystem *renderSystem = SystemManager::getInstance().getSystem<RenderSystem>();
	auto camera = renderSystem->getActiveCamera();
	glm::mat4 proj = renderSystem->getWindow()->getProjectionMatrix();
	glm::mat4 view;

	if (camera)
	{
		view = camera->getViewMatrix();
	}

	// only update if needed
	if (!forceUpdate && view == lastView && proj == lastProj)
	{
		return;
	}
	lastProj = proj;
	lastView = view;

	for (GuiWindow *window: windows3D)
	{
		glm::vec4 pos(proj * view * window->position3D);

		struct nk_rect &bounds = window->bounds;

		// hide windows that are behind the camera
		if (pos.z <= 0.0f)
		{
			struct nk_rect &bounds = window->bounds;
			bounds.w = 0;
			bounds.h = 0;
			continue;
		}

		const float windowWidth = Gui::dimensions.windowWidth;
		const float windowHeight = Gui::dimensions.windowHeight;

		bounds.x = (pos.x/pos.w  + 1.0f) * 0.5f * windowWidth - bounds.w * 0.5f;
		bounds.y = (-pos.y/pos.w + 1.0f) * 0.5f * windowHeight - bounds.h * 0.5f;

		//const float spacing = 20.0f;
		//if (x + width > windowWidth - spacing)
		//	x = windowWidth - width - spacing;
		//else if (x < spacing)
		//	x = spacing;
		//if (y < spacing)
		//	y = spacing;
		//else if (y + height > windowHeight - spacing)
		//	y = windowHeight - spacing - height;

		//if (x < blockedConerWidth && y < blockedCornerHeight)
		//	if (x < y)
		//		y = blockedCornerHeight;
		//	else
		//		x = blockedCornerWidth;
	}
}
