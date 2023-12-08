#ifndef led_h_HAS_ALREADY_BEEN_INCLUDED
#define led_h_HAS_ALREADY_BEEN_INCLUDED
/**
 * @file
 * Defines the local LED API.
 * @author software@arreckx.com
 * @addtogroup driver
 * @{
 * @addtogroup led
 * @{
 *****************************************************************************
 * A single function API to prepare the MPU for driving its LED.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Prepare the ios for the single LED on the board */
void led_init(void);

#ifdef __cplusplus
}
#endif

/**@}*//**@}*/
#endif /* led_h_HAS_ALREADY_BEEN_INCLUDED */