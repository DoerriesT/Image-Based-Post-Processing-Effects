#pragma once
#include "..\nuklearInclude.h"
#include "GuiElement.h"
#include <functional>
#include "..\IIdentifiable.h"
#include "..\IDimensionable.h"
#include "..\Style\IStyleable.h"

class GuiWindow;
class GuiLayout;

class GuiRow : public IGuiEventListener, public IIdentifiable, public IStyleable, private IDimensionable
{
	friend GuiLayout;
	friend GuiWindow;
public:
	typedef std::function<void(nk_context *ctx)> LayoutCodeBlock;

	GuiRow(GuiWindow *window, const std::string &id, const std::string &height);
	~GuiRow();
	void addElement(GuiElement* element);
	void removeElement(GuiElement* element);

	GuiWindow* getWindow() const;
	void setHeight(const std::string &height);
	float getHeight() const;

	void setHidden(bool hidden);
	bool isHidden() const;
	void setRatios(std::vector<float> ratios);
	void setRatios(std::vector<std::string> ratios);
	void clearRatios();

	void layout(nk_context *ctx);
	virtual void guiEventNotification(struct GuiEvent &_event) override;

	virtual void updateDimensions() override;

	void setLayoutCodeBlock(LayoutCodeBlock &&code);
	void clearLayoutCodeBlock();
	bool hasLayoutCodeBlock() const;

	void updateColumnCount();

	template<typename T>
	bool getElementById(const char *id, T *&resultElement);

private:
	std::vector<GuiElement*> elements;
	char height[MAX_TEXT_LEN];
	float h;
	bool hidden;
	size_t columns;
	GuiWindow *window;

	bool hasLayoutCode;
	LayoutCodeBlock layoutCodeBlock;

	bool hasRatio;
	std::vector<float> ratioValues;
	std::vector<float> ratio;

	void updateRatio();
};

template<typename T>
bool GuiRow::getElementById(const char *id, T *& resultElement)
{
	if (std::is_same<T, GuiRow>::value)
	{
		if (Util::equals(this->id, id))
		{
			resultElement = (T*)this;
			return true;
		}
	}

	for (GuiElement *element : elements)
	{
		if (element->getElementById(id, resultElement))
		{
			return true;
		}
	}

	return false;
}
