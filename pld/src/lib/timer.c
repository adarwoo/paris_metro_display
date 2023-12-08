/**
 * @file
 * [Timer](group__timer.html) service implementation
 * @internal
 * @addtogroup service
 * @{
 * @addtogroup timer
 * @{
 * @author gax
 */

#include <string.h>
#include <stdint.h>

#include "timer.h"
#include "alert.h"
#include "reactor.h"

 /************************************************************************/
 /* Local macros                                                         */
 /************************************************************************/

 /**
  * @typedef _timer_int_level_t
  * Level of interrupt prior to stopping it
  * @fn _timer_restore_interrupt_level
  * Restore the timer interrupt back on applying the same level as before
  * @fn _timer_get_interrupt_level_and_disable
  * Get the current timer interrupt level, and disable the timer interrupts
  * @def TIMER_ATOMIC_BLOCK
  * Stop the timer within the following block
  */
#ifndef _WIN32
#include <asf.h>
#include "tc.h"

typedef enum TC_INT_LEVEL_t _timer_int_level_t;

static inline void _timer_restore_interrupt_level(_timer_int_level_t* level)
{
	tc_set_overflow_interrupt_level(&TIMER_TC, *level);
}

static inline _timer_int_level_t _timer_get_interrupt_level_and_disable(void)
{
	_timer_int_level_t level =
		(((TC0_t*)&TIMER_TC)->INTCTRLA & TC0_OVFINTLVL_gm) >> TC0_OVFINTLVL_gp;
	tc_set_overflow_interrupt_level(&TIMER_TC, TC_INT_LVL_OFF);

	return level;
}

#define TIMER_ATOMIC_BLOCK() \
      for ( enum TC_INT_LEVEL_t level __attribute__((__cleanup__(_timer_restore_interrupt_level))) = \
         _timer_get_interrupt_level_and_disable(), __ToDo = 1; __ToDo ; __ToDo = 0 )
#else
#define TIMER_ATOMIC_BLOCK()
#endif

/************************************************************************/
/* Local types                                                          */
/************************************************************************/

/** Keep the arg and the callback together */
typedef struct _timer_future_t
{
	timer_callback_t cb;
	timer_count_t count;
	timer_instance_t instance;
	void* arg;
} _timer_future_t;

/** One of the timer slot or -1 if not valid */
typedef volatile int_fast8_t _timer_slot_t;

/************************************************************************/
/* Local variables                                                      */
/************************************************************************/


/**
 * @def TIMER_MAX_CALLBACK
 * Default number of callbacks
 */
#ifndef TIMER_MAX_CALLBACK
#  define TIMER_MAX_CALLBACK 32
#endif

 /** Invalid slot marker */
#define TIMER_INVALID_SLOT -1

/**
 * Keep track of running timers
 * This array is sliding and sorted such as the first items are
 *  those that expire first.
 * This way, the interrupt handler
 */
static _timer_future_t _timer_future_sorted_list[TIMER_MAX_CALLBACK] = {0};

/** Free running counter. */
static volatile timer_count_t _timer_free_running_ms_counter = 0;

/** 0 based index to the next slot to use by the reactor loop */
static _timer_slot_t _timer_slot_reactor = TIMER_INVALID_SLOT;

/** 0 based index to the next slot to use upon IT */
static _timer_slot_t _timer_slot_active = 0;

/** 0 based index to the next available slot */
static _timer_slot_t _timer_slot_avail = 0;

/** Last instance given */
static timer_instance_t _timer_current_instance = TIMER_INVALID_INSTANCE;

/** The API Handle for the reactor */
static reactor_handle_t _timer_reactor_handle = 0;


/************************************************************************/
/* Private helpers                                                      */
/************************************************************************/

/** @return The index to the right */
static inline _timer_slot_t _timer_right_of(_timer_slot_t index)
{
	return index == (TIMER_MAX_CALLBACK - 1) ? 0 : index + 1;
}

/** @return The index to the left */
static inline _timer_slot_t _timer_left_of(_timer_slot_t index)
{
	return index == 0 ? (TIMER_MAX_CALLBACK - 1) : (index - 1);
}

/** @return The distance in tick from the current position */
static inline int32_t _timer_distance_of(timer_count_t from, timer_count_t to)
{
	int32_t retval = to - from;

	return retval;
}

/************************************************************************/
/* Local API                                                            */
/************************************************************************/
void timer_overflow_it(void); // Not static as required by the simulation

/************************************************************************/
/* Public API                                                           */
/************************************************************************/

/**
 * Get the timer count.
 * @return The current free running counter
 */
timer_count_t timer_get_count(void)
{
	timer_count_t retval;

	TIMER_ATOMIC_BLOCK() // Atomic in Win32
	{
		retval = _timer_free_running_ms_counter;
	}

	return retval;
}

/**
 * Ready the timer
 * Configure the timer and enable the interrupt.
 * Assumes irq are started
 */
void timer_init(void)
{
	// Reset the timer list
	//memset(_timer_future_sorted_list, 0, sizeof(_timer_future_sorted_list));

	// Interrupt code for the AVR target only
	// This is initialised automatically for the simulator
#ifndef _WIN32
	tc_enable(&TIMER_TC);

	// Simple timer compare mode required
	tc_set_wgm(&TIMER_TC, TC_WG_NORMAL);

	// The timer service interrupts run at low level to allow
	//  other interrupts to take precedence
	tc_set_overflow_interrupt_level(&TIMER_TC, TC_INT_LVL_HI);

	// Set the callback
	tc_set_overflow_interrupt_callback(&TIMER_TC, &timer_overflow_it);

	// Set the top to 125
	tc_write_period(&TIMER_TC, 125);

	// Source is main clock (assuming 32MHz) divided by 256
	// Gives a overall clock as 125kHz
	// This effectively starts the timer ticking
	tc_write_clock_source(&TIMER_TC, TC_CLKSEL_DIV256_gc);
#endif   

	// Register with the reactor
	_timer_reactor_handle = reactor_register(&timer_dispatch);
}

/**
 * Compute the expiry timer count.
 * The value returned can be used with the timer_arm function.
 *
 * @param delayMs Delay to add to the count now
 * @return The count to hit in the future.
 */
timer_count_t timer_get_count_from_now(timer_count_t delayMs)
{
	// The returned value may roll over
	return (timer_get_count() + delayMs);
}

/**
 * Compute the time elapsed since a previous time
 *
 * @param count The previous timer count
 * @return The time elapsed since and from now
 */
timer_count_t timer_time_lapsed_since(timer_count_t count)
{
	return timer_get_count() - count;
}

/**
 * Arm a timer.
 * This function checks for several conditions:
 *  * Now more slots!
 *  * Time given too short
 * The list is sorted to help the interrupt be short.
 * This function can be safely called from within an interrupt context.
 *
 * @param cb Function to call on expiry. This function is called from within interrupt
 *  context.
 * @param count Deadline value as a timer_count. This value is best computed by
 *               calling timer_get_count_from_now
 * @param arg Extra argument passed to the caller
 *
 * @return The handle (slot position of the timer)
 */
timer_instance_t timer_arm(
	timer_callback_t cb,
	timer_count_t count,
	void* arg)
{
	_timer_slot_t insertPoint;
	_timer_slot_t i;
	timer_instance_t retval;

	timer_count_t now = timer_get_count();

	// Start from the active position
	insertPoint = _timer_slot_active;

	// Special case where the slots have met up with the active being used
	// Alert the user and drop the oldest slot
	if (insertPoint == _timer_slot_avail && _timer_future_sorted_list[insertPoint].cb != NULL)
	{
		alert();
	}

	// Look for the effective insertion position (sorted)
	while (insertPoint != _timer_slot_avail)
	{
		// The count is relative to the current position
		// Depending of where we are, we need to decide if the new count is a roll
		//  over
		if (_timer_distance_of(now, count)
			< _timer_distance_of(now, _timer_future_sorted_list[insertPoint].count))
		{
			break;
		}

		// Move onto next slot. Use modulus to loop round
		insertPoint = _timer_right_of(insertPoint);
	}

	// Shift all items to the right
	for (i = _timer_slot_avail; i != insertPoint; )
	{
		_timer_slot_t oneLeftOf = _timer_left_of(i);

		memcpy(
			&_timer_future_sorted_list[i],         // To
			&_timer_future_sorted_list[oneLeftOf], // From
			sizeof(_timer_future_t)
		);

		i = oneLeftOf;
	}

	// Insert the new item
	_timer_future_t next = {
		.cb = cb,
		.count = count,
		.instance = ++_timer_current_instance,
		.arg = arg,
	};

	_timer_future_sorted_list[insertPoint] = next;

	// Move next available slot
	_timer_slot_avail = _timer_right_of(_timer_slot_avail);

	// Set the retval
	retval = _timer_current_instance;

	return retval;
}

/**
 * Called by the timer ISR every 1ms
 * Only increment the timer if the dispatch has been called.
 * This guarantees, all handlers are called in time.
 */
void timer_overflow_it(void)
{
	++_timer_free_running_ms_counter;
	// Tell the reactor to process the tick
	reactor_notify(_timer_reactor_handle);
}

/**
 * Allow processing timer events in a reactor pattern.
 * This is called every ms and should be swift, but no race condition should
 *  occur since the timer structure is not managed by the interrupt
 * Only 1 callback is procceced at a time - that is 1 callback/ms max
 */
void timer_dispatch(void)
{
	// Store on the stack all the callbacks to invoke to 
	//  disable interrupt for the shortest while and free the slots quickly
	struct _tCb
	{
		timer_callback_t cb;
		timer_instance_t i;
		void* arg;
	} cb = { 0 };

	// Grab an atomic copy of the time now
	timer_count_t timeNow = timer_get_count();

	// At least one pending timer
	if (_timer_slot_active != _timer_slot_avail)
	{
		_timer_future_t* pFuture = &_timer_future_sorted_list[_timer_slot_active];

		if (timeNow >= pFuture->count)
		{
			// Handle the callback
			cb.cb = pFuture->cb;
			cb.i = pFuture->instance;
			cb.arg = pFuture->arg;

			// Reset the call to see it's avail
			pFuture->cb = NULL;

			// Move the pointer to the next item
			_timer_slot_active = _timer_right_of(_timer_slot_active);
		}
	}

	if (cb.cb)
	{
		// Call the callback of the timer
		cb.cb(cb.i, cb.arg);
	}
}

/**@}*/
/**@} ---------------------------  End of file  --------------------------- */