/*
 * VTS_Project_Config.h
 *
 * Created: 29-01-2020 19:53:31
 *  Author: Somashekara V
 */ 


#ifndef VTS_PROJECT_CONFIG_H_
#define VTS_PROJECT_CONFIG_H_


//#define USART0 (0x40024000U) //(0x40024000U) /**< \brief (USART0) Base Address */
#define USART1 (0x40028000U) /**< \brief (USART1) Base Address */
#define USART2 (0x4002C000U) /**< \brief (USART1) Base Address */

#define ID_USART0 (13) //13/**< \brief USART 0 (USART0) */
#define ID_USART1 (14) /**< \brief USART 1 (USART1) */
#define ID_USART2 (15) /**< \brief USART 2 (USART2) */
#define ID_TWIHS0 (19) /**< \brief TWIHS 0 (TWIHS0) */

/* A reference setting for UART1 as console*/
//#define CONSOLE_UART               USART1
//#define CONSOLE_UART_ID            ID_USART1
// #define CONSOLE_USART               USART1
// #define CONSOLE_USART_ID            ID_USART1
// #define CONSOLE_USART_BAUDRATE		115200

#define CONSOLE_UART               UART0
#define CONSOLE_UART_ID            ID_UART0
#define CONSOLE_UART_BAUDRATE		115200


/** Enable Com Port. */
#define CONF_BOARD_UART_CONSOLE

/** Definition of TWI interrupt ID on board. */                                                 
#define BOARD_TWIHS_IRQn          TWIHS0_IRQn
#define BOARD_TWIHS_Handler    TWIHS2_Handler

/** Configure TWI2 pins */
#define CONF_BOARD_TWIHS0

/** Configure QSPI PINS */
#define CONF_BOARD_QSPI

/** Configure SPI PINS */
#define CONF_BOARD_SPI

/* USART0 module is used in SYNC. mode. */
//#define CONF_BOARD_USART_RXD
//#define CONF_BOARD_USART_TXD
//#define CONF_BOARD_USART_SCK

#define __FPU_USED 1


/* A reference setting USART0 & USART2 for GPS & GSM*/
/** Baudrate setting */
#define CONF_USART0_BAUDRATE     115200
#define CONF_USART1_BAUDRATE     115200
#define CONF_USART2_BAUDRATE     9600

/** Character length setting */
#define CONF_UART_CHAR_LENGTH  (0x3u << 6)
/** Parity setting */
#define CONF_UART_PARITY       (0x4u << 9) 
/** Stop bits setting */
#define CONF_UART_STOP_BITS    (0x0u << 12)

//////////////////////////////////////for USB
/*! \name GPIO Connections of VBUS monitoring
 */
// ! @{
#define USB_VBUS_FLAGS         (PIO_INPUT | PIO_PULLUP)
#define USB_VBUS_PIN             PIO_PC9_IDX  /* As IO pin input */
#define USB_VBUS_PIN_IRQn ( PIOC_IRQn)
#define USB_VBUS_PIO_ID       ID_PIOC
#define USB_VBUS_PIO_MASK  PIO_PC9 
//! @}

/*! \name GPIO Connections of ID detecting
 */
//! @{
#define USB_ID_FLAGS             (PIO_INPUT | PIO_PULLUP)
#define USB_ID_PIN               PIO_PC16_IDX /* As IO pin input */
#define USB_ID_PIN_IRQn     (PIOC_IRQn)
#define USB_ID_PIO_ID         ID_PIOC
#define USB_ID_PIO_MASK    PIO_PC16
// ! @}
//////////////////////////////////////for USB



/** GPS Device Connectivity **/
//#define BOARD_ID_USART             ID_USART0
//#define BOARD_USART                USART0

//#define USART_Handler              USART0_Handler
//#define GSM_Handler                USART2_Handler


enum deviceType {
	GSM_Device,
	GPS_Device
};

enum interfaceType {
	IF_USART0,
	IF_USART1,
	IF_USART2,
	IF_UART0,
	IF_UART1,
	IF_UART2
};

#endif /* VTS_PROJECT_CONFIG_H_ */