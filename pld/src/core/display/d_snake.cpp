/**
 * @addtogroup core
 * @{
 * @addtogroup mode
 * @{
 * @addtogroup snake
 * @{
 *****************************************************************************
 * A snake like display which lights the LEDs and fades them
 *****************************************************************************
 * @file
 * Implementation of the snake display
 * @author software@arreckx.com
 * @internal
 */
#include "core/topo.h"
#include "driver/fb.h"
#include "lib/singleton.hpp"
#include "display.hpp"

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

namespace
{
   /** Number of LEDs in the chaser */
   constexpr uint8_t CHASER_COUNT = 16;

   /** Upper limit for the chaser */
   constexpr uint8_t CHASER_UPPER = CHASER_COUNT-1;

   /** Period of update of the chaser */
   constexpr timer_count_t chaser_period = TIMER_MILLISECONDS(50);

   /** Journey stations */
   constexpr route_id_t journey[] = {A5_A2, A4_A1, A3_A4, INVALID_ROUTE};
}

// ---------------------------------------------------------------------------
// Class definition
// ---------------------------------------------------------------------------

namespace display
{
   class Snake : public lib::Singleton<IDisplay<>, Snake>
   {
      /** Index of the currently operating route */
      tiny_index_t route_index = 0;

      /** Index of the station within a route */
      tiny_index_t station_index = 0;

      /** Keep a local copy of the frame buffer */
      fb_mem_t our_fb;

      /**
       * Keep a sorted list of all the active leds in the chaser
       * The position in the chaser is the light level.
       * Items are inserted right to left, then rotated right to left
       */
      fb_index_t chaser[CHASER_COUNT] = { 0 };
   
   public:
      void reset() override
      {
         // Start from 0
         route_index = station_index = 0;
      
         // Reset the chaser memory
         memset( chaser, TOPO_OUT_OF_RANGE, CHASER_COUNT );

         // Reset the local fb
         fb_use(&our_fb);
         fb_clear();
      }
      
      timer_count_t display(void *arg) override
      {
         timer_count_t retval = chaser_period;
         fb_use(&our_fb);

         //
         // Rotate the buffer and apply dimming
         //
         tiny_index_t i = 0;
         bool allOff = true;
   
         // Rotate the chaser buffer right to left
         // 0<-1 ; 1<-2 ; 2<-3 ; ... ; 14<-15
         while ( i < CHASER_UPPER )
         {
            fb_index_t toDim = chaser[i];
      
            // Last LED is turned off
            if ( toDim != TOPO_OUT_OF_RANGE )
            {
               if ( i == 0 )
               {
                  fb_turn_off(toDim);
               }
               else
               {
                  // Dim
                  fb_set(toDim, LED_ON, i);
                  allOff = false;
               }
            }
      
            // and shift for next time
            chaser[i] = chaser[i+1];
      
            // Increment the index!!
            ++i;
         }
   
         route_id_t route = journey[route_index];
   
         //
         // Grab next current led
         //
         if ( route != INVALID_ROUTE )
         {
            // Grab the next station
            fb_index_t nextLed = topo_get_led(station_index++, route);
      
            // End of the route
            if (nextLed == TOPO_OUT_OF_RANGE)
            {
               // Next route in our list
               route = journey[++route_index];
         
               if ( route != INVALID_ROUTE )
               {
                  station_index=0;

                  nextLed = topo_get_led(0, route);
                  fb_set(nextLed, LED_ON, LED_LEVEL_FULL);
               }
            }
            else
            {
               fb_set(nextLed, LED_ON, LED_LEVEL_FULL);
            }
      
            chaser[CHASER_UPPER] = nextLed;
         }
         else
         {
            if ( allOff )
            {
               // That's it until the buffer is empty
               retval = 0;
            }
         }
         
         return retval;
      }
   };
}

/** Reserve static space for the singleton instance */
ALLOCATE_INSTANCE(display::Snake);

/**@} snake*/
/**@} core */
/**@} ---------------------------  End of file  --------------------------- */