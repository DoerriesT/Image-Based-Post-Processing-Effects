#pragma once

/**
* A IKeyListener will be informed of any keys that are pressed or released. Keys are non-character keys such as
* "CTRL".
*/
class IKeyListener
{
public:
	/**
	* Callback that is invoked when a key is pressed or released.
	*
	* @param key
	*            The key code of the key.
	* @param action
	*            Either RELEASE, PRESS or REPEAT, indicating what action was performed on the key.
	*/
	virtual void onKey(const int &_key, const int &_action) = 0;
};