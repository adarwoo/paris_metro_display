#ifndef lum_h_HAS_ALREADY_BEEN_INCLUDED
#define lum_h_HAS_ALREADY_BEEN_INCLUDED
/**
 * @file
 * Luminosity API declaration
 * @addtogroup driver
 * @{
 * @addtogroup luminosity
 * @{
 * @author software@arreckx.com
 *****************************************************************************
 * Higher level driver for the light sensor.
 */ 

 #ifdef __cplusplus
extern "C" {
#endif

/** Initialize this driver */
void lum_init(void);

/** Grab the luminosity level straight out of the ADC */
uint8_t lum_get_raw(bool);

/** Get the filterer luminosity value */
uint8_t lum_get_filtered(void);

#ifdef __cplusplus
}
#endif

/** @} luminosity */
/** @} driver */
#endif /* ndef lum_h_HAS_ALREADY_BEEN_INCLUDED */
