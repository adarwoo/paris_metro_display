/**
 * @file
 * Implementation of the GPS management
 * @addtogroup service
 * @{
 * @addtogroup gps
 * @{
 *****************************************************************************    
 * Handle the GPS data.
 * The GPS is used to synchronize the internal RTC. 
 * Since the board may not have a crystal for the RTC (but an RC oscillator),
 *  the RTC clock is expected to drift badly and is not allowed to go
 *  un-synced for more than 1 hour.
 * We the system starts, the internal RTC is set to 0 and a flag is
 *  set to indicate the RTC clock is not synced with the GPS time.
 * When a sync occurs, it is only valid for 1 hour.
 * In the mean time, the GPS is put to sleep for 10 minutes to save power.
 * After 10 minutes, it's turned back on until it yields a valid time again.
 * The internal RTC is only updated if the time drift exceeds a given number
 *  of seconds.
 */ 

/** Maximum drift allowed before updating the RTC clock */
#define MAX_SECONDS_DIFFERENCE_TO_UPDATE_RTC 10

/** Hour to add (or subtract if negative) to the UTC time for local time */
#define TZ_HOUR_ADJUST 1

/** Period in which to check that the GPS is still alive */
#ifndef DEBUG
#  define GPS_HEALTH_CHECK_PERIOD TIMER_HOURS(2)
#else
#  define GPS_HEALTH_CHECK_PERIOD TIMER_MINUTES(2)
#endif

#include "lib/gps.h"

extern "C" {

#include <asf.h>
#include <sio2host.h>

#include <math.h>

#include "lib/tz.h"
#include "lib/timer.h"
#include "lib/reactor.h"

#include "core/gps_manager.h"

/** Local GPS object */
TinyGPS gps;

/** Flag to signal that the GPS has sent a valid string - therefore it must be ready */
static bool _gps_time_is_initialized = false;

/** Flag to signal the time has been synced with the GPS time */
static bool _gps_time_is_synchronized = false;

/** Reactor handle */
reactor_handle_t _gps_reactor_handle = 0;

/** Called by the timer interrupt to notify the reactor in all cases */
static void _kick_reactor(void)
{
   reactor_notify(_gps_reactor_handle);
}

#ifdef DEBUG
/** Helper for debugging - set the time by setting this to 1 */
uint8_t override_time_once = 0;
#endif

/** 
 * Called periodically to check that the GPS data is still occasionally being received. 
 * If not, invalidate the RTC clock as to avoid displaying drifting time.
 * Case : If the GPS or its signal die, the RTC will slowly drift and eventually, the
 *  time shown will be badly wrong.
 * We need to receive the clock from the GPS from time to time.
 * This function rearm itself.
 */
void _gps_check_gps_data_age( timer_instance_t i, void *arg )
{
   if ( ! _gps_time_is_synchronized )
   {
      // Invalidate the system clock
      rtc_set_time(0);
   }
   
   // Reset the flag
   _gps_time_is_synchronized = false;

   // Re-arm   
   timer_arm_from_now( &_gps_check_gps_data_age, GPS_HEALTH_CHECK_PERIOD, 0 );
}

/** Process incoming NMEA data from the GPS receiver */
static void _gps_update(void)
{
   int c;
   uint8_t day, month, hour, minute, second, hundreth;
   int year;
   unsigned long fix_age;
   uint32_t epochGps;
   
   while ( (c=sio2host_getchar_nowait()) != -1 )
   {
      if ( gps.encode((char)c) )
      {
         // Configure the GPS once it's up and running
         if ( ! _gps_time_is_initialized )
         {
            gps_configure();
            _gps_time_is_initialized = true;
            
            // Do not decode this frame as the module is not configured the way we want
            // Wait for the next
            return;
         }

         gps.crack_datetime(
            &year, &month, &day, &hour, &minute, &second, &hundreth, &fix_age
         );
         
#ifdef DEBUG
         // For testing, override the time once by setting override_time_once to 1
         if ( override_time_once > 0 )
         {
            if ( override_time_once == 1 )
            {
               struct calendar_date date = {
                  .second=second,
                  .minute=minute,
                  .hour=hour,
                  .date=day, // First day of month is 0
                  .month=month, // First month of year is 0
                  .year=(uint16_t)year
               };
         
               epochGps = tz_convert_to_cet( calendar_date_to_timestamp( &date ) );
               calendar_timestamp_to_date( epochGps, &date );
               
               // ************************************************************
               // Break point HERE and modify date
               // ************************************************************
               rtc_set_time( calendar_date_to_timestamp( &date ) );
               
               ++override_time_once;
            }
         }
         else
#endif         
         
         // If the month and the day are 0 - we only have the time
         if ( month!=0 && day!=0 )
         {
            // Adjust day and month to be 0 based index as opposed to 1.
            // So, January becomes 0 (rather than 1 as returned by the GPS NMEA)
            --month;
            --day;
         
            // Convert to Unix epoch
            struct calendar_date date = {
               .second=second,
               .minute=minute,
               .hour=hour,
               .date=day, // First day of month is 0
               .month=month, // First month of year is 0
               .year=(uint16_t)year
            };
         
            // Convert to CET (winter) epoch time
            epochGps = tz_convert_to_cet( calendar_date_to_timestamp( &date ) );
            uint32_t epochRtc = rtc_get_time();

            // Different?
            if ( abs(epochGps - epochRtc) > MAX_SECONDS_DIFFERENCE_TO_UPDATE_RTC )
            {
               // Update the RTC clock
               rtc_set_time(epochGps);
            }
         
            // Indicate the system time has been synchronized
            _gps_time_is_synchronized = true;
         }
      }         
   }
}

/************************************************************************/
/* Public API                                                           */
/************************************************************************/

/** 
 * Prepare the gps manager 
 * The ASF API is a bit short when it comes to interactive buffering,
 *  and in particular, it does not allow to sniff the buffer or hook a
 *  callback for when the buffer has data waiting to be processed.
 * Not mentioning the lack of transmit buffer...
 * So we use a timer to check the buffer on a regular basis.
 */
void gps_manager_init(void)
{
   // Arm the timer
   // Enable the timer
   tc_enable( &GPS_TIMER_TC );

   // Simple timer compare mode required
   tc_set_wgm( &GPS_TIMER_TC, TC_WG_NORMAL );

   // The timer service interrupts run at low level to allow
   //  other interrupts to take precedence
   tc_set_overflow_interrupt_level( &GPS_TIMER_TC, TC_INT_LVL_MED );

   // Set the callback
   tc_set_overflow_interrupt_callback( &GPS_TIMER_TC, &_kick_reactor );

   // Aiming for 100ms, which at 9600 baud is around 10 characters
   
   // Set the top to get 10Hz ( 32000000/1024/3125 )
   tc_write_period( &GPS_TIMER_TC, 3125 );
   tc_write_clock_source( &GPS_TIMER_TC, TC_CLKSEL_DIV1024_gc );
   
   // Register with the reactor for power saving
   _gps_reactor_handle = reactor_register(&_gps_update);
   
   // Reset the GPS to make sure it is not turned off or starting
   // We start configuring once it gets talking as this
   //  software is up and running much faster that the GPS from cold start.
   ioport_configure_pin(GPS_N_RESET, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
   
   // Recommended delay >10ms from datasheet of L26
   delay_ms(20);
   
   // Back to running.
   ioport_set_pin_high(GPS_N_RESET);
   
   // Check that the GPS has received valid data
   timer_arm_from_now( &_gps_check_gps_data_age, GPS_HEALTH_CHECK_PERIOD, 0 );
}

void gps_configure(void)
{
   // Initialise the GPS mode
   puts_P( PSTR("$PMTK353,1,0*36\r\n") );   
}

void gps_activate(void)
{
   // Send a wakeup
   puts_P( PSTR("$PMTK225,0*2B\r\n") );
}

void gps_standby(void)
{
   // Send
   puts_P( PSTR("$PMTK161,0*28\r\n") );
}

} // End of extern "C"

 /**@}*/
 /**@}*/