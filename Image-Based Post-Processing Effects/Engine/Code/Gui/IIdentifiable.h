#pragma once
#include <string>
#include "GuiUtility.h"

class IIdentifiable
{
public:
	const char* getId() const
	{
		return id;
	}

	IIdentifiable(const std::string &id)
	{
		GuiUtil::copy(this->id, id);
	}

protected:
	char id[MAX_TEXT_LEN];
};
