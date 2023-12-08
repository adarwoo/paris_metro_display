#ifndef debug_h_HAS_ALREADY_BEEN_INCLUDED
#define debug_h_HAS_ALREADY_BEEN_INCLUDED
/**
 * @addtogroup service
 * @{
 * @addtogroup debug
 * @{
 *****************************************************************************
 * A set of macro to allow an API to use debug pins if allowed.
 * To enable a debug pin for a given API, simply define it in the
 * board configuration header as:
 * @code{.c}
 *    #define DEBUG_xxx IOPORT_CREATE_PIN(PORTC, 0)
 * @endcode
 * Where:
 * - xxx is the name of the API group such as REACTOR, FB etc..
 * The API only needs to make use of the debug API as if the pin
 *  had been defined.
 *     debug_set(REACTOR);
 * If the macro has not been defined, this will expand to nothing.
 * @file
 * Reactor API definition
 * @author software@arreckx.com
 */ 

#include "cpp.h"

/** Define macro to check if the argument x expands to DEBUG_x */
#define IS_DEBUG_PORT(x) IS_PAREN( CAT(DEBUG_, x) (()) )

/** Set the given pin, if it has been defined as DEBUG_x, an an output pin set to 0 */
#define debug_init(x) IIF(IS_DEBUG_PORT(x))(ioport_configure_pin(DEBUG_ ## x, IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW),{})
/** Set the corresponding debug pin  */
#define debug_set(x) IIF(IS_DEBUG_PORT(x))(ioport_set_pin_high(DEBUG_ ## x),{})
/** Clear the corresponding debug pin  */
#define debug_clear(x) IIF(IS_DEBUG_PORT(x))(ioport_set_pin_low(DEBUG_ ## x),{})

/**@}*/
/**@}*/
#endif /* ndef debug_h_HAS_ALREADY_BEEN_INCLUDED */