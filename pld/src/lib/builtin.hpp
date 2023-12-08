#ifndef lib_builtin_h_HAS_ALREADY_BEEN_INCLUDED
#define lib_builtin_h_HAS_ALREADY_BEEN_INCLUDED
/**
 * @addtogroup service
 * @{
 * @addtogroup builtin
 * @{
 *****************************************************************************
 * Requiered to use placement new operator. If these functions are not
 *  defined, the error complains about new having 2 parameters instead of
 *  one.
 *****************************************************************************
 * @file
 * Prototypes for all C++ mandatory built-in functions for new operator
 * @author gax
 */

#include <stdlib.h>
#include <lib/alert.h>

/** Invoked craftingly when using a placement new */
inline void* operator new (size_t, void* p) 
   { return p; }

/** Placement delete is baned for this application */
inline void operator delete (void *, void *) 
   { alert_and_stop(); }

// Stop MSVC from defining its own placement new operator
#define __PLACEMENT_NEW_INLINE

/**@}*/
/**@}*/
#endif /* ndef lib_builtin_h_HAS_ALREADY_BEEN_INCLUDED */
