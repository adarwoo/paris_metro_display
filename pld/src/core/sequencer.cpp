/**
 * @addtogroup core
 * @{
 * @addtogroup sequencer
 * Manages the changes on the display
 * @{
 *****************************************************************************
 * This sequencer relies on the timer service and a simple state machine
 *  to alternate the various modes of display.
 *****************************************************************************
 * @file
 * Implementation of the sequencer
 * @author software@arreckx.com
 * @internal
 */
#include "driver/fb.h"
#include "lib/timer.h"
#include "sequencer.h"
#include "configuration.hpp"

using namespace mode;
using namespace config;

// ---------------------------------------------------------------------------
// Local defines
// ---------------------------------------------------------------------------
namespace
{
   /** Gap in ms to wait when switching from one sequence to the next */
   const timer_count_t GAP_BTW_SEQ(100);

   /** Logger domain for this file */
   const char DOM[] = "sq";
}


// ---------------------------------------------------------------------------
// Local variable
// ---------------------------------------------------------------------------
namespace
{
   /**
    * Keep the last timer instance in order to validate the callback
    * The sequencer can be stopped by invalidating this instance.
    */
   static timer_instance_t ongoing_timer_instance = TIMER_INVALID_INSTANCE;

   /**
    * @class ModeManager
    * Simple wrapper to manage the modes 
    * @var mode_manager
    * The local instance of mode manager used by the API and the timer
    *  callback #timer_callback function.
    */
   class ModeManager
   {
      /** Current mode index */
      uint8_t index;

      /** Is the demo mode on ? */
      bool mode_is_demo;

   public:
      /** Reset all that's all. No timer is armed yet. A call to start is required. */
      ModeManager() : index(0), mode_is_demo(false)
      {
         short_push_sequence[index]->reset();
      }

      /**
       * Update the mode
       * @return The result of an update on the mode
       */
      timer_count_t update()
      {
         IMode *pMode = mode_is_demo ? long_push_mode : short_push_sequence[index];
         return pMode->update();
      }

      /**
       * Advance to the next sequence.
       * If a demo was active, turn the demo mode off, and resume the previous
       *  mode.
       * Otherwise, advance (or rotate) to the next mode and reset it.
       */
      void next()
      {
         if (!mode_is_demo)
         {
            index = (index + 1) % number_of_sequence;
            short_push_sequence[index]->reset();
         }

         mode_is_demo = false;
         sequencer_start();
      }

      /** Switch to the long push mode */
      void demo()
      {
         mode_is_demo = true;
         long_push_mode->reset();
         sequencer_start();
      }
   } mode_manager;
}

// ---------------------------------------------------------------------------
// Private functions
// ---------------------------------------------------------------------------
namespace
{
   /** Called by the timer to handle the next move */
   extern "C" void timer_callback(timer_instance_t instance, void *arg)
   {
      static fb_mem_t working_fb;

      // Is this timer instance still valid?
      if (instance == ongoing_timer_instance)
      {
         // Switch to the working copy of frame buffer
         fb_use(&working_fb);

         // Reset the frame buffer
         fb_clear();

         //
         // Call the mode handler to update the frame buffer
         //
         timer_count_t nextCount = mode_manager.update();

         // Copy the modified frame buffer into the live frame buffer
         fb_commit();

         // Check the reply : >0 - Stay in the same mode // 0 - Switch
         if (nextCount > 0)
         {
            ongoing_timer_instance = timer_arm_from_now(timer_callback, nextCount, 0);
         }
         else
         {
            mode_manager.next();
         }
      }
   }
} // End of anonymous namespace
   
// ---------------------------------------------------------------------------
// API implementation
// ---------------------------------------------------------------------------
extern "C"
{
   /** Initialize and start the timer */
   void sequencer_start(void)
   { 
      ongoing_timer_instance = timer_arm_from_now(timer_callback, GAP_BTW_SEQ, 0);
   }

   /** A long key press takes back to the snake demo */
   void sequencer_switch_long(void)
      { mode_manager.demo(); }

   /** A short key press allow toggling between all the available modes */
   void sequencer_switch_short(void)
      { mode_manager.next(); }
}

/**@}*/
/** @} ------------------------  End of file  ---------------------------- */