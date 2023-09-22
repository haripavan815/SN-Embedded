/************************************************************************************************************************************/
/*																																	*/
/*  File Name:   usart_app.c                                                                                                         */
/*                                                                                                                                  */
/*  Description:  Contains usart related functionalities.
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
#include <string.h>
#include "VTS_Project_Config.H"
#include "usart_app.h"

extern volatile bool IsSystemBooted, IsUSARTConfigured;
	
QueueHandle_t xQueue_usart0_response = NULL;
QueueHandle_t xQueue_usart1_response = NULL;

uint8_t tx_buffer_usart0[TRANSFER_SIZE] = {0};
uint8_t rx_buffer_usart0[TRANSFER_SIZE] = {0};

uint8_t tx_buffer_usart1[TRANSFER_SIZE] = {0};
uint8_t rx_buffer_usart1[TRANSFER_SIZE] = {0};
	
bool tx_completed_usart0 = true;
bool rx_completed_usart0 = false;

bool tx_completed_usart1 = true;
bool rx_completed_usart1 = false;

/** XDMAC channel configuration. */
static xdmac_channel_config_t xdmac_tx_cfg,xdmac_rx_cfg;
IsGpsUartInitialized = false;
IsBoardHealthSuccess = false;

bool InitialiseUsart0Queue()
{
	xQueue_usart0_response = xQueueCreate( 4, TRANSFER_SIZE );
	if( xQueue_usart0_response == 0 )
	{
		printf("Failed to create the xQueue_usart0_response\n");
	}
}

bool InitialiseUsart1Queue()
{
	xQueue_usart1_response = xQueueCreate( 4, TRANSFER_SIZE );
	if( xQueue_usart1_response == 0 )
	{
		printf("Failed to create the xQueue_usart1_response\n");
	}
}

void configure_usart(uint32_t DevID, uint32_t ul_baudrate)
{
	uint32_t timeout = 10;
	Usart* devPtr = NULL; 
	
	sam_usart_opt_t usart_console_settings = {
		ul_baudrate,
		US_MR_CHRL_8_BIT,
		US_MR_PAR_NO,
		US_MR_NBSTOP_1_BIT,
		US_MR_CHMODE_NORMAL,
		/* This field is only used in IrDA mode. */
		0
	};

	//Check the correctness of device ID
	switch (DevID) {
		case 13: devPtr = USART0; break;
		case 14: devPtr = USART1; break;
		//case 15: devPtr = USART2; break;
		default:
		printf("\nUSART Device ID is unknown");
		return;
	}
	/* Enable the peripheral clock in the PMC. */
	sysclk_enable_peripheral_clock(DevID);

	usart_init_rs232(devPtr, &usart_console_settings,sysclk_get_peripheral_hz());

	/* Disable all the interrupts. */
	usart_disable_interrupt(devPtr, 0xffffffff);

	/* Enable TX & RX function. */
	usart_enable_tx(devPtr);
	usart_enable_rx(devPtr);

	//Receive timeout for variable length of receive frame
	usart_set_rx_timeout(devPtr, timeout);
	
	//usart_enable_interrupt(devPtr,  US_IER_RXRDY | US_IER_TIMEOUT | US_IER_RXBRK);
	usart_enable_interrupt(devPtr, US_IER_TIMEOUT);

	/* Configure and enable interrupt of USART. */
	NVIC_ClearPendingIRQ((IRQn_Type)DevID);
	NVIC_DisableIRQ((IRQn_Type)DevID);
	NVIC_SetPriority((IRQn_Type)DevID, configMAX_SYSCALL_INTERRUPT_PRIORITY-1);
	NVIC_EnableIRQ((IRQn_Type)DevID);
	
	IsUSARTConfigured = true;
}

/**
 * \brief configure xdmac for USART0 and ready to transfer.
 *
 * \param puart  Base address of the USART
 */
void usart0_xdmac_configure_tx(Usart *const puart, const uint8_t* tx_buffer_usart0, uint32_t len_tx)
{	
	uint32_t xdmaint;

	/* Initialize and enable DMA controller */
	pmc_enable_periph_clk(ID_XDMAC);

	xdmaint = (XDMAC_CIE_BIE |
		XDMAC_CIE_DIE   |
		XDMAC_CIE_FIE   |
		XDMAC_CIE_RBIE  |
		XDMAC_CIE_WBIE  |
		XDMAC_CIE_ROIE);

	/* Initialize channel config for transmitter */
	xdmac_tx_cfg.mbr_ubc = 1;
	xdmac_tx_cfg.mbr_sa = (uint32_t)tx_buffer_usart0;
	xdmac_tx_cfg.mbr_da = (uint32_t)&(puart->US_THR);
	xdmac_tx_cfg.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
		XDMAC_CC_MBSIZE_SINGLE |
		XDMAC_CC_DSYNC_MEM2PER |
		XDMAC_CC_CSIZE_CHK_1 |
		XDMAC_CC_DWIDTH_BYTE |
		XDMAC_CC_SIF_AHB_IF0 |
		XDMAC_CC_DIF_AHB_IF1 |
		XDMAC_CC_SAM_INCREMENTED_AM |
		XDMAC_CC_DAM_FIXED_AM |
		XDMAC_CC_PERID(USART0_XDMAC_TX_CH_NUM);

	xdmac_tx_cfg.mbr_bc = len_tx;
	xdmac_tx_cfg.mbr_ds =  0;
	xdmac_tx_cfg.mbr_sus = 0;
	xdmac_tx_cfg.mbr_dus = 0;

	xdmac_configure_transfer(XDMAC, XDMAC_TX_CH, &xdmac_tx_cfg);

	xdmac_channel_set_descriptor_control(XDMAC, XDMAC_TX_CH, 0);
		
	xdmac_channel_enable_interrupt(XDMAC, XDMAC_TX_CH, xdmaint);
	
	xdmac_channel_enable(XDMAC, XDMAC_TX_CH);
	xdmac_enable_interrupt(XDMAC, XDMAC_TX_CH);

	/*Enable XDMAC interrupt */
	NVIC_ClearPendingIRQ(XDMAC_IRQn);
	NVIC_SetPriority( XDMAC_IRQn ,configMAX_SYSCALL_INTERRUPT_PRIORITY-1);
	NVIC_EnableIRQ(XDMAC_IRQn);	

}

/**
 * \brief configure xdmac for USART0 and ready to receive.
 *
 * \param puart  Base address of the USART
 */
void usart0_xdmac_configure_rx(Usart *const puart, const uint8_t* rx_buffer_usart0, uint32_t len_rx)
{	
	memset(rx_buffer_usart0, 0 , TRANSFER_SIZE);	
	
	uint32_t xdmaint;

	/* Initialize and enable DMA controller */
	pmc_enable_periph_clk(ID_XDMAC);

	xdmaint = (XDMAC_CIE_BIE |
		XDMAC_CIE_DIE   |
		XDMAC_CIE_FIE   |
		XDMAC_CIE_RBIE  |
		XDMAC_CIE_WBIE  |
		XDMAC_CIE_ROIE);

	/* Initialize channel config for receiver */
	xdmac_rx_cfg.mbr_ubc = 1;
	xdmac_rx_cfg.mbr_da = (uint32_t)rx_buffer_usart0;
	xdmac_rx_cfg.mbr_sa = (uint32_t)&puart->US_RHR;
	xdmac_rx_cfg.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
		XDMAC_CC_MBSIZE_SINGLE |
		XDMAC_CC_DSYNC_PER2MEM |
		XDMAC_CC_CSIZE_CHK_1 |
		XDMAC_CC_DWIDTH_BYTE|
		XDMAC_CC_SIF_AHB_IF1 |
		XDMAC_CC_DIF_AHB_IF0 |
		XDMAC_CC_SAM_FIXED_AM |
		XDMAC_CC_DAM_INCREMENTED_AM |
		XDMAC_CC_PERID(USART0_XDMAC_RX_CH_NUM);

	xdmac_rx_cfg.mbr_bc = len_rx;
	xdmac_tx_cfg.mbr_ds =  0;
	xdmac_rx_cfg.mbr_sus = 0;
	xdmac_rx_cfg.mbr_dus = 0;

	xdmac_configure_transfer(XDMAC, XDMAC_RX_CH, &xdmac_rx_cfg);
	xdmac_channel_set_descriptor_control(XDMAC, XDMAC_RX_CH, 0);
	xdmac_channel_enable_interrupt(XDMAC, XDMAC_RX_CH, xdmaint);
	xdmac_channel_enable(XDMAC, XDMAC_RX_CH);
	xdmac_enable_interrupt(XDMAC, XDMAC_RX_CH);

	/*Enable XDMAC interrupt */
	NVIC_ClearPendingIRQ(XDMAC_IRQn);
	NVIC_SetPriority( XDMAC_IRQn ,configMAX_SYSCALL_INTERRUPT_PRIORITY-1);
	NVIC_EnableIRQ(XDMAC_IRQn);
}	

/**
 * \brief configure xdmac for USART1 and ready to transfer.
 *
 * \param puart  Base address of the USART
 */
void usart1_xdmac_configure_tx(Usart *const puart, const uint8_t* tx_buffer_usart1, uint32_t len_tx)
{	
	uint32_t xdmaint;

	/* Initialize and enable DMA controller */
	pmc_enable_periph_clk(ID_XDMAC);

	xdmaint = (XDMAC_CIE_BIE |
		XDMAC_CIE_DIE   |
		XDMAC_CIE_FIE   |
		XDMAC_CIE_RBIE  |
		XDMAC_CIE_WBIE  |
		XDMAC_CIE_ROIE);

	/* Initialize channel config for transmitter */
	xdmac_tx_cfg.mbr_ubc = 1;
	xdmac_tx_cfg.mbr_sa = (uint32_t)tx_buffer_usart1;
	xdmac_tx_cfg.mbr_da = (uint32_t)&(puart->US_THR);
	xdmac_tx_cfg.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
		XDMAC_CC_MBSIZE_SINGLE |
		XDMAC_CC_DSYNC_MEM2PER |
		XDMAC_CC_CSIZE_CHK_1 |
		XDMAC_CC_DWIDTH_BYTE |
		XDMAC_CC_SIF_AHB_IF0 |
		XDMAC_CC_DIF_AHB_IF1 |
		XDMAC_CC_SAM_INCREMENTED_AM |
		XDMAC_CC_DAM_FIXED_AM |
		XDMAC_CC_PERID(USART1_XDMAC_TX_CH_NUM);

	xdmac_tx_cfg.mbr_bc = len_tx;
	xdmac_tx_cfg.mbr_ds =  0;
	xdmac_tx_cfg.mbr_sus = 0;
	xdmac_tx_cfg.mbr_dus = 0;

	xdmac_configure_transfer(XDMAC, XDMAC_TX_CH1, &xdmac_tx_cfg);

	xdmac_channel_set_descriptor_control(XDMAC, XDMAC_TX_CH1, 0);
		
	xdmac_channel_enable_interrupt(XDMAC, XDMAC_TX_CH1, xdmaint);
	
	xdmac_channel_enable(XDMAC, XDMAC_TX_CH1);
	xdmac_enable_interrupt(XDMAC, XDMAC_TX_CH1);

	/*Enable XDMAC interrupt */
	NVIC_ClearPendingIRQ(XDMAC_IRQn);
	NVIC_SetPriority( XDMAC_IRQn ,configMAX_SYSCALL_INTERRUPT_PRIORITY-1);
	NVIC_EnableIRQ(XDMAC_IRQn);	

}

/**
 * \brief configure xdmac for USART1 and ready to receive.
 *
 * \param puart  Base address of the USART
 */
void usart1_xdmac_configure_rx(Usart *const puart, const uint8_t* rx_buffer_usart1, uint32_t len_rx)
{	
	memset(rx_buffer_usart1, 0 , TRANSFER_SIZE);	
	
	uint32_t xdmaint;

	/* Initialize and enable DMA controller */
	pmc_enable_periph_clk(ID_XDMAC);

	xdmaint = (XDMAC_CIE_BIE |
		XDMAC_CIE_DIE   |
		XDMAC_CIE_FIE   |
		XDMAC_CIE_RBIE  |
		XDMAC_CIE_WBIE  |
		XDMAC_CIE_ROIE);

	/* Initialize channel config for receiver */
	xdmac_rx_cfg.mbr_ubc = 1;
	xdmac_rx_cfg.mbr_da = (uint32_t)rx_buffer_usart1;
	xdmac_rx_cfg.mbr_sa = (uint32_t)&puart->US_RHR;
	xdmac_rx_cfg.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
		XDMAC_CC_MBSIZE_SINGLE |
		XDMAC_CC_DSYNC_PER2MEM |
		XDMAC_CC_CSIZE_CHK_1 |
		XDMAC_CC_DWIDTH_BYTE|
		XDMAC_CC_SIF_AHB_IF1 |
		XDMAC_CC_DIF_AHB_IF0 |
		XDMAC_CC_SAM_FIXED_AM |
		XDMAC_CC_DAM_INCREMENTED_AM |
		XDMAC_CC_PERID(USART1_XDMAC_RX_CH_NUM);

	xdmac_rx_cfg.mbr_bc = len_rx;
	xdmac_tx_cfg.mbr_ds =  0;
	xdmac_rx_cfg.mbr_sus = 0;
	xdmac_rx_cfg.mbr_dus = 0;

	xdmac_configure_transfer(XDMAC, XDMAC_RX_CH1, &xdmac_rx_cfg);

	xdmac_channel_set_descriptor_control(XDMAC, XDMAC_RX_CH1, 0);
	xdmac_channel_enable_interrupt(XDMAC, XDMAC_RX_CH1, xdmaint);

	xdmac_channel_enable(XDMAC, XDMAC_RX_CH1);
	xdmac_enable_interrupt(XDMAC, XDMAC_RX_CH1);

	/*Enable XDMAC interrupt */
	NVIC_ClearPendingIRQ(XDMAC_IRQn);
	NVIC_SetPriority( XDMAC_IRQn ,configMAX_SYSCALL_INTERRUPT_PRIORITY-1);
	NVIC_EnableIRQ(XDMAC_IRQn);
}	
void USART0_Handler(void)
{	
	uint32_t ul_status;
	
	/* Read USART0 Status. */
	ul_status = usart_get_status(USART0);
	
	if(ul_status & US_CSR_TIMEOUT) {
		//printf("USART0 timeout received\n");
		
		//Clear the timeout interrupt
		usart_start_rx_timeout(USART0);

		// No need to handle before tasks setup
		if(!IsSystemBooted) return;
				
		static signed long xHigherPriorityTaskWoken;

		xHigherPriorityTaskWoken = pdFALSE;
		xQueueSendFromISR( xQueue_usart0_response, rx_buffer_usart0 , &xHigherPriorityTaskWoken );

		usart0_xdmac_configure_rx(USART0, rx_buffer_usart0, TRANSFER_SIZE);
				
		if(xHigherPriorityTaskWoken)
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
				
	}
}

void USART1_Handler(void)
{	
	uint32_t ul_status;
	
	/* Read USART1 Status. */
	ul_status = usart_get_status(USART1);
	
	if(ul_status & US_CSR_TIMEOUT) {
		//printf("USART1 timeout received\n");
		
		//Clear the timeout interrupt
		usart_start_rx_timeout(USART1);
		
		// No need to handle before tasks setup
		if(!IsSystemBooted) return;
			
		static signed long xHigherPriorityTaskWoken;

		xHigherPriorityTaskWoken = pdFALSE;
		xQueueSendFromISR( xQueue_usart1_response, rx_buffer_usart1 , &xHigherPriorityTaskWoken );

		usart1_xdmac_configure_rx(USART1, rx_buffer_usart1, TRANSFER_SIZE);
		
		if(xHigherPriorityTaskWoken)
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		
	}
}