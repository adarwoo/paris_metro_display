<COPY THIS FILE AND RENAME AS PER THE CORRECT MODE>
<REPLACE ALL XXX WITH THE CORRECT MODE NAME>
/**
 * @internal
 * @addtogroup core
 * @{
 * @addtogroup mode
 * @{
 * @addtogroup XXX
 * @{
 *****************************************************************************
 * This mode ..... <describe what the mode is about>
 *****************************************************************************
 * @file
 * Implementation of the XXX mode
 * @author software@arreckx.com
 */ 
#include "core/display/display.hpp"
#include "mode.hpp"

// The display to use
namespace display { class XXX; }

// ---------------------------------------------------------------------------
// Class definition
// ---------------------------------------------------------------------------
namespace mode
{
   class XXX : public lib::Singleton<IMode, XXX>
   {
      /** Called to refresh the display */
      timer_count_t update() override
      {
		 // Delegate the display
         return display::instance_display<display::XXX>();
      }
   };
}

/** Reserve static space for the singleton instance */
ALLOCATE_INSTANCE(mode::XXX);

/**@}*/
/**@}*/
/**@}*/
/* ---------------------------  End of file  --------------------------- */