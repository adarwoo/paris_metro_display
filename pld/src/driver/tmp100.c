/**
 * @file
 * Texas tmp100 chip low level driver implementation
 * @internal
 * @addtogroup driver
 * @{
 * @addtogroup tmp100
 * @{
 * @author gax
 *****************************************************************************
 * This driver provide basic access to the data.
 * The chip i2c address must be provided in full through the TMP100_I2C_ADDR
 *  macro.
 * The twi interface is not initialised by this API.
 */

#include <asf.h>
#include "tmp100.h"

/** 
 * Grab the raw value from the temperature sensor
 *
 * @param twi Pointer to the two wire device used
 * @param no_wait True to use a value filled in by the i2c interrupt
 * @return The raw temperature as a signed 16-bits matching the value
 *          returned by the sensor
 */
int16_t tmp100_read(TWI_t *twi, bool no_wait)
{
   static uint8_t data_received[2];
   int16_t raw = 0;
   
   // Not waiting means we rely on the last values   
   if ( no_wait )
   {
      // Convert to signed number
      raw = data_received[0]<<8 | data_received[1];
   }
         
   twi_package_t packet_read =
   {
     .chip        = TMP100_I2C_ADDR, // TWI slave bus address
     .addr        = {0},
     .addr_length = 1, // Length of the address
     .buffer      = data_received,  // Storage area
     .length      = 2,               // transfer data size (bytes)
     .no_wait    = no_wait
   };

   twi_master_read(twi, &packet_read);

   // If waiting use the value here and now   
   if ( ! no_wait )
   {
      // Convert to signed number
      raw = data_received[0]<<8 | data_received[1];
   }         
   
   // Remove unused bits
   return raw >> 4;
}

/** Init the driver */
void tmp100_init(TWI_t *twi)
{
   // Issue a general call to the tmp100 to reset it
   twi_package_t packet = {
      .chip        = 0, // General call
      .addr        = {0b110},
      .addr_length = 0,
      .buffer      = 0,
      .length      = 0,
      .no_wait     = false
   };
   
   twi_master_write(twi, &packet);
    
   // Set the precision as 12-bits
   uint8_t data[] = { 0b01100000 };
   
   twi_package_t config = {
      .chip         = TMP100_I2C_ADDR,      // TWI slave bus address
      .addr         = {0b01},
      .addr_length  = 1,   // TWI slave memory address data size
      .buffer       = data,
      .length       = 1,  // transfer data size (bytes)
      .no_wait      = false
   };
 
   twi_master_write(twi, &config);
}

 /**@}*/
 /**@} ---------------------------  End of file  --------------------------- */