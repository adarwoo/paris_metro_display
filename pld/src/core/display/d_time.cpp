/**
 * @addtogroup core
 * @{
 * @addtogroup mode
 * @{
 * @addtogroup time
 * @{
 *****************************************************************************
 * The time display shows the hours and minutes using the lines A1_A2.
 *****************************************************************************
 * @file
 * Implementation of the time mode
 * @author software@arreckx.com
 * @internal
 */

#include "driver/fb.h"
#include "lib/tz.h"
#include "lib/singleton.hpp"
#include "core/topo.h"
#include "display.hpp"

// ---------------------------------------------------------------------------
// Local constants
// ---------------------------------------------------------------------------
namespace
{
   /** Led used to indicate slice of the day */
   const fb_index_t _TIME_MORNING_STATION = MAISONS_LAFFITTE;
   const fb_index_t _TIME_AFTERNOON_STATION = SARTROUVILLE;
   const fb_index_t _TIME_EVENING_STATION = HOUILLES_CARRIERE_SUR_SEINE;

   /** Route used to shown the part of the slice */
   const route_id_t _TIME_HOURS_ROUTE = A3_A4;

   /** Leds to mark minutes tenths */
   const route_id_t _TIME_TENTH_MINUTES_ROUTE = A1_A2;
   const fb_index_t _TIME_TENTH_MINUTES_FIRST = SAINT_GERMAIN_EN_LAYE;

   /** Leds to mark minutes decimals */
   const route_id_t _TIME_DEC_MINUTES_ROUTE = A1_A2;
   const fb_index_t _TIME_DEC_MINUTES_FIRST = FONTENAY_SOUS_BOIS;

   /** Led used to indicate time mode */
   const fb_index_t _TIME_MODE_LED = ACHERES_GRAND_CORMIER;
   
   /** Interval in ms for the chaser when waiting for a valid clock */
   const fb_index_t _TIME_WAITING_CHASER_INTERVAL = TIMER_MILLISECONDS(250);
}


// ---------------------------------------------------------------------------
// Class definition
// ---------------------------------------------------------------------------
namespace display
{
   class Time : public lib::Singleton<IDisplay<tz_datetime_t *>, Time>
   {
      /** Offset for the tenth of a minute position */
      tiny_index_t tenth_offset;

      /** Offset for the decimal value of a minute position */
      tiny_index_t dec_offset;
   public:
      Time() :
         tenth_offset(topo_get_offset(_TIME_TENTH_MINUTES_FIRST, _TIME_TENTH_MINUTES_ROUTE)),
         dec_offset(topo_get_offset(_TIME_DEC_MINUTES_FIRST, _TIME_DEC_MINUTES_ROUTE))
      {}
         
      void showTimeMode()
      {
         // Turn on led to indicate time mode
         fb_set(_TIME_MODE_LED, LED_FLASH_FAST, LED_LEVEL_LOW);
      }         
         
      void showHours( tiny_index_t hours )
      {
         fb_index_t station;
         tiny_index_t i;
         
         // Fill in slice
         if ( hours < 6 )
         {
            // Nothing
         }
         else if ( hours < 12 )
         {
            fb_set(_TIME_MORNING_STATION, LED_ON, LED_LEVEL_LOW);
            hours -= 6;
         }
         else if ( hours < 18 )
         {
            fb_set(_TIME_AFTERNOON_STATION, LED_ON, LED_LEVEL_LOW);
            hours -= 12;
         }
         else
         {
            fb_set(_TIME_EVENING_STATION, LED_ON, LED_LEVEL_LOW);
            hours -= 18;
         }
         
         // Fill in the hours
         for (i=0; i<hours; ++i)
         {
            station = topo_get_led(i, _TIME_HOURS_ROUTE);
            fb_set( station, LED_ON, (i&1)?LED_LEVEL_FULL:LED_LEVEL_LOW );
         }
      }
      
      void showMinutes( tiny_index_t minutes )
      {
         fb_index_t station;
         tiny_index_t i;
         
         // Fill in tenth of minutes
         tiny_index_t tenth = (minutes / 10);
         
         for (i=0; i<tenth; ++i)
         {
            station = topo_get_led(i+tenth_offset, _TIME_TENTH_MINUTES_ROUTE);
            fb_set( station, LED_ON, (i&1)?LED_LEVEL_FULL:LED_LEVEL_LOW );
         }

         // Fill in minutes
         tiny_index_t dec = (minutes % 10);
         
         for (i=0; i<dec; ++i)
         {
            station = topo_get_led(dec_offset+i, _TIME_DEC_MINUTES_ROUTE);
            fb_set( station, LED_ON, (i&1)?LED_LEVEL_FULL:LED_LEVEL_LOW );
         }
      }
      
      timer_count_t display(tz_datetime_t *pDate) override
      {
         showTimeMode();
         showHours(pDate->hour);
         showMinutes(pDate->minute);
         
         return TIMER_SECONDS(1);
      }
   };
}

/** Reserve static space for the singleton instance */
ALLOCATE_INSTANCE(display::Time);

 /**@}*/
 /**@}*/
 /**@} ---------------------------  End of file  --------------------------- */