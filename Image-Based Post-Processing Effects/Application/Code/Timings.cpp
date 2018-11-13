#include "Timings.h"

App::Timings &App::Timings::operator+=(const Timings &rhs)
{
	// Careful, this only works if the struct is made up exclusively of doubles.
	// TODO: find less hackey method
	for (size_t i = 0; i < sizeof(Timings) / sizeof(double); ++i)
	{
		((double *)this)[i] += ((double *)&rhs)[i];
	}

	return *this;
}

App::Timings &App::Timings::operator*=(double rhs)
{
	// Careful, this only works if the struct is made up exclusively of doubles.
	// TODO: find less hackey method
	for (size_t i = 0; i < sizeof(Timings) / sizeof(double); ++i)
	{
		((double *)this)[i] *= rhs;
	}

	return *this;
}
