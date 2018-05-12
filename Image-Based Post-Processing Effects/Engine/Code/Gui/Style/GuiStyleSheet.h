#pragma once
#include <map>
#include <vector>
#include <string>
#include "GuiStyleBlock.h"

class IStyleable;

class GuiStyleSheet
{
	friend IStyleable;
public:
	GuiStyleSheet();
	~GuiStyleSheet();
	void clear();

	void addClassProperty(const std::string &name, GuiStyleProperty *prop);

	GuiStyleBlock* getClassBlock(const std::string &name);

	void addListener(IStyleable *l);
	void removeListener(IStyleable *l);

	GuiStyleSheet& operator=(const GuiStyleSheet &other);
	GuiStyleSheet& operator+=(const GuiStyleSheet &other);

private:
	std::map<std::string, GuiStyleBlock> classes;
	std::vector<IStyleable*> listener;

	void notifyListener();
};

