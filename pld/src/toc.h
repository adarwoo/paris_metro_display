/**
 * @file
 * Table of contents for the Doxygen documentation.
 * @mainpage PLD
 * @section Introduction
 * This project contains the source code for the PLD project which aims
 *  at turning  a Paris RER A dynamic metro plan into a decorative piece of
 *  furniture, that additionally provides the time and the temperature and
 *  various animation modes.
 * @section Description
 * The code is articulated around services and drivers.
 * The overall architectural decision is the use of a [reactor](group__reactor.html) engine
 *  to drive the application. The reactor allow running in the main context
 *  (as opposed to interrupt context), handlers for events such as:
 *  - timers
 *  - key presses
 *
 * The reactor is used by a [sequencer](group__sequencer.html) which delegates to mode handlers
 *  the job of displaying information.
 * The 46 LEDs of the board are automatically driver by the [framebuffer](group_framebuffer.html) API
 *  made of DMA and interrupts.
 * Led level, blinking, turning off etc. is autonomous.
 * @section Configuration
 * New modes and displays can be added to the application easily.
 * @n
 * The displays are managed by modes and the modes are managed by the sequencer.
 * The mode sequence can be edited from the configuration.hpp file which defines
 *  the sequence of mode toggled by a short key push and the demo mode activated
 *  by a long push.
 *
 * To modify the file core/configuration.hpp, proceed as follow :
 *  - In the 'Forward declarations' block, make sure all the modes to use are
 *      declared.
 *  - You can then add the new mode to the short_push_sequence or as the 
 *      long_push_mode.
 * Tip: To test a newly designed mode, it is best to set it as the first mode
 * of the short_push_sequence
 */
 
 /**
  * @defgroup core core
  * Application specific software.
  * @{
  * @defgroup mode mode
  * Regroups the various display mode implementations
  * @}
  * @defgroup driver driver
  * Regroups the low and higher level drivers for the various hardware devices.
  * @defgroup service service
  * Regroups services and helpers which are not application specific.
  */