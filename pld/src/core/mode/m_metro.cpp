/**
 * @addtogroup core
 * @{
 * @addtogroup mode
 * @{
 * @addtogroup metro
 * @{
 *****************************************************************************
 * This mode simulates the real metro display. It never resets the display.
 *****************************************************************************
 * @file
 * Implementation of the metro mode
 * @author software@arreckx.com
 * @internal
 */
#include "core/display/display.hpp"
#include "mode.hpp"

// The display to use
namespace display { class Metro; }

// ---------------------------------------------------------------------------
// Class definition
// ---------------------------------------------------------------------------
namespace mode
{
   class Metro : public lib::Singleton<IMode, Metro>
   {
      /** Called to refresh the temperature */
      timer_count_t update() override
      {
         return lib::get_instance_ptr<display::IDisplay<>, display::Metro>()->display(0); 
      }
   };
}

/** Reserve static space for the singleton instance */
ALLOCATE_INSTANCE(mode::Metro);

/**@}*/
/**@}*/
/**@}*/
/* ---------------------------  End of file  --------------------------- */