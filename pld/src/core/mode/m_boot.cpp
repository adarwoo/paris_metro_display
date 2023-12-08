/**
 * @addtogroup core
 * @{
 * @addtogroup mode
 * @{
 * @addtogroup boot
 * @{
 *****************************************************************************
 * This initial mode waits for the luminosity to be suitable to start
 *  displaying data.
 *****************************************************************************
 * @file
 * Implementation of the metro mode
 * @author software@arreckx.com
 * @internal
 */

#include "lib/singleton.hpp"
#include "core/measurements.h"
#include "mode.hpp"

namespace mode
{
   class Boot : public lib::Singleton<IMode, Boot>
   {
      /** Wait for the luminosity level to be high enough to show something */
      timer_count_t update() override
      {
         timer_count_t retval = TIMER_SECONDS(1);
      
         if ( ! measurement_luminosity_is_dark() )
         {
            retval = 0;
         }
      
         return retval;
      }
   };
}

/** Reserve static space for the singleton instance */
ALLOCATE_INSTANCE(mode::Boot);

/**@}*/
/**@}*/
/**@}*/
/* ---------------------------  End of file  --------------------------- */