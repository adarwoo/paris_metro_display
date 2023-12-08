#ifndef CONF_BOARD_H
#define CONF_BOARD_H
/**
 * \file
 *
 * \brief User board configuration template
 *
 */

/************************************************************************/
/* General purpose timer API                                            */
/************************************************************************/
#define TIMER_TC           TCC0

// Limit the number of timers to catch race more easily when debugging
#ifdef DEBUG
#  define TIMER_MAX_CALLBACK 16
#else
#  define TIMER_MAX_CALLBACK 64
#endif

/************************************************************************/
/* HC595s shift register + ULN2803 darlington driver configuration      */
/************************************************************************/

/** DMA channel used for shift reg transfers */
#define HC595_DMA_CHANNEL  0

/** USART used to talk to the shift registers */
#define HC595_USART        USARTC1

/** Shift registers data in */
#define HC595_TX           IOPORT_CREATE_PIN(PORTC, 7)
/** Main shift registers data clock */
#define HC595_CLK          IOPORT_CREATE_PIN(PORTC, 5)
/** Shift registers latch clock (i.e. internal shift reg to output latch */
#define HC595_LATCH        IOPORT_CREATE_PIN(PORTC, 4)


/************************************************************************/
/* Framebuffer configuration                                            */
/************************************************************************/

/** External HC595+ULN2803 drivers */
#define FB_NUMBER_OF_DRIVERS 6

/** Bits per driver */
#define FB_BITS_PER_DRIVER   8

/** Max number of supported LEDs */
#define FB_NUMBER_OF_LEDS (FB_NUMBER_OF_DRIVERS * FB_BITS_PER_DRIVER)

/** Timer to use to sequence the frame buffer */
#define FB_TIMER_TC       TCC1

/************************************************************************/
/* Test pins                                                           */
/************************************************************************/
#define TP1          IOPORT_CREATE_PIN(PORTC, 0)
#define TP2          IOPORT_CREATE_PIN(PORTD, 4)
#define TP3          IOPORT_CREATE_PIN(PORTD, 5)
#define TP8          IOPORT_CREATE_PIN(PORTC, 1)
#define TP9          IOPORT_CREATE_PIN(PORTC, 1)

/************************************************************************/
/* Alert config                                                         */
/************************************************************************/

/** Alert if turns on */
#define ALERT_OUTPUT_PIN   TP1

/************************************************************************/
/* Debug pin by function                                                */
/************************************************************************/

/** Debug pin for the reactor idle */
#define DEBUG_REACTOR_IDLE TP2

/** Debug pin for the reactor */
#define DEBUG_REACTOR_BUSY TP3

/** Debug pin for the frame buffer */
#define DEBUG_FB           TP9

/************************************************************************/
/* Texas Tmp100 temperature sensor                                      */
/************************************************************************/

/** TWI device used to communicate with the tmp100 */
#define TMP100_TWI TWIE

/** Device i2c address, with pin ADD1 tied to ground and ADD2 left floating */
#define TMP100_I2C_ADDR 0b1001001

/************************************************************************/
/* MAX1036 i2c ADC                                                      */
/************************************************************************/

/** TWI device used to communicate with the tmp100 */
#define MAX1036_TWI TWIE

/************************************************************************/
/* Key configuration                                                    */
/************************************************************************/

/** Pin connected to the push-button */
#define KEY_PIN      IOPORT_CREATE_PIN(PORTC, 3)

/** Timer to use to sequence the key sampling */
#define KEY_TIMER_TC TCD0

/************************************************************************/
/* GPS configuration                                                    */
/************************************************************************/

/** Extended 1pps blink from the GPS */
#define ONE_PPS_LED        IOPORT_CREATE_PIN(PORTA, 1)

/** 1PPS input from the gps module */
#define GPS_1PPS_SIGNAL    IOPORT_CREATE_PIN(PORTA, 6)

/** Reset of the gps module */
#define GPS_N_RESET        IOPORT_CREATE_PIN(PORTB, 1)

/** Timer used for the GPS serial port sampling */
#define GPS_TIMER_TC       TCD1

#endif // CONF_BOARD_H
