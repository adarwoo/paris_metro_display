#ifndef topo_h_HAS_ALREADY_BEEN_INCLUDED
#define topo_h_HAS_ALREADY_BEEN_INCLUDED
/**
 * @file
 * Local database for the station map
 * @addtogroup core
 * @{
 * @addtogroup topo
 * @{
 * @author gax
 *****************************************************************************
 */
#include <stdint.h>

#include "driver/fb.h"
#include "core/topo_stations.h"

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
// Public defines
// ---------------------------------------------------------------------------

/** Marker mask for indicating the route is going backwards */
#define ROUTE_BACKWARDS_MASK 0x80

// ---------------------------------------------------------------------------
// Public types
// ---------------------------------------------------------------------------

/** Direction for a route */
typedef enum { e_dir_forward, e_dir_backward } direction_t;
   
/** 
 * Short hand for the route info index.
 * Forward route are positive numbers whereas backwards is the same as
 *  the forward with msb set.
 */
typedef enum
{
   A3_A4=0,
   A3_A2,
   A5_A4,
   A5_A2,
   A1_A4, 
   A1_A2,
   ROUTE_ID_MAX_FORWARD,
   A4_A3 = ROUTE_BACKWARDS_MASK | A3_A4,
   A2_A3 = ROUTE_BACKWARDS_MASK | A3_A2,
   A4_A5 = ROUTE_BACKWARDS_MASK | A5_A4,
   A2_A5 = ROUTE_BACKWARDS_MASK | A5_A2,
   A4_A1 = ROUTE_BACKWARDS_MASK | A1_A4,
   A2_A1 = ROUTE_BACKWARDS_MASK | A1_A2,
   INVALID_ROUTE = -1, //< Invalid or out of array
} route_id_t;

/** Index for the sequencer */
typedef uint_fast8_t tiny_index_t;

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

/** Grab a led on any route at the given 0 based station index */
fb_index_t topo_get_led(tiny_index_t stationPosition, route_id_t route);

/** Grab the station offset on a route. */
tiny_index_t topo_get_offset(const fb_index_t station, route_id_t routeId );

#ifdef __cplusplus
}
#endif

 /**@}*//**@}*/
#endif /* ndef topo_h_HAS_ALREADY_BEEN_INCLUDED */