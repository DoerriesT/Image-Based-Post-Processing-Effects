#define _USE_MATH_DEFINES
#include <cmath>
#include "EasingFunctions.h"

double linear(double _passedTime, double _totalDuration)
{
	return _passedTime / _totalDuration;
}

double quadraticEasingIn(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration;
	return _passedTime * _passedTime;
}

double quadraticEasingOut(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration;
	return -_passedTime * (_passedTime - 2.0);
}

double quadraticEasingInOut(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration * 0.5;
	if (_passedTime < 1.0f)
	{
		return 0.5 * _passedTime * _passedTime;
	}

	--_passedTime;
	return -0.5 * (_passedTime * (_passedTime - 2.0) - 1.0);
}

double cubicEasingIn(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration;
	return _passedTime * _passedTime * _passedTime;
}

double cubicEasingOut(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration;
	--_passedTime;
	return _passedTime * _passedTime * _passedTime + 1.0;
}

double cubicEasingInOut(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration * 0.5;
	if (_passedTime < 1.0)
	{
		return 0.5 * _passedTime * _passedTime * _passedTime;
	}
	else
	{
		_passedTime -= 2.0;
		return 0.5 * (_passedTime * _passedTime * _passedTime + 2.0);
	}
}

double quarticEasingIn(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration;
	return _passedTime * _passedTime * _passedTime * _passedTime;
}

double quarticEasingOut(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration;
	--_passedTime;
	return -(_passedTime * _passedTime * _passedTime * _passedTime - 1.0);
}

double quarticEasingInOut(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration * 0.5;
	if (_passedTime < 1.0)
	{
		return 0.5 * _passedTime * _passedTime * _passedTime * _passedTime;
	}
	_passedTime -= 2.0;
	return -0.5 * (_passedTime * _passedTime * _passedTime * _passedTime - 2.0f);

}

double quinticEasingIn(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration;
	return _passedTime * _passedTime * _passedTime * _passedTime * _passedTime;
}

double quinticEasingOut(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration;
	--_passedTime;
	return _passedTime * _passedTime * _passedTime * _passedTime * _passedTime + 1.0;
}

double quinticEasingInOut(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration * 0.5;
	if (_passedTime < 1.0)
	{
		return 0.5 * _passedTime * _passedTime * _passedTime * _passedTime * _passedTime;
	}
	else
	{
		_passedTime -= 2.0;
		return 0.5 * (_passedTime * _passedTime * _passedTime * _passedTime * _passedTime + 2.0);
	}

}

double sinusoidalEasingOut(double _passedTime, double _totalDuration)
{
	return -cos(_passedTime / _totalDuration * (M_PI / 2.0)) + 1.0;
}

double sinusoidalEasingInOut(double _passedTime, double _totalDuration)
{
	return sin(_passedTime / _totalDuration * (M_PI / 2.0));
}

double exponentialEasingIn(double _passedTime, double _totalDuration)
{
	return pow(2.0, 10.0 * (_passedTime / _totalDuration - 1.0));
}

double exponentialEasingOut(double _passedTime, double _totalDuration)
{
	return -pow(2.0, -10.0 * _passedTime / _totalDuration) + 1.0;
}

double exponentialEasingInOut(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration * 0.5;
	if (_passedTime < 1.0)
	{
		return 0.5 * pow(2.0, 10.0 * (_passedTime - 1.0));
	}
	--_passedTime;
	return 0.5 * (-pow(2.0, -10.0 * _passedTime) + 2.0);

}

double circularEasingIn(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration;
	return -(sqrt(1.0 - _passedTime * _passedTime) - 1.0);
}

double circularEasingOut(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration;
	--_passedTime;
	return sqrt(1.0 - _passedTime * _passedTime);
}

double circularEasingInOut(double _passedTime, double _totalDuration)
{
	_passedTime /= _totalDuration * 0.5;
	if (_passedTime < 1.0)
	{
		return -0.5 * (sqrt(1.0 - _passedTime * _passedTime) - 1.0);
	}
	_passedTime -= 2.0;
	return 0.5 * (sqrt(1.0 - _passedTime * _passedTime) + 1.0);

}