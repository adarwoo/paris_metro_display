/**
 * @addtogroup core
 * @{
 * @addtogroup mode
 * @{
 * @addtogroup temperature
 * @{
 *****************************************************************************
 * This mode display the ambient temperature on the display
 *****************************************************************************
 * @file
 * Implementation of the temperature mode
 * @author software@arreckx.com
 * @internal
 */

#include "driver/fb.h"

#include "lib/alert.h"
#include "lib/singleton.hpp"

#include "core/topo.h"
#include "core/sequencer.h"
#include "core/measurements.h"

#include "display.hpp"

// ---------------------------------------------------------------------------
// Local constants
// ---------------------------------------------------------------------------
namespace
{
   /** Route used to shown degrees */
   const route_id_t TEMP_ROUTE(A3_A4);

   /** Route used to show tenth */
   const route_id_t TENTH_ROUTE(A1_A2);

   /** Led used to indicate temperature mode */
   const fb_index_t MODE_LED(POISSY);

   /** Station on the tenth route, where the display starts */
   const fb_index_t TENTH_STATION(FONTENAY_SOUS_BOIS);

   /** Reference temperature for the reference station */
   const int8_t REF_TEMP(19);

   /** Temperature from which it's hot */
   const int8_t HOT_TEMP(24);

   /** Station on the degree route where 19°C (or the ref temp) is reached */
   const fb_index_t REF_DEGREE_STATION(CHARLES_DE_GAULLE_ETOILE);
}

// ---------------------------------------------------------------------------
// Class definition
// ---------------------------------------------------------------------------
namespace display
{
   class Temperature : public lib::Singleton<IDisplay<int16_t>, Temperature>
   {
      /** Offset on route where 19°C is reached */
      tiny_index_t ref_station_offset;

      /** Offset on route where to show tenth of degrees */
      tiny_index_t tenth_station_offset;
   
   public:
      Temperature() :
         ref_station_offset(topo_get_offset(REF_DEGREE_STATION, TEMP_ROUTE)),
         tenth_station_offset(topo_get_offset(TENTH_STATION, TENTH_ROUTE))
      {}
         
      virtual timer_count_t display( int16_t temp )
      {
         register tiny_index_t i;
   
         // Turn on led to indicate temp mode
         fb_set(MODE_LED, LED_FLASH_FAST, LED_LEVEL_LOW);
   
         int8_t deg = temp/10;
         int8_t tenth = temp - deg*10;
   
         // Get the final offset
         int8_t routeFinalOffset = ref_station_offset;
         routeFinalOffset += (deg - REF_TEMP);

         if ( routeFinalOffset < 0 )
         {
            // Just flash the cold LED! very fast
            fb_set(topo_get_led(0, TEMP_ROUTE), LED_FLASH_VFAST, LED_LEVEL_FULL);
         }
         else
         {
            fb_index_t station = topo_get_led(routeFinalOffset, TEMP_ROUTE);
      
            // Light the up most station led (points to the degree)
            // Check actually valid, and if comfortable
            if ( station != TOPO_OUT_OF_RANGE )
            {
               if ( deg >= REF_TEMP && deg < HOT_TEMP )
               {
                  // Goldy lock zone
                  fb_turn_on(station);
               }
               else
               {
                  // Too hot or too cold
                  fb_set(station, LED_FLASH_VFAST, LED_LEVEL_FULL);
               }
            }

            // Light the route all the way short of the last degree
            for (i=0; i<routeFinalOffset; ++i)
            {
               station = topo_get_led(i, TEMP_ROUTE);
         
               if ( station != TOPO_OUT_OF_RANGE )
               {
                  if ( i < ref_station_offset )
                  {
                     fb_set(station, LED_ON, LED_LEVEL_LOW);
                  }
                  else switch ( i - ref_station_offset )
                  {
                     case 0: fb_set(station, LED_ON, LED_LEVEL_MED); break; // 19
                     case 1: fb_set(station, LED_ON, LED_LEVEL_MED+2); break; // 20
                     case 2: fb_set(station, LED_ON, LED_LEVEL_MED+4); break; // 21
                     case 3: fb_set(station, LED_ON, LED_LEVEL_MED+6); break; // 22
                     default:
                     fb_turn_on(station);
                  }
               }
            }
      
            // Light tenth
            for (i=0; i<tenth; ++i)
            {
               fb_index_t stationLed = topo_get_led(i + tenth_station_offset, TENTH_ROUTE);
               fb_set( stationLed, LED_ON, (i&1)?LED_LEVEL_FULL:LED_LEVEL_LOW );
            }
         }
         
         return 0;
      }
   };
}

/** Reserve static space for the singleton instance */
ALLOCATE_INSTANCE(display::Temperature);

/**@} temperature */
/**@} mode */
/**@} ---------------------------  End of file  --------------------------- */