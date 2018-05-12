#pragma once
#include <rapidjson\rapidjson.h>
#include <vector>
#include <glm\glm.hpp>
#include "GuiRow.h"
#include "..\nuklearInclude.h"
#include "..\GuiEvent.h"
#include "..\Style\IStyleable.h"
#include "..\IIdentifiable.h"
#include "..\IDimensionable.h"

class GuiLayout;
class GuiJSONParser;

class GuiWindow : public IGuiEventListener, public IIdentifiable, public IStyleable, private IDimensionable
{
	friend GuiLayout;
	friend GuiJSONParser;
public:
	GuiWindow(GuiLayout *layout, const std::string &id);
	~GuiWindow();
	void addRow(GuiRow* row);
	void removeRow(GuiRow* row);

	void layout(struct nk_context *ctx);
	void setCentered(bool centered);

	void setPosition(const std::string &x, const std::string &y);
	void setSize(const std::string &w, const std::string &h);

	void setX(const std::string &x);
	void setY(const std::string &y);
	void setWidth(const std::string &w);
	void setHeight(const std::string &h);
	const struct nk_rect getBounds() const;
	bool isWindowArea(float x, float y);

	void setTitle(const std::string &title);
	void setBordered(bool bordered);

	void set3DPosition(float x, float y, float z);
	void clear3DPosition();
	
	void setFlag(nk_flags flag, bool enabled);
	bool getFlag(nk_flags flag) const;
	void setHidden(bool hidden);
	bool isHidden() const;
	void focus();

	void setScrollbarHidden(bool scrollbarHidden);

	void setDisabled(bool disabled);
	bool isDisabled() const;
	void updateDisabled();

	void setParent(GuiWindow *window);
	GuiWindow* getParent() const;
	GuiLayout* getParentLayout() const;

	virtual void updateDimensions() override;
	virtual void guiEventNotification(struct GuiEvent &_event) override;

	template<typename T>
	bool getElementById(const char *id, T *&resultElement);
	int64_t userData;

private:
	std::vector<GuiRow*> rows;
	GuiWindow *parentWindow;
	GuiLayout *parentLayout;
	struct nk_rect bounds;
	nk_flags flags;

	char height[MAX_TEXT_LEN];
	char width[MAX_TEXT_LEN];
	char x[MAX_TEXT_LEN];
	char y[MAX_TEXT_LEN];
	char title[MAX_TEXT_LEN];
	bool centered;
	bool disabled;
	bool hidden;

	glm::vec4 position3D;
};


template<typename T>
bool GuiWindow::getElementById(const char *id, T *&resultElement)
{
	if (std::is_same<T, GuiWindow>::value)
	{
		if (Util::equals(this->id, id))
		{
			resultElement = (T*)this;
			return true;
		}
	}
	else
	{
		for(GuiRow *row : rows)
		{
			if (row->getElementById(id, resultElement))
			{
				return true;
			}
		}
	}
	return false;
}