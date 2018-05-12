#pragma once

#ifdef _DEBUG
#include <string.h>
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
//USAGE: DBG_LOG(const char *format, ...), example: DBG_LOG("pos: %d, %d", x, y)
#define DBG_LOG(...) printf("%s - %s[%d] - %s()\n-> ", __TIME__, __FILENAME__, __LINE__, __FUNCTION__); printf(##__VA_ARGS__); printf("\n");
#else
#define DBG_LOG(...)
#endif