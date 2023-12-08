#ifndef temperature_h_HAS_ALREADY_BEEN_INCLUDED
#define temperature_h_HAS_ALREADY_BEEN_INCLUDED
/**
 * @file
 * Temperature API definition
 * @addtogroup driver
 * @{
 * @addtogroup temperature
 * @{
 *****************************************************************************
 * Higher level API to measure the temperature.
 * This API relies on the tmp100 API and provides additional filtering.
 * The filter depth can be adjusted by overriding the macro 
 * @author gax
 */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void temperature_init(void);
int16_t temperature_get_filtered(void);

#ifdef __cplusplus
}
#endif

/**@}*/
/**@}*/
#endif /* ndef temperature_h_HAS_ALREADY_BEEN_INCLUDED */