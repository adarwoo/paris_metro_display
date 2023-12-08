/**
 * @addtogroup core
 * @{
 * @addtogroup mode
 * @{
 * @addtogroup pharma
 * @{
 *****************************************************************************
 * This mode alternate between time and temperature with gong every hour and
 *  on the half hour.
 *****************************************************************************
 * @file
 * Implementation of the metro mode
 * @author software@arreckx.com
 * @internal
 */

#include "lib/tz.h"
#include "core/measurements.h"
#include "core/display/display.hpp"
#include "mode.hpp"

using namespace display;
using lib::get_instance_ptr;

// The display to use
namespace display
{
   class Time;
   class Temperature;
   class WaitForValidClock;
   class Snake;
   class Fade;
}

// ---------------------------------------------------------------------------
// Local type
// ---------------------------------------------------------------------------
namespace 
{
   /** Time to remain in the time mode before switching to temperature */
   const uint8_t SECS_TO_STAY_IN_TIME_MODE = 20;

   /** Seconds to remain in the temp mode before switching back to time */
   const uint8_t SECS_TO_STAY_IN_TEMPERATURE_MODE = 6;

   /** 
    * Make sure to be in time mode all least n seconds before 
    * a time event such as a hour gong or half-hour gong.
    * That way, the user gets to see the time before the gong.
    */
   const uint8_t MIN_SECS_TO_GONG = 5;
   
   /** Gong sub-mode */
   enum class Mode : uint8_t
   {
      time = 0,
      temperature,
      gong_hour,
      gong_half_hour,
      wait_for_clock
   };
}
 
namespace mode
{
   /** The pharmacy mode shows the time alternating with the temperature */
   class Pharmacy : public lib::Singleton<IMode, Pharmacy>
   {
      /** Current mode */
      Mode mode;
      
      /** Stay in mode till the given number of seconds */
      uint8_t seconds_to_switch;
   public:
      Pharmacy() : mode(Mode::wait_for_clock), seconds_to_switch(-1) {}

   protected:
      /** After a reset (comming back to pharma, show the time */
      void reset() override
         { mode = Mode::wait_for_clock; }

      /**
       * Called to switch from time to gong or temperature
       */
      void compute_next_mode( tz_datetime_t *date )
      {
         uint8_t secsToMinute = 60-date->second;
         
         if ( secsToMinute == 1 && date->minute == 29 )
         {
            mode = Mode::gong_half_hour;
            get_instance_ptr<IDisplay <>, Snake>()->reset();
         }
         else if ( secsToMinute == 1 && date->minute == 59 )
         {
            mode = Mode::gong_hour;
            get_instance_ptr<IDisplay <>, Fade>()->reset();
         }
         else if ( --seconds_to_switch == 0 ) // Time left in this mode
         {
            // If in temperature mode, switch to time
            if ( mode == Mode::temperature )
            {
               // Reset the counter
               seconds_to_switch = SECS_TO_STAY_IN_TIME_MODE;
               mode = Mode::time;
            }
            // We should switch to temperature. However, if we're too close to gong
            //  we postpone the switch till after.
            else if
            ( 
               (date->minute != 29 && date->minute != 59) ||
               secsToMinute > (SECS_TO_STAY_IN_TEMPERATURE_MODE+MIN_SECS_TO_GONG)
            )
            {
               // We have enough time to show the temperature
               mode = Mode::temperature;
               seconds_to_switch = SECS_TO_STAY_IN_TEMPERATURE_MODE;
            }
         }
      }

      /** 
       * Grab the current mode and display
       */
      timer_count_t update() override
      {
         timer_count_t retval(TIMER_SECONDS(1));
         tz_datetime_t now;
   
         // Since we rely on the clock, make sure it's valid
         if ( tz_now( &now ) )
         {
            switch ( mode )
            {
            case Mode::wait_for_clock:
               // Clock back-on
               mode = Mode::time;
               seconds_to_switch = SECS_TO_STAY_IN_TIME_MODE;
               // proceed to showing the time
            case Mode::time:
               get_instance_ptr<IDisplay<calendar_date *>, Time>()->display(&now);
               compute_next_mode(&now);
               break;                  
            case Mode::temperature:
               get_instance_ptr<IDisplay<int16_t>, Temperature>()->display( measurement_get_temperature() );
               compute_next_mode(&now);
               break;
            case Mode::gong_hour:
               retval = get_instance_ptr<IDisplay <>, Snake>()->display(0);
               break;
            case Mode::gong_half_hour:
               retval = get_instance_ptr<IDisplay <>, Fade>()->display(0);
               break;
            }
            
            // One of the gong mode has finished. Go back to time.
            if ( retval == 0 )
            {
               mode = Mode::time;
               retval = TIMER_SECONDS(1);
               
               // Wait half the normal time since we know the minutes
               seconds_to_switch = SECS_TO_STAY_IN_TIME_MODE / 2;
            }
         }
         else
         {
            // Clock is not valid. Immediately switch to wait for clock.
            if ( mode != Mode::wait_for_clock )
            {
               mode = Mode::wait_for_clock;
               get_instance_ptr<IDisplay <>, WaitForValidClock>()->reset();
            }
            
            retval = get_instance_ptr<IDisplay <>, WaitForValidClock>()->display(0);
         }            
         
         return retval;
      }
   };
}

/** Reserve static space for the singleton instance */
ALLOCATE_INSTANCE(mode::Pharmacy);

/**@}*/
/**@}*/
/**@}*/
/* ---------------------------  End of file  --------------------------- */