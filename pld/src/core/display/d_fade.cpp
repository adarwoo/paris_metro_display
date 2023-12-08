/**
 * @addtogroup core
 * @{
 * @addtogroup mode
 * @{
 * @addtogroup fade
 * @{
 *****************************************************************************
 * Turn on all LEDs and fade them
 *****************************************************************************
 * @file
 * Implementation of the fade mode
 * @author software@arreckx.com
 * @internal
 */

#include "driver/fb.h"
#include "core/topo.h"
#include "lib/singleton.hpp"

#include "display.hpp"

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

namespace
{
   /** Period of update of the dimmer */
   const timer_count_t TIME_FADE_NEXT_CHANGE = TIMER_MILLISECONDS(100);

   /** Time to wait between state changes in ms */
   const timer_count_t TIME_WAIT_IN_BETWEEN = TIMER_MILLISECONDS(1500);
}

// ---------------------------------------------------------------------------
// Class definition
// ---------------------------------------------------------------------------
namespace display
{
   class Fade : public lib::Singleton<IDisplay<>, Fade>
   {
      /** On-going level */
      uint8_t level = 0;

      /** Current stage (SM) */
      enum { brightning_e, dimming_e, pausing_e } _stage = brightning_e;
   public:
      void reset() override
      {
         // Start from 0
         level = 0;
            
         // And stage reset
         _stage = brightning_e;
      }
      
      timer_count_t display(void *arg) override
      {
         tiny_index_t i;
         timer_count_t period = TIME_FADE_NEXT_CHANGE;
   
         //
         // Set the level for all
         //
         switch ( _stage )
         {
            case brightning_e:
            {
               fb_led_t composite = { LED_ON, level };

               for ( i=0; i<TOPO_ACTIVE_STATIONS_COUNT; ++i)
               {
                  fb_set_composite(i, composite);
               }
         
               ++level;
         
               if ( level > LED_ON )
               {
                  period = TIME_WAIT_IN_BETWEEN;
                  _stage = dimming_e;
               }
            }
            break;
      
            case dimming_e:
            if ( level == LED_OFF )
            {
               for ( i=0; i<TOPO_ACTIVE_STATIONS_COUNT; ++i)
               {
                  fb_turn_off(i);
               }
         
               // That's it
               _stage = pausing_e;
               period = TIME_WAIT_IN_BETWEEN;
            }
            else
            {
               --level;
         
               for ( i=0; i<TOPO_ACTIVE_STATIONS_COUNT; ++i)
               {
                  fb_set(i, LED_ON, level);
               }
            }
            break;
            case pausing_e:
            // That's it.
            period = 0;
            break;
         }
   
         return period;
      }
   };
}

/** Reserve static space for the singleton instance */
ALLOCATE_INSTANCE(display::Fade);

/**@}*/
/**@}*/
/**@} ---------------------------  End of file  --------------------------- */