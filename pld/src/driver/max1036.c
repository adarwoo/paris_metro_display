/**
 * @file
 * Implementation of the max1036 driver
 * @internal
 * @addtogroup driver
 * @{
 * @addtogroup max1036
 * @{
 * @author software@arreckx.com
 *****************************************************************************
 * Perform background analog measurements for the temperature
 *  and the ambient light level.
 * The values are available at all time as the measurements take place in the
 *  background.
 */ 
#include <asf.h>
#include "max1036.h"

/** 
 * Initialise the max1036.
 * The twi interface MUST already be initialised along with its clock, pins etc.
 * The configuration bits are spread between the config and setup register.
 * No initial conversion takes place.
 *
 * @param twi Pointer to the device being used
 * @param setupAndConfig Bit fields used to apply the config.
 */
void max1036_init( TWI_t *twi, uint16_t setupAndConfig )
{
   uint8_t data[] = {
      (MAX1036_REGISTER_SETUP_gc | MAX1036_RESET_CONFIG_gc | setupAndConfig) & 0xFF,
      (MAX1036_REGISTER_CONFIG_gc | setupAndConfig) >> 8
   };
   
   twi_package_t config = {
      .chip         = MAX1036_I2C_ADDR,   // TWI slave bus address
      .addr         = {1},
      .addr_length  = 0,                  // TWI slave memory address data size
      .buffer       = data,
      .length       = sizeof(data),       // transfer data size (bytes)
      .no_wait      = false
   };
   
   // Transmit
   twi_master_write(twi, &config);
}

/** 
 * Grab the raw adc value
 *
 * @param twi Pointer to the device being used
 * @param no_wait True to use a value filled in by the i2c interrupt
 * @return The raw 8-bit value
 */
uint8_t max1036_read( TWI_t *twi, bool no_wait)
{
   static uint8_t data_received[1];
   uint8_t retval;
   
   twi_package_t packet_read =
   {
      .chip        = MAX1036_I2C_ADDR, // TWI slave bus address
      .addr        = {0},
      .addr_length = 0,                // Address of the length
      .buffer      = data_received,    // Storage area
      .length      = sizeof(data_received), // transfer data size (bytes)
      .no_wait     = no_wait
   };
   
   // Not waiting means we rely on the last values
   if ( no_wait )
   {
      // Convert to signed number
      retval = data_received[0];
   }
   
   twi_master_read(twi, &packet_read);
   
   // If waiting use the value here and now
   if ( ! no_wait )
   {
      // Convert to signed number
      retval = data_received[0];
   }
   
   return retval;
}

/**@}*/
/**@} ---------------------------  End of file  --------------------------- */