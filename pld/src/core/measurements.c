/**
 * @addtogroup service
 * @{
 * @addtogroup measurement
 * @{
 *****************************************************************************
 * API to grab all the measurement that take place in the background.
 * Perform background analog measurements for the temperature
 *  and the ambient light level.
 * The values are available at all time as the measurements take place in the
 *  background.
 *****************************************************************************
 * @file
 * Implementation of the measurement service API
 * @author software@arreckx.com
 */ 

#include <asf.h>

#include "driver/lum.h"
#include "driver/temperature.h"

#include "lib/timer.h"

#include "core/measurements.h"

/************************************************************************/
/* Local defines                                                        */
/************************************************************************/

/**
 * @def _LUM_DARK_THRESHOLD
 * Threshold in % of light sensor to switch off
 */
#ifndef _LUM_DARK_THRESHOLD
#  define _LUM_DARK_THRESHOLD 14
#endif

/**
 * @def MEASUREMENT_ALTERNATE_PERIOD
 * Period to alternate measurements
 */
#ifndef MEASUREMENT_ALTERNATE_PERIOD
#  define MEASUREMENT_ALTERNATE_PERIOD TIMER_MILLISECONDS(250)
#endif

/**
 * @def TWI_BUS_SPEED
 * Normal i2c for all 
 */
#ifndef TWI_BUS_SPEED
#  define TWI_BUS_SPEED 100000
#endif

/************************************************************************/
/* Local variables                                                      */
/************************************************************************/

/** Current filtered temperature */
static int16_t filtered_temperature = 0;

/** Current filtered luminosity */
static uint8_t filtered_luminosity = 0;

//
// For debug, allow override
//
#ifdef DEBUG
bool override_measurements = false;
#endif

/** Type of measurement */
typedef enum measurement_type_enum {
   measurement_type_temperature=0,
   measurement_type_luminosity=1
} measurement_type_t;

/************************************************************************/
/* Local functions                                                      */
/************************************************************************/

/** Helper to initialise the i2c bus as master */
static void _init_twi_bus_as_master( TWI_t *twi )
{
   // Start the twi as i2c master running at 400KHz (fast i2c)
   twi_options_t opt = {
      .speed = TWI_BUS_SPEED,
      .chip  = 0,  // We do not reply, but only drive      .speed_reg = TWI_BAUD(sysclk_get_cpu_hz(), TWI_BUS_SPEED)   };
   
   sysclk_enable_peripheral_clock(twi);
   twi_master_init(twi, &opt);
   twi_master_enable(twi);
}
   
/** 
 * Self repeating function in charge of alternating measuring the temperature
 *  and the light flux.
 * The results are stored in the local storage.
 */
static void _make_a_measurement( timer_instance_t ti, void *arg )
{
   measurement_type_t type = (measurement_type_t)arg;
   measurement_type_t next;

   #ifdef DEBUG
   if ( ! override_measurements )
   #endif
   
   switch ( type )
   {
      case measurement_type_temperature:
         next=measurement_type_luminosity;
         filtered_temperature = temperature_get_filtered();
         break;
      case measurement_type_luminosity:
      default:
         next=measurement_type_temperature;
         filtered_luminosity = lum_get_filtered();
   }
   
   timer_arm_from_now( &_make_a_measurement, MEASUREMENT_ALTERNATE_PERIOD, (void *)next);
}

/************************************************************************/
/* Public functions                                                     */
/************************************************************************/

/** @return The current temperature in 10th degrees */
int16_t measurement_get_temperature(void)
{
   return filtered_temperature;
}

/** @return The current luminosity in % */
uint8_t measurement_get_luminosity(void)
{
   return filtered_luminosity;
}

bool measurement_luminosity_is_dark(void)
{
   return filtered_luminosity < _LUM_DARK_THRESHOLD;
}

/**
 * Initialise the analog measurement.
 * Following the function, both measurements are available
 */
void measurement_init(void)
{
   // Prepare the twi as master
   _init_twi_bus_as_master(&TMP100_TWI);
   
   // Also init the lum i2c unless it is shared with the tmp100
   if ( &MAX1036_TWI != &TMP100_TWI )
   {
      _init_twi_bus_as_master(&MAX1036_TWI);
   }
   
   // Initialise the temperature API
   temperature_init();
   filtered_temperature=temperature_get_filtered();
   
   // Initialise the luminosity API
   lum_init();
   filtered_luminosity=lum_get_filtered();

   // Alternate measuring temperature and luminosity
   timer_arm_from_now( 
      &_make_a_measurement, 
      MEASUREMENT_ALTERNATE_PERIOD, 
      (void*)measurement_type_temperature
   );
}

/**@}*/
/**@} ---------------------------  End of file  --------------------------- */