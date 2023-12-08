/**
 * @addtogroup service
 * @{
 * @addtogroup builtin
 * @{
 *****************************************************************************
 * Provide C++ builtin method that need supplying in the case of the AVR.
 *****************************************************************************
 * @file
 * Built-in C++ symbols
 * @author gax
 */

#include "lib/alert.h"

/** Called if a pure virtual method is called at run-time */
extern "C" void __cxa_pure_virtual() 
{
   alert_and_stop(); 
}

/**@}*/
/**@}*/
/* ---------------------------  End of file  --------------------------- */