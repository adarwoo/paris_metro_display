#ifndef core_display_display_h_HAS_ALREADY_BEEN_INCLUDED
#define core_display_display_h_HAS_ALREADY_BEEN_INCLUDED
/**
 * @addtogroup core
 * @{
 * @addtogroup display
 * @{
 *****************************************************************************
 * You need to edit this file if a new show mode is added.
 * @n
 * The prototype should typically be:
 * @code
 * timer_count_t show_xxx(bool init);
 * @endcode
 *****************************************************************************
 * @file
 * Prototypes for all [shows](group__show.html) for the application
 * @author gax
 */

#include "lib/timer.h"

namespace display
{
   /** 
    * Interface for a display class
    * @tparam P Type of data to be displayed. Defaults to void*.
    */
   template <typename P=void*> struct IDisplay
   {
      /** 
       * Called to reset the display internal state machine
       */
      virtual void reset() {};
         
      /**
       * Called to put the data on the display, with optionally the data
       *  as a parameter.
       */
      virtual timer_count_t display(P) = 0;
   };
}

/**@}*/
/**@}*/
#endif /* ndef core_display_display_h_HAS_ALREADY_BEEN_INCLUDED */