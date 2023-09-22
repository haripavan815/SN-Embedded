
#ifndef USART_APP_H_
#define USART_APP_H_

// XDMAC channel number for USART0, USART1 and UART0
#define XDMAC_TX_CH 0
#define XDMAC_RX_CH 1

#define XDMAC_TX_CH1 2
#define XDMAC_RX_CH1 3

#define XDMAC_TX_CH_u 4
#define XDMAC_RX_CH_u 5

// XDMAC channel HW Interface number for USART0, USART1 and UART0
#define USART0_XDMAC_TX_CH_NUM  7
#define USART0_XDMAC_RX_CH_NUM  8

#define USART1_XDMAC_TX_CH_NUM  9
#define USART1_XDMAC_RX_CH_NUM  10

#define UART0_XDMAC_TX_CH_NUM  20
#define UART0_XDMAC_RX_CH_NUM  21

// XDMAC Buffer Transfer Size
#define TRANSFER_SIZE 512

void usart0_xdmac_configure_tx(Usart *const puart, const uint8_t* tx_buffer_usart0, uint32_t len_tx);
void usart0_xdmac_configure_rx(Usart *const puart, const uint8_t* rx_buffer_usart0, uint32_t len_rx);

void usart1_xdmac_configure_tx(Usart *const puart, const uint8_t* tx_buffer_usart1, uint32_t len_tx);
void usart1_xdmac_configure_rx(Usart *const puart, const uint8_t* rx_buffer_usart1, uint32_t len_rx);

bool InitialiseUsart0Queue();
bool InitialiseUsart1Queue();

void configure_usart(uint32_t DevID, uint32_t ul_baudrate);

#endif /* USART_APP_H_ */