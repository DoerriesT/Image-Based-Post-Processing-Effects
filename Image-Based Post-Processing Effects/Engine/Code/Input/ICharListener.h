#pragma once
#include "InputTokens.h"

/**
* A ICharListener will be informed of any character keys that are pressed or released. Character keys are keys
* such as "F".
*/
class ICharListener
{
public:
	/**
	* Callback that is invoked when a character key is pressed.
	*
	* @param key
	*            The key code of the key.
	*/
	virtual void onChar(InputKey _key) = 0;
};