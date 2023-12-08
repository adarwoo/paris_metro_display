#ifndef reactor_HAS_ALREADY_BEEN_INCLUDED
#define reactor_HAS_ALREADY_BEEN_INCLUDED
/**
 * @file
 * Reactor API declaration
 * @addtogroup service
 * @{
 * @addtogroup reactor
 * @{
 *****************************************************************************
 * Reactor API.
 * The reactor pattern allow to manage many asynchronous events
 *  within the same thread of processing.
 * This allow to process in the main function time and stack, events generated
 *  within interrupt code.
 * The events are prioritized such that the first handler that registers is
 *  always dealt with first.
 * This API is very simple, but deals with the complexity of atomically suspending
 *  the MPU in the main loop whilst processing interrupt outside of the interrupt
 *  context.
 * The interrupt do not need to hug the CPU time for long and simply notify the
 *  reactor that work is needed.
 * Therefore, all the work is done in the same context and stack avoiding
 *  nasty race conditions.
 * To use this API, simply register a handler with #reactor_register.
 * Then, if the main program needs to process the event, register a event handler
 *  using #reactor_register.
 * Finally, let the reactor loose once all the interrupts are up and running with
 *  #reactor_run.
 * @author software@arreckx.com
 */
 
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def REACTOR_MAX_HANDLERS
 * Maximum number of handlers for the reactor. This defines defaults
 *  to 8 and can be overridden in the board_config.h file if more are
 *  required.
 */
#ifndef REACTOR_MAX_HANDLERS
   #define REACTOR_MAX_HANDLERS 8
#endif

/**
 * @typedef reactor_handle_t 
 * A handle created by #reactor_register to use with #reactor_notify to
 *  tel the reactor to process the callback.
 * The size of this handle depend on the maximum number of reactor's handlers
 *  required.
 */
#if REACTOR_MAX_HANDLERS > 32
   #error "Maximum number of reactor handlers cannot exceed 32"
#elif REACTOR_MAX_HANDLERS > 16
   typedef uint32_t reactor_handle_t;
#elif REACTOR_MAX_HANDLERS > 8
   typedef uint16_t reactor_handle_t;
#else
   typedef uint8_t reactor_handle_t;
#endif

/** @cond internal */
/** Holds all on-going notification flags. This must not be used directly */
extern volatile reactor_handle_t reactor_notifications;
/** @endcond */

/** Callback type called by the reactor when an event has been logged */
typedef void (*reactor_handler_t)(void);

/** Initialize the reactor API */
void reactor_init(void);

/** Add a new reactor process */
reactor_handle_t reactor_register( const reactor_handler_t );

/** 
 * Notify a handler should be invoke next time the loop is processed
 * Interrupt safe. No lock here since this is processed in normal
 * (not interrupt) context.
 */
void reactor_notify( const reactor_handle_t handle );

/** Process the reactor loop */
void reactor_run(void);

#ifdef __cplusplus
}
#endif

/** @} */
/** @} */
#endif /* ndef reactor_HAS_ALREADY_BEEN_INCLUDED */