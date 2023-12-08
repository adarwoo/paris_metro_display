/**
 * @addtogroup service
 * @{
 * @addtogroup timezone
 * @{
 *****************************************************************************
 * This API can work out the local time for a CET time zone.
 * Also added is the possibility to check a date is valid, that is, it is more
 *  recent than the date this product was put in service.
 *****************************************************************************
 * @file
 * Implementation of the timezone API
 * @author software@arreckx.com
 * @internal
 */

#include "tz.h"

/** @cond simulator_only */
// Prototype to be added to the simulator
extern uint32_t rtc_get_time(void);
/** @endcond */

// Months are 0 indexed

/** Index of the march month */
#define MARCH_MONTH   2

/** Index of the october month */
#define OCTOBER_MONTH 9

/** Number of seconds in 1 hour */
#define SECONDS_IN_ONE_HOUR (60*60)

/**
 * Helper to determine if the given UTC time is within CET summer time or not.
 * @param epochIn Epoch UTC time
 * @return true if summer time
 */
static bool tz_is_summer_time( const uint32_t epochIn )
{
   struct calendar_date date;

   // We need the day of week - convert back
   calendar_timestamp_to_date(epochIn, &date);
 
   // Quickly rule out cases outside of the month
   if (date.month < MARCH_MONTH || date.month > OCTOBER_MONTH)
   {
      return false; 
   }
   
   if (date.month > MARCH_MONTH && date.month < OCTOBER_MONTH)
   {
      return true; 
   }
   
   // Need to workout if the day is before the last sunday of the week
   int previousSunday = date.date - date.dayofweek;

   if (date.month == MARCH_MONTH)
   {
      return (previousSunday >= 25) && (date.hour > 1);
   }
   
   if (date.month == OCTOBER_MONTH) 
   {
      return (previousSunday < 25) && (date.hour > 1);
   }

   return false; // this line never gonna happend
}

/**
 * Given a epoch time in UTC, adjust it to become a local CET time.
 * The conversion takes into account DST.
 *
 * @param epochIn Time to convert. This time is UTC.
 * @return A local epoch time
 */
uint32_t tz_convert_to_cet( uint32_t epochIn )
{
   bool isSummer = tz_is_summer_time(epochIn);
   
   if ( isSummer )
   {
      epochIn += (2 * SECONDS_IN_ONE_HOUR);
   }
   else
   {
      epochIn += SECONDS_IN_ONE_HOUR;
   }

   return epochIn;
}

/**
 * The result is given in the correct timezone.
 *
 * @param pDate Pointer to the structure to hold the result
 * @return The same pointer as pDate or 0 if the rtc returned value is
 *          too old to be true.
 */
tz_datetime_t *tz_now( tz_datetime_t *pDate )
{
   uint32_t timeNow = rtc_get_time();
   
   if ( ! tz_time_is_valid(timeNow) )
   {
      return 0;
   }
   
   // Localize
   tz_convert_to_cet(timeNow);

   // Convert to a date
   calendar_timestamp_to_date( timeNow, pDate );
   
   return pDate;
}

 /**@} group timezone */
 /**@} group service */
 /*---------------------------  End of file  --------------------------- */