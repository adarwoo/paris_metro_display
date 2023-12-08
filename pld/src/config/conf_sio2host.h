#ifndef CONF_SIO2HOST_H_INCLUDED
#define CONF_SIO2HOST_H_INCLUDED

#define SERIAL_RX_BUF_SIZE_HOST   156

#define USART_HOST                &USARTD0
#define USART_HOST_BAUDRATE       9600
#define USART_HOST_CHAR_LENGTH    USART_CHSIZE_8BIT_gc
#define USART_HOST_PARITY         USART_PMODE_DISABLED_gc
#define USART_HOST_STOP_BITS      false

#define USART_HOST_RX_ISR_ENABLE() \
   usart_set_rx_interrupt_level( \
      USART_HOST, \
		USART_INT_LVL_MED)
      
#define USART_HOST_ISR_VECT()      ISR(USARTD0_RXC_vect)

#endif /* CONF_SIO2HOST_H_INCLUDED */
