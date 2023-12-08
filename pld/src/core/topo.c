/**
 * @file
 * Implementation of the topo API
 * @addtogroup core
 * @{
 * @addtogroup topo
 * API for managing the display topology
 * @{
 */
#include "driver/fb.h"

#include "core/topo.h"
#include "core/topo_routes.h"

// ---------------------------------------------------------------------------
// Local types
// ---------------------------------------------------------------------------

/** Route info */
typedef struct
{
   /** Pointer to the station (which is a list of led_t) */
   fb_index_t *start;

   /** Number of stations in the list */
   tiny_index_t length;
} route_info_t;


// ---------------------------------------------------------------------------
// Local data
// ---------------------------------------------------------------------------

/** 
 * Route database table
 * Columns are:
 *  - route id
 *  - number of stations on route
 */
static const route_info_t _routes_info[] = {
   { route_A3_A4, sizeof(route_A3_A4) },
   { route_A3_A2, sizeof(route_A3_A2) },
   { route_A5_A4, sizeof(route_A5_A4) },
   { route_A5_A2, sizeof(route_A5_A2) },
   { route_A1_A4, sizeof(route_A1_A4) },
   { route_A1_A2, sizeof(route_A1_A2) },
   {0}
};


/**
 * @param stationIndex 0 based index of the station on the route
 * @param routeId Starting route 1 to
 */
fb_index_t topo_get_led( tiny_index_t stationIndex, route_id_t routeId )
{
   fb_index_t retval = TOPO_OUT_OF_RANGE;
   route_id_t forwardRoute = routeId & (~ROUTE_BACKWARDS_MASK);
   
   if ( forwardRoute < ROUTE_ID_MAX_FORWARD )
   {
      // Which way are we reading the route?
      tiny_index_t routeNumberOfStations = _routes_info[forwardRoute].length;
      
      if ( stationIndex < routeNumberOfStations )
      {
         direction_t dir = (forwardRoute == routeId) ? e_dir_forward : e_dir_backward;
         
         if ( dir == e_dir_forward )
         {
            retval = _routes_info[forwardRoute].start[stationIndex];
         }
         else
         {
            retval = _routes_info[forwardRoute].start[routeNumberOfStations-stationIndex-1];
         }
      }
      else
      {
         retval = TOPO_OUT_OF_RANGE;
      }
   }
   
   return retval;
}

/**
 * @param station Station to position
 * @param routeId Id of the route
 */
tiny_index_t topo_get_offset(const fb_index_t station, route_id_t routeId )
{
   tiny_index_t retval = TOPO_OUT_OF_RANGE;
   
   route_id_t forwardRoute = routeId & (~ROUTE_BACKWARDS_MASK);
   
   // Only deal with valid values
   if ( forwardRoute < ROUTE_ID_MAX_FORWARD )
   {
      // Look for the station in the route
      fb_index_t *pStart = _routes_info[forwardRoute].start;
      register tiny_index_t index = 0;
      register tiny_index_t count = _routes_info[forwardRoute].length;
      
      while ( index < count )
      {
         if ( pStart[index] == station )
         {
            direction_t dir = (forwardRoute == routeId) ? e_dir_forward : e_dir_backward;
            
            if ( dir == e_dir_forward )
            {
               retval = index;
            }
            else
            {
               retval = count - index;
            }
            
            // That's it
            break;
         }
         
         ++index;
      }
   }      
   
   return retval;
}

 /**@}*/
 /**@} ---------------------------  End of file  --------------------------- */