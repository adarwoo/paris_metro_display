/**
 * @addtogroup core
 * @{
 * @addtogroup mode
 * @{
 * @addtogroup demo
 * @{
 *****************************************************************************
 * Display some demo pattern on the board
 *****************************************************************************
 * @file
 * Implementation of the demo mode
 * @author software@arreckx.com
 * @internal
 */

#include "lib/cpp.h"
#include "core/display/display.hpp"
#include "mode.hpp"

using namespace display;

// The display to use
namespace display 
{ 
   class Snake;
   class Fade;
}

// ---------------------------------------------------------------------------
// Locals
// ---------------------------------------------------------------------------
namespace
{
   /** Update rate for the temperature display */
   const timer_count_t update_period = TIMER_SECONDS(1);

   /** Handlers to be used */
   IDisplay<> *handlers[]
   {
      lib::get_instance_ptr<IDisplay<>, Snake>(),
      lib::get_instance_ptr<IDisplay<>, Fade>(),
   };
}

namespace mode
{
   class Demo : public lib::Singleton<IMode, Demo>
   {
      /** Current mode */
      uint8_t mode;

   public:
      Demo() : mode(0)
      {}

      void reset() override
      {
         mode = 0;
         handlers[mode]->reset();
      }
      
      /** Wait for the luminosity level to be high enough to show something */
      timer_count_t update() override
      {
         timer_count_t next = 0;
         
         if ( mode < countof(handlers) )
         {
            next = handlers[mode]->display(0);
            
            if ( next == 0 )
            {
               ++mode;
               
               if ( mode < countof(handlers) )
               {
                  handlers[mode]->reset();
                  next = TIMER_SECONDS(1);
               }
            }
         }
         
         return next;
      }
   };
}

/** Reserve static space for the singleton instance */
ALLOCATE_INSTANCE(mode::Demo);

/**@}*/
/**@}*/
/**@}*/
/* ---------------------------  End of file  --------------------------- */
