/**
 * @addtogroup core
 * @{
 * @addtogroup mode
 * @{
 * @weakgroup wait_for_valid_clock
 * @{
 *****************************************************************************
 * This mode display a chaser whilst waiting for a valid clock
 *****************************************************************************
 * @file
 * Implementation of the WaitForValidClock class
 * @author software@arreckx.com
 * @internal
 */

#include "driver/fb.h"
#include "lib/singleton.hpp"
#include "core/topo.h"
#include "display.hpp"

// ---------------------------------------------------------------------------
// Local constants
// ---------------------------------------------------------------------------
namespace
{
   /** Led used to indicate time mode */
   const fb_index_t _TIME_MODE_LED = ACHERES_GRAND_CORMIER;
   
   /** Led used to indicate slice of the day */
   const fb_index_t _TIME_MORNING_STATION = MAISONS_LAFFITTE;

   /** Next in chaser */   
   const fb_index_t _TIME_AFTERNOON_STATION = SARTROUVILLE;
   
   /** Final in chaser */
   const fb_index_t _TIME_EVENING_STATION = HOUILLES_CARRIERE_SUR_SEINE;
   
   /** Interval in ms for the chaser when waiting for a valid clock */
   const fb_index_t _TIME_WAITING_CHASER_INTERVAL = TIMER_MILLISECONDS(250);
}


// ---------------------------------------------------------------------------
// Class definition
// ---------------------------------------------------------------------------
namespace display
{
   class WaitForValidClock : public lib::Singleton<IDisplay<>, WaitForValidClock>
   {
      /** Current station led for displaying no reliable clock is found */
      fb_index_t waiting_for_clock_current_led = 0;
   
   public:
      void reset() override
      {
         waiting_for_clock_current_led = _TIME_MORNING_STATION;
      }
      
      timer_count_t display( void * arg ) override
      {
         // Turn on led to indicate time mode
         fb_set(_TIME_MODE_LED, LED_FLASH_FAST, LED_LEVEL_LOW);
         
         // Chaser for waiting
         fb_set( waiting_for_clock_current_led, LED_ON, LED_LEVEL_FULL);
            
         // Move chaser
         switch ( waiting_for_clock_current_led )
         {
            case _TIME_MORNING_STATION:
               waiting_for_clock_current_led = _TIME_AFTERNOON_STATION;
               break;
            case _TIME_AFTERNOON_STATION:
               waiting_for_clock_current_led = _TIME_EVENING_STATION;
               break;
            case _TIME_EVENING_STATION:
            default:
               waiting_for_clock_current_led = _TIME_MORNING_STATION;
               break;
         }
         
         return _TIME_WAITING_CHASER_INTERVAL;
      }
   };
}

/** Reserve static space for the singleton instance */
ALLOCATE_INSTANCE(display::WaitForValidClock);

/**@}*/
/**@}*/
/**@} ---------------------------  End of file  --------------------------- */