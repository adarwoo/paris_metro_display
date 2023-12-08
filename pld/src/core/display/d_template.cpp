// Check list
// 1 - Replace @name@ with your own name
// 2 - Write the reset method
// 3 - Write the display method
// Delete the check list afterwards
/**
 * @addtogroup core
 * @{
 * @addtogroup mode
 * @{
 * @addtogroup @name@
 * @{
 *****************************************************************************
 * <Add a description of the display HERE>
 *****************************************************************************
 * @file
 * Implementation of the @name@ mode
 * @author <Your email address or name or delete this line if you feel the shame>
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
   // >>>> Add all local constant HERE
   // <<<< Note : Time type is timer_count_t. To initialise time use one of the TIMER_XXX macros
   // <<<< LED index are fb_index_t whereas the topo index is tiny_index_t

   /** Period of update of the dimmer */
   const timer_count_t TIME_FADE_NEXT_CHANGE = TIMER_MILLISECONDS(100);

   /** Route used to show tenth */
   const route_id_t TEMP_TENTH_ROUTE(A1_A2);

   /** Led used to indicate temperature mode */
   const fb_index_t MODE_LED(POISSY);
}

// ---------------------------------------------------------------------------
// Class definition
// ---------------------------------------------------------------------------
namespace display
{
   class @name@ : public lib::Singleton<IDisplay<>, @name@>
   {
      // Data members section
   public:
      void reset() override
      {
         // Add reset code here
      }

      timer_count_t display(void *arg) override
      {
         timer_count_t period = TIMER_SECONDS(2);

         //
         // Add display code here
         //
   
         return period;
      }
   };
}

/** Reserve static space for the singleton instance */
ALLOCATE_INSTANCE(display::@name@);

/**@}*/
/**@}*/
/**@} ---------------------------  End of file  --------------------------- */