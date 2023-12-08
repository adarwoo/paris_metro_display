/**
 * @addtogroup driver
 * @{
 * @addtogroup luminosity
 * @{
 *****************************************************************************
 * This driver relies on the max1036 low level API to mesure the luminosity
 *  level.
 *****************************************************************************
 * @file
 * Luminosity driver implementation
 * @author software@arreckx.com
 */
#include <asf.h>
#include "max1036.h"
#include "lum.h"

/** Depth of the luminosity filter. Powers of 2 are better since more efficient */
#define LUM_FILTER_SIZE 32

/**
 * Update the current readings
 *
 * @param value raw luminosity level
 * @return The measured value
 */
static uint8_t _update_lum(uint8_t value)
{
   // Average 16 measurement using a sliding average buffer
   static uint8_t average_buffer[LUM_FILTER_SIZE] = {0};
      
   // Keep a pointer to the next position
   static uint8_t buffer_pos = 0;
   
   // Store into the buffer
   average_buffer[buffer_pos] = value;
   
   // Rotate the pointer
   ++buffer_pos;
   buffer_pos %= LUM_FILTER_SIZE;
   
   // Add up
   register uint8_t i = 0;
   uint16_t sum = 0;
   
   for ( ; i<LUM_FILTER_SIZE; ++i )
   {
      sum += average_buffer[i];
   }

   return (sum / LUM_FILTER_SIZE);
}

/** Init the driver */
void lum_init(void)
{
   // Prepare the chip
   max1036_init(&MAX1036_TWI, 0
      | MAX1036_SCAN_SEL_gc
      | MAX1036_CHANNEL_AIN0_gc
      | MAX1036_SEL_INT_REF_ON_gc
      | MAX1036_CLOCK_EXTERNAL_gc
      | MAX1036_MODE_SINGLE_ENDED_gc
   );

   // Fill in the filter buffer
   size_t i = 0;
   int8_t l;

   // Grab reading and wait for the result
   l = max1036_read(&MAX1036_TWI, false);
   
   // Fill in N-1 raw measurements
   for ( i=0; i<LUM_FILTER_SIZE-1; ++i)
   {
      _update_lum(l);
   }
}

/** @return The filtered light level */
uint8_t lum_get_filtered(void)
{
   uint8_t l = max1036_read(&MAX1036_TWI, true);
   return _update_lum(l);
}


/**@}*/
/**@} ---------------------------  End of file  --------------------------- */