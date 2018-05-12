#pragma once

/**
* A ScrollListener will be informed of any mouse scrolling.
*/
class IScrollListener
{
public:
	/**
	* Callback that is invoked on mouse scrolling
	*
	* @param xOffset
	*            The scrolling offset on the x axis.
	* @param yOffset
	*            The scrolling offset on the y axis.
	*/
	virtual void onScroll(const double &_xOffset, const double &_yOffset) = 0;
};