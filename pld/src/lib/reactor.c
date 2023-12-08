/**
 * @addtogroup service
 * @{
 * @addtogroup reactor
 * @{
 *****************************************************************************
 * Implementation of the reactor pattern.
 * This reactor allow dealing with asynchronous events handled by interrupts
 *  within the time frame of the main application.
 * When no asynchronous operation take place, the micro-controller is put to
 *  sleep saving power.
 * The reactor cycle time can be monitored defining debug pins REACTOR_IDLE
 *  and REACTOR_BUSY
 *****************************************************************************
 * @file
 * Implementation of the reactor API
 * @author software@arreckx.com
 * @internal
 */
#include <asf.h>
#include <stdint.h>

#include "debug.h"
#include "reactor.h"

/** @cond internal */
// Force in the data segmemnt
volatile reactor_handle_t reactor_notifications = 0;
/** @endcond */

/** Current number of handlers */
static uint8_t _next_handle = 0;

/** Keep an array of handlers whose position match the bit position of the handle */
static reactor_handler_t _handlers[REACTOR_MAX_HANDLERS] = {0};

/** Initialize the reactor API */
void reactor_init(void)
{
   // Use a debug pin if available
   debug_init(REACTOR_IDLE);
   debug_init(REACTOR_BUSY);

   // Allow simplest sleep mode to resume very fast
   sleep_set_mode(SLEEP_SMODE_IDLE_gc);
   sleep_enable();
}

/** Add a new reactor process */
reactor_handle_t reactor_register( const reactor_handler_t handler )
{
   reactor_handle_t retval;
   
   _handlers[_next_handle] = handler;
   retval = 1 << _next_handle++;
   
   return retval;
}

/** 
 * Interrupts are disable to avoid the or'ing to endup badly
 */
void reactor_notify( const reactor_handle_t handle )
{
   cli();
   reactor_notifications |= handle; 
   sei();
}

/** Process the reactor loop */
void reactor_run(void)
{
   uint8_t i;
   reactor_handle_t flags;
   
   // Atomically read and clear the notification flags allowing more
   //  interrupt from setting the flags which will be processed next time round
   while (true)
   {
      debug_clear(REACTOR_BUSY);
      cli();
   
      if ( reactor_notifications == 0 )
      {
         debug_set(REACTOR_IDLE);

         // The AVR guarantees that sleep is executed before any pending interrupts
         sei();
         sleep_cpu();
         debug_clear(REACTOR_IDLE);
      }
      else
      {
         flags = reactor_notifications;
         reactor_notifications = 0;
         debug_set(REACTOR_BUSY);
         sei();
         
         // Handle the flags
         for ( i=0; i<_next_handle; ++i )
         {
            if ( flags & 1 )
            {
               // Keep the system alive for as long as the reactor is calling handlers
               // We assume that if no handlers are called, the system is dead.
               wdt_reset();
               _handlers[i]();
            }
      
            // Move onto next notification
            flags>>=1;
         }
      }
   };
}

 /**@}*/
 /**@}*/
 /**@} ---------------------------  End of file  --------------------------- */