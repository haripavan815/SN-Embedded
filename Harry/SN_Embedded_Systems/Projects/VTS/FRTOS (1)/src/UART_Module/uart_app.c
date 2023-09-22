/************************************************************************************************************************************/
/*																																	*/
/*  File Name:   uart_app.c                                                                                                         */
/*                                                                                                                                  */
/*  Description:  Contains uart related functionalities.
/*																					                                                */
/*																																	*/
/*  -----------------------------------------------------------------------------------------------------------------------------   */
/*  |  Author              |    Date      | Rev  |									Comments								|		*/
/*  -----------------------------------------------------------------------------------------------------------------------------	*/
/*  | Sanjay S N           |  12-16-2021  | 1.0  |          Created                                                         |       */
/*------------------------------------------------------------------------------------------------------------------------------	*/
/*																																	*/
/*  Copyright (c) 2021, SNES																										*/
/*  All rights reserved.																											*/
/*																																    */
/************************************************************************************************************************************/

#include <asf.h>
#include "uart_app.h"

/**
 * \brief Configure the console UART.
 */
void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
#if (defined CONF_UART_CHAR_LENGTH)
		.charlength = CONF_UART_CHAR_LENGTH,
#endif
		.paritytype = CONF_UART_PARITY,
#if (defined CONF_UART_STOP_BITS)
		.stopbits = CONF_UART_STOP_BITS,
#endif
	};
	/* Configure console UART. */
	stdio_serial_init(CONF_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
#if defined(__GNUC__)
	setbuf(stdout, NULL);
#else
	/* Already the case in IAR's Normal DLIB default configuration: printf()
	 * emits one character at a time.
	 */
#endif

}

void uart0_write_string(uint8_t* char_data, uint32_t size)
{
	for(int i = 0; i< size; i++)
	{
		uint8_t data = char_data[i];
		if(uart_write(UART0, data))
		{
			//delay_ms(1);
			i--;
		}
	}
}