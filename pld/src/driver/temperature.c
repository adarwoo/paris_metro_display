/**
 * @addtogroup driver
 * @{
 * @addtogroup temperature
 * @{
 *****************************************************************************
 * Access to the system filtered temperature.
 *****************************************************************************
 * @file
 * Implementation of the temperature driver
 * @author software@arreckx.com
 * @internal
 */
#include <asf.h>

#include "temperature.h"
#include "tmp100.h"

/**
 * @def TEMPERATURE_FILTER_SIZE
 * Size of the sliding average filter buffer for the temperature
 */
#ifndef TEMPERATURE_FILTER_SIZE
   #define TEMPERATURE_FILTER_SIZE 16
#endif

/**
 * Update the current temperature readings
 *
 * @param raw_temp raw temperature
 * @return The average temperature in 10th of a degrees
 */
static int16_t _update_temperature(int16_t raw_temp)
{
   // Average 16 measurement using a sliding average buffer
   static int16_t average_buffer[TEMPERATURE_FILTER_SIZE] = {0};
      
   // Keep a pointer to the next position
   static uint8_t buffer_pos = 0;
   
   // Store into the buffer
   average_buffer[buffer_pos] = raw_temp;
   
   // Rotate the pointer
   buffer_pos = (buffer_pos + 1) % TEMPERATURE_FILTER_SIZE;
   
   // Add up
   register uint8_t i = 0;
   int16_t sum = 0;
   
   for ( ; i<TEMPERATURE_FILTER_SIZE; ++i )
   {
      sum += average_buffer[i];
   }
   
   // The raw result is a 12bit representing 1/16th of a degree
   // We return 1/10th of a degree
   int16_t retval = (sum * 10l) / (TEMPERATURE_FILTER_SIZE * 16);

   return retval;
}

/** @return The filtered temperature */
int16_t temperature_get_filtered(void)
{
   int16_t t = tmp100_read(&TMP100_TWI, true);
   
   // Use the last measurement
   return _update_temperature(t);
}

/** Initialise the temperature API */
void temperature_init(void)
{
   register uint_fast8_t i = 0;
   int16_t t;
   
   // Init the device driver
   tmp100_init(&TMP100_TWI);

   // Grab temperature and wait for the result
   t = tmp100_read(&TMP100_TWI, false);
   
   // Fill in N-1 raw measurements
   for ( i=0; i<TEMPERATURE_FILTER_SIZE-1; ++i)
   {
      _update_temperature(t);
   }
}

 /**@}*/
 /**@} ---------------------------  End of file  --------------------------- */