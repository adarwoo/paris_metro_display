/**
 * @addtogroup driver
 * @{
 * @addtogroup led
 * @{
 *****************************************************************************
 * GPS led management
 * Blink the board 1PPS led when detecting activity on the 1PPS signal.
 *****************************************************************************
 * @file
 * Implementation of the led API
 * @author gax
 * @internal
 */
#include <asf.h>

#include "lib/timer.h"
#include "lib/reactor.h"
#include "driver/led.h"


/** Keep the led on for the given number of ms */
#define LED_PERSIST_DURATION TIMER_MILLISECONDS(500)

/** Handle for the reactor to process the 1pps tick */
reactor_handle_t handle_pps = 0;


/** Called by the timer to turn off an LED */
static void _turn_one_pps_off(timer_instance_t i, void *arg)
{
   // Turn LED on
   ioport_set_pin_level( ONE_PPS_LED, false);
}

/**
 * Reactor handler
 */
static void _pps_handler(void)
{
   // Turn LED on
   ioport_set_pin_level( ONE_PPS_LED, true);
   
   // Arm a timer to turn it off in 500ms
   timer_arm_from_now(_turn_one_pps_off, LED_PERSIST_DURATION, 0);
}

/**
 * The 1PPS led is configured to replicate the pulsating output of the
 *  GPS chip. Since the pulse is narrow, it is widened by the interrupt
 *  using the pulse ioctl of the output driver.
 */
void led_init( void )
{
   PORT_t *base = arch_ioport_pin_to_base( GPS_1PPS_SIGNAL );
   
   // Get a reactor slot to deal with this event outside of interrupts
   handle_pps = reactor_register( &_pps_handler );
   
   // Set the LED pin as output
   ioport_set_pin_dir( ONE_PPS_LED, IOPORT_DIR_OUTPUT );

   // Pin is not inverted
   ioport_set_pin_dir( GPS_1PPS_SIGNAL, IOPORT_DIR_INPUT );

   // Turn on the pull down on the pin to guarantee a 0
   //  even if the GPS chip is missing.
   ioport_set_pin_mode( GPS_1PPS_SIGNAL, IOPORT_MODE_PULLDOWN );

   // Detect changes on rising edge
   ioport_set_pin_sense_mode( GPS_1PPS_SIGNAL, IOPORT_SENSE_RISING );

   // Enable interrupt on pin ONE_PPS
   base->INTCTRL = PORT_INT0LVL_MED_gc;
   base->INT0MASK = ioport_pin_to_mask( GPS_1PPS_SIGNAL );
}

/** Deal with a change on the 1pps pin */
ISR( PORTA_INT0_vect )
{
   reactor_notify(handle_pps);
}

/**@}*/
/**@} ---------------------------  End of file  --------------------------- */