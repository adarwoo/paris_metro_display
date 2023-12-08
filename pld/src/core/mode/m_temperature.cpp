/**
 * @addtogroup core
 * @{
 * @addtogroup mode
 * @{
 * @addtogroup temperature
 * @{
 *****************************************************************************
 * This mode continuously displays the ambient temperature. It never ends.
 * It is stateless and does nothing on reset.
 *****************************************************************************
 * @file
 * Implementation of the temperature mode
 * @author software@arreckx.com
 * @internal
 */
#include "mode.hpp"
#include "core/display/display.hpp"
#include "core/measurements.h"

// The display to use
namespace display { class Temperature; }

namespace 
{
   const timer_count_t repeat_period = TIMER_SECONDS(2);
}

// ---------------------------------------------------------------------------
// Class definition
// ---------------------------------------------------------------------------
namespace mode
{
   class Temperature : public lib::Singleton<IMode, Temperature>
   {
      /** Called to refresh the temperature */
      timer_count_t update() override
      {
         // return display::Temperature::instance()
         lib::get_instance_ptr<display::IDisplay<int16_t>, display::Temperature>()->display(
            measurement_get_temperature()
         );
         
         return repeat_period;
      }
   };
}

/** Reserve static space for the singleton instance */
ALLOCATE_INSTANCE(mode::Temperature);

/**@}*/
/**@}*/
/**@}*/
/* ---------------------------  End of file  --------------------------- */