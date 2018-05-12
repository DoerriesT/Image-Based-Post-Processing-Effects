#pragma once

#include <string>
#include <cstring>
#include <assert.h>

#define MAX_TEXT_LEN 128

namespace GuiUtil
{
	inline void copy(char *dst, const char *src)
	{
		assert(strlen(src) < MAX_TEXT_LEN);
		strncpy_s(dst, MAX_TEXT_LEN, src, MAX_TEXT_LEN - 1);
		//dst[MAX_TEXT_LEN - 1] = '\0';
	}

	inline void copy(char *dst, const std::string &src)
	{
		copy(dst, src.c_str());
	}
}