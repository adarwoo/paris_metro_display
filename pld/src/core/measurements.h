#ifndef measurement_HAS_ALREADY_BEEN_INCLUDED
#define measurement_HAS_ALREADY_BEEN_INCLUDED
/**
 * @addtogroup service
 * @{
 * @addtogroup measurement
 * @{
 *****************************************************************************
 * This API manages the lower level drivers and makes background measurements.
 *****************************************************************************
 * @file
 * Definition of the measurement service API
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialise the analog measurement.
 * Following the function, both measurements are available
 */
void measurement_init(void);

/** @return The current temperature in 10th degrees */
int16_t measurement_get_temperature(void);

/** @return The current luminosity in % */
uint8_t measurement_get_luminosity(void);

/** @return true if the ambient light is dark */
bool measurement_luminosity_is_dark(void);

#ifdef __cplusplus
}
#endif

/** @} */
/** @} */
#endif /* ndef measurement_HAS_ALREADY_BEEN_INCLUDED */
