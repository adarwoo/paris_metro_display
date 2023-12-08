/**
 * \file
 *
 * \brief User board initialization template
 *
 */

#include <asf.h>
#include <board.h>
#include <conf_board.h>

#include "driver/led.h"

void board_init(void)
{
   // Set the UART pins as the ASF API does not (shame)
   ioport_configure_pin(IOPORT_CREATE_PIN(PORTD, 3), IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
   ioport_configure_pin(IOPORT_CREATE_PIN(PORTD, 2), IOPORT_DIR_INPUT);

   // Peripherals and clocks
   pmic_init();
   sysclk_init();
   dma_enable();
   led_init();
   delay_init();
   
   cpu_irq_enable();
}
