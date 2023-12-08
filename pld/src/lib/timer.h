#ifndef timer_h_HAS_ALREADY_BEEN_INCLUDED
#define timer_h_HAS_ALREADY_BEEN_INCLUDED
/**
 * @addtogroup service
 * @{
 * @addtogroup timer
 * @{
 *****************************************************************************
 * Reactor timer service.
 * This API allow registering a callback to be called later from the reactor.
 * The timer can be (re)armed within interrupt context.
 * \n
 * You must set the maximum number of timers which can be on-going 
 *  simultaneously.
 * \n
 * This can be adjusted using the #TIMER_MAX_CALLBACK define.
 * By default, it is 8.
 * The timer service must initialised before it can be used with #timer_init.
 * \n
 * Example:
 * @code
 * #include "lib/timer.h"
 * #include "lib/reactor.h"
 * // Initialise the timer
 * timer_init();
 * // Arm an event to be called in 1 hour and 12 minutes
 * // Pass the value 12 when calling
 * timer_arm_from_now( callback, TIMER_HOURS(1)+TIMER_MINUTES(12), (void*)12);
 * @endcode
 * @file
 * [Timer](group__timer.html) service API declaration
 * @author gax
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/* Public types                                                         */
/************************************************************************/

/** Value to hit for all timer future objects */
typedef uint_fast32_t timer_count_t;

/**
 * Unique timer handle
 * Its size makes it unique. Even at 1ms interval (insane), it would take
 *  2^32 / 1000 seconds = 49 days to roll over.
 */
typedef uint32_t timer_instance_t;

/** Expire handler */
typedef void (*timer_callback_t)( timer_instance_t, void * );

/************************************************************************/
/* Public constants                                                     */
/************************************************************************/

/** Specify a non valid timer instance. To be used for markers */
#define TIMER_INVALID_INSTANCE ((timer_instance_t)-1)

/** Number of timer count in the given number of milliseconds */
#define TIMER_MILLISECONDS(x) ((timer_count_t)x)

/** Number of timer count in the given number of seconds */
#define TIMER_SECONDS(x) (((timer_count_t)1000)*((timer_count_t)x))

/** Number of timer count in the given number of minutes */
#define TIMER_MINUTES(x) TIMER_SECONDS(60*x)

/** Number of timer count in the given number of hours */
#define TIMER_HOURS(x) TIMER_MINUTES(60*x)


/************************************************************************/
/* Public API                                                           */
/************************************************************************/

/** Ready the service */
void timer_init( void );

/** Get the current counter */
timer_count_t timer_get_count( void );

/** Compute a count */
timer_count_t timer_get_count_from_now( timer_count_t count );

/** Get time elapsed from a previous time */
timer_count_t timer_time_lapsed_since( timer_count_t count );

/** Arm a timer */
timer_instance_t timer_arm(
   timer_callback_t cb,
   timer_count_t count,
   void *arg );

/** Short handle for arming in some time in the future */
static inline timer_instance_t timer_arm_from_now(
   timer_callback_t cb,
   timer_count_t count,
   void *arg )
{
   return timer_arm( cb, timer_get_count_from_now( count ), arg );
}

/** To be called from the reactor only. Look for expired jobs and process */
void timer_dispatch(void);

#ifdef __cplusplus
}
#endif

/**@} timer */
/**@} service */
#endif /* timer_h_HAS_ALREADY_BEEN_INCLUDED */
