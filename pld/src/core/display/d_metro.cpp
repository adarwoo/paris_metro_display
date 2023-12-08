/**
 * @addtogroup core
 * @{
 * @addtogroup mode
 * @{
 * @addtogroup metro
 * @{
 *****************************************************************************
 * The metro mode simulated the normal operation of the PLD
 *****************************************************************************
 * @file
 * Implementation of the metro mode
 * @author software@arreckx.com
 * @internal
 */
#include <stdlib.h>

#include "driver/fb.h"
#include "lib/alert.h"
#include "core/topo.h"
#include "lib/singleton.hpp"

#include "display.hpp"

// ---------------------------------------------------------------------------
// Local types
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Local variable
// ---------------------------------------------------------------------------
namespace
{
   /** Journey stations */
   const route_id_t journey[] = 
      { A3_A4, A4_A3, A3_A2, A2_A1, A1_A4, A4_A5, A5_A2, A2_A5, A5_A4, A4_A1, A1_A2, A2_A3, INVALID_ROUTE };
}





// ---------------------------------------------------------------------------
// Class definition
// ---------------------------------------------------------------------------
namespace display
{
   class Metro : public lib::Singleton<IDisplay<>, Metro>
   {
      //
      // Private types
      //
      
      /** Create a random delay range */
      enum DelayRangeType
      {
         inter_station=0,
         wait_in_station=1
      };
     
	   enum class State : uint8_t
      {
   	   start_route,
   	   handle_train_move,
   	   handle_train_stop,
	   };
      
      //
      // Data members
      //
      
      /** Index of the currently operating route */
      tiny_index_t route_index = 0;

      /** Index of the station within a route */
      fb_index_t station_index = 0;
      
      /** Current state of the train within this mode */#
      State state;
      
      //
      // Private helpers
      //
   protected:
      /**
       * Generate a random delay
       *
       * @param range Range to consider
       */
      static timer_count_t random_delay( const DelayRangeType range )
      {
         /** Combine with a range */
         struct DelayRange
         {
            timer_count_t min_delay;
            timer_count_t max_delay;
         };
         
         static const DelayRange delay_range[]
         {  //     Minimum           Maximum
            { TIMER_SECONDS(20), TIMER_SECONDS(40) },  // Inter-station
            { TIMER_SECONDS(5), TIMER_SECONDS(10) }    // Wait in station
         };
         
         timer_count_t duration = delay_range[range].max_delay - delay_range[range].min_delay;

         // Grab a random value
		 // Forced to used non-reentrant version since MSVC won't supply the _r version
		 // This is however OK since not used in interrupts
         int r = rand();
   
         // Use modulus to scale down
         r %= (unsigned long)duration;
   
         return  ((timer_count_t)r) + delay_range[range].min_delay;
      }

      /** 
       * Light up the remaining part of the route
       * @return The current route id
       */
      route_id_t show_remaining_station()
      {
         tiny_index_t i = 0;
         route_id_t route = journey[route_index];
         fb_index_t led;

         // Keep on the route (don't overrun)
         while ((led = topo_get_led(++i, route)) != TOPO_OUT_OF_RANGE)
         {
            if (i > station_index)
            {
               fb_set(led, LED_ON, LED_LEVEL_MED);
            }
         }

         return route;
      }
      
      /** 
       * Train powers up 
       * Turn on all LEDs of the current route
       */
      void start_route()
      {
         // Start from 0
         station_index = 0;
         route_id_t route = show_remaining_station();
	
	      // Flash current station
	      fb_set( topo_get_led(0, route), LED_FLASH_MEDIUM, LED_LEVEL_FULL);
      }

      /** 
       * Handle a train moving away from the station
       * @return true if the terminus is reached
       */
      bool handle_train_move()
      {
         bool retval = false;
         route_id_t route = show_remaining_station();
	
	      // We've reached the terminus
	      if ( topo_get_led(++station_index, route) == TOPO_OUT_OF_RANGE )
	      {
            station_index = 0;
            route_id_t route = journey[++route_index];
      
            if ( route == INVALID_ROUTE )
            {
			      route_index = 0;
		      }
      
            retval = true;
	      }
   
         return retval;
      }

      void handle_train_stop()
      {
         route_id_t route = show_remaining_station();
      
	      // Flash current station
	      fb_set( topo_get_led(station_index, route), LED_FLASH_MEDIUM, LED_LEVEL_FULL);
      }

   //
   // Interface implementation
   //
   public:
      /** Constructor */
      Metro() : state(State::start_route)
      {}

      timer_count_t display(void *arg) override
      {
         timer_count_t randon_delay = 0;
   
	      switch ( state )
	      {
	      case State::start_route:
		      start_route();
            randon_delay = random_delay(DelayRangeType::wait_in_station);
            state = State::handle_train_move;
		      break;
	      case State::handle_train_move:
            if ( handle_train_move() )
            {
               randon_delay = random_delay(DelayRangeType::wait_in_station) * 4;
               // Start a route and return 0
               state = State::start_route;
            }
            else
            {
               randon_delay = random_delay(DelayRangeType::inter_station);
               state = State::handle_train_stop;
            }
            break;
	      case State::handle_train_stop:
   	      handle_train_stop();
            randon_delay = random_delay(DelayRangeType::wait_in_station);
            state = State::handle_train_move;
	         break;
         default:
            // Unexpected!
            alert_and_stop();
	      }
      
         return randon_delay;
      }
   };
} // End of namespace 'display'

/** Reserve static space for the singleton instance */
ALLOCATE_INSTANCE(display::Metro);

/**@} metro */
/**@} mode */
/**@} ---------------------------  End of file  --------------------------- */