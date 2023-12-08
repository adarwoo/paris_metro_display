#ifndef core_mode_mode_h_HAS_ALREADY_BEEN_INCLUDED
#define core_mode_mode_h_HAS_ALREADY_BEEN_INCLUDED
/**
 * @addtogroup core
 * @{
 * @addtogroup mode
 * @{
 *****************************************************************************
 * Base class for the modes.
 * @n
 * Example:
 * @code
 * #include "core/mode/mode.h"
 * // Create a new mode
 * struct ModeSnake
 * {
 *
 * // Arm an event to be called in 1 hour and 12 minutes
 * // Pass the value 12 when calling
 * timer_arm_from_now( callback, TIMER_HOURS(1)+TIMER_MINUTES(12), (void*)12);
 * @endcode
 *****************************************************************************
 * @file
 * [Mode](group__mode.html) base class and API
 * @author gax
 */

#include "lib/timer.h"
#include "lib/singleton.hpp"

namespace mode
{
   /**
    * Base virtual mode class
    * All modes should inherit from this class and override at least
    *  the #update method.
    */
   struct IMode
   {
      /** 
       * This is called when the mode is restarted.
       * Override for stateful modes to reset internal states.
       */
      virtual void reset() {}
   
      /** 
       * Called by the sequencer when after some time to update the LED panel.
       * This method must be overridden. The frame buffer is readily switched
       *  to a blank working frame buffer and must be refilled entirely.
       * If the mode is reset, the reset method is called prior to calling this
       *  function.
       *
       * @return Time gap until the next update
       */
      virtual timer_count_t update() = 0;
   };

   /**
    * Shortcut to obtaining a mode singleton instance pointer
    * @tparam M Class whose singleton pointer to get
    * @return A pointer to the singleton instance
    */
   template <class M> constexpr IMode *get_mode_ptr()
   {
      return lib::get_instance_ptr<IMode, M>();
   }

} // End of namespace 'mode'

/**@}*/
/**@}*/
#endif /* ndef core_mode_mode_h_HAS_ALREADY_BEEN_INCLUDED */