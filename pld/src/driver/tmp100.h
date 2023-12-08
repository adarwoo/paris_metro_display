#ifndef tmp100_h_HAS_ALREADY_BEEN_INCLUDED
#define tmp100_h_HAS_ALREADY_BEEN_INCLUDED
/**
 * @file
 * Header for the tmp100 API
 * @addtogroup driver
 * @{
 * @addtogroup tmp100
 * @{
 *****************************************************************************
 * Low level interface to the Texas TMP100/TMP101 chip.
 * This chip measures the temperature and dialogues over an i2c bus
 * @author gax
 */ 
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialise the driver */
void tmp100_init(TWI_t *twi);

/** Read a single value */
int16_t tmp100_read(TWI_t *twi, bool no_wait);

#ifdef __cplusplus
}
#endif

/**@}*/
/**@}*/
#endif /* ndef tmp100_h_HAS_ALREADY_BEEN_INCLUDED */