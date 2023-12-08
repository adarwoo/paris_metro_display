#include "stdafx.h"

#include "core/measurements.h"

namespace {
   int16_t temp = 196;
}

// For simulation only
void measurement_set_temperature(int16_t t)
{
   temp = t;
}

extern "C"
{
	/**
	* Initialise the analog measurement.
	* Following the function, both measurements are available
	*/
	void measurement_init(void)
	{}

	/** @return The current temperature in 10th degrees */
	int16_t measurement_get_temperature(void)
	{
		return temp;
	}

	/** @return The current luminosity in % */
	uint8_t measurement_get_luminosity(void)
	{
		return 13;
	}

	/** @return true if the ambient light is dark */
	bool measurement_luminosity_is_dark(void)
	{
		return false;
	}

}