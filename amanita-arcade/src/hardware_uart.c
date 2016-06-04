/*
 * hardware_uart.c
 *
 *  Created on: Jun 4, 2016
 *      Author: jlunder
 */

#include "hardware.h"

#include "hardware_internal.h"

static hw_usart_struct_t hw_usart_1;
static hw_usart_struct_t hw_usart_2;
static hw_usart_struct_t hw_usart_3;
static hw_usart_struct_t hw_uart_4;
static hw_usart_struct_t hw_uart_5;
static hw_usart_struct_t hw_usart_6;

hw_assignment_id_t hw_uart_assign(hw_resource_id_t uart,
		hw_resource_id_t tx_pin, hw_resource_id_t rx_pin,
		hw_resource_id_t cts_pin, hw_resource_id_t rts_pin) {
	hw_assignment_id_t id;
	hw_usart_struct_t * uarts = NULL;

	cu_verify(tx_pin != HWR_NONE || tx_pin != HWR_NONE);

	switch(uart) {
	case HWR_USART1:
		cu_verify(tx_pin == HWR_PA9 || tx_pin == HWR_PB6 || tx_pin == HWR_NONE);
		cu_verify(rx_pin == HWR_PA10 || rx_pin == HWR_PB7 || rx_pin == HWR_NONE);
		cu_verify(cts_pin == HWR_PA11 || cts_pin == HWR_NONE);
		cu_verify(rts_pin == HWR_PA12 || rts_pin == HWR_NONE);
		uarts = &hw_usart_1;
		uarts->usart_handle.Instance = USART1;
		break;
	case HWR_USART2:
		cu_verify(tx_pin == HWR_PA2 || tx_pin == HWR_PD5 || tx_pin == HWR_NONE);
		cu_verify(rx_pin == HWR_PA3 || rx_pin == HWR_PD6 || rx_pin == HWR_NONE);
		cu_verify(cts_pin == HWR_PA0 || cts_pin == HWR_PD3 || cts_pin == HWR_NONE);
		cu_verify(rts_pin == HWR_PA1 || rts_pin == HWR_PD4 || rts_pin == HWR_NONE);
		uarts = &hw_usart_2;
		uarts->usart_handle.Instance = USART2;
		break;
	case HWR_USART3:
		cu_verify(tx_pin == HWR_PB10 || tx_pin == HWR_PC10 || tx_pin == HWR_PD8 || tx_pin == HWR_NONE);
		cu_verify(rx_pin == HWR_PB11 || rx_pin == HWR_PC11 || rx_pin == HWR_PD9 || rx_pin == HWR_NONE);
		cu_verify(cts_pin == HWR_PB13 || cts_pin == HWR_PD11 || cts_pin == HWR_NONE);
		cu_verify(rts_pin == HWR_PB14 || rts_pin == HWR_PD12 || rts_pin == HWR_NONE);
		uarts = &hw_usart_3;
		uarts->usart_handle.Instance = USART3;
		break;
	case HWR_UART4:
		cu_verify(tx_pin == HWR_PA0 || tx_pin == HWR_PC10 || tx_pin == HWR_NONE);
		cu_verify(rx_pin == HWR_PA1 || rx_pin == HWR_PC11 || rx_pin == HWR_NONE);
		cu_verify(cts_pin == HWR_NONE);
		cu_verify(rts_pin == HWR_NONE);
		uarts = &hw_uart_4;
		uarts->usart_handle.Instance = UART4;
		break;
	case HWR_UART5:
		cu_verify(tx_pin == HWR_PC12 || tx_pin == HWR_NONE);
		cu_verify(rx_pin == HWR_PD2 || rx_pin == HWR_NONE);
		cu_verify(cts_pin == HWR_NONE);
		cu_verify(rts_pin == HWR_NONE);
		uarts = &hw_uart_5;
		uarts->usart_handle.Instance = UART5;
		break;
	case HWR_USART6:
		cu_verify(tx_pin == HWR_PC6 || tx_pin == HWR_PG14 || tx_pin == HWR_NONE);
		cu_verify(rx_pin == HWR_PC7 || rx_pin == HWR_PG9 || rx_pin == HWR_NONE);
		cu_verify(cts_pin == HWR_PG13 || cts_pin == HWR_PG15 || cts_pin == HWR_NONE);
		cu_verify(rts_pin == HWR_PG8 || rts_pin == HWR_PG12 || rts_pin == HWR_NONE);
		uarts = &hw_usart_6;
		uarts->usart_handle.Instance = USART6;
		break;
	default:
		cu_error("uart does not identify a UART or USART peripheral");
		break;
	}

	id = hw_resource_assign(uart, (intptr_t)uarts);
	memset(uarts, 0, sizeof uarts);
	if(tx_pin != HWR_NONE) {
		uarts->tx = hw_pin_assign(tx_pin);
	}
	if(rx_pin != HWR_NONE) {
		uarts->rx = hw_pin_assign(rx_pin);
	}
	if(cts_pin != HWR_NONE) {
		uarts->cts = hw_pin_assign(cts_pin);
	}
	if(rts_pin != HWR_NONE) {
		uarts->rts = hw_pin_assign(rts_pin);
	}

	switch(uart) {
	case HWR_USART1:
	case HWR_USART2:
	case HWR_USART3:
		if(tx_pin != HWR_NONE) {
			hw_pin_configure(uarts->tx, HWPM_AF_PP | HWPM_ALT_7);
		}
		if(rx_pin != HWR_NONE) {
			hw_pin_configure(uarts->rx, HWPM_AF_PP | HWPM_ALT_7);
		}
		if(cts_pin != HWR_NONE) {
			hw_pin_configure(uarts->cts, HWPM_AF_PP | HWPM_ALT_7);
		}
		if(rts_pin != HWR_NONE) {
			hw_pin_configure(uarts->rts, HWPM_AF_PP | HWPM_ALT_7);
		}
		break;
	case HWR_UART4:
	case HWR_UART5:
	case HWR_USART6:
		if(tx_pin != HWR_NONE) {
			hw_pin_configure(uarts->tx, HWPM_AF_PP | HWPM_ALT_8);
		}
		if(rx_pin != HWR_NONE) {
			hw_pin_configure(uarts->rx, HWPM_AF_PP | HWPM_ALT_8);
		}
		if(cts_pin != HWR_NONE) {
			hw_pin_configure(uarts->cts, HWPM_AF_PP | HWPM_ALT_8);
		}
		if(rts_pin != HWR_NONE) {
			hw_pin_configure(uarts->rts, HWPM_AF_PP | HWPM_ALT_8);
		}
		break;
	default:
		break;
	}

	uarts->started = false;
	hw_clock_enable(hw_resource_definitions[uart].clock_id);
	hw_clock_enable(hw_resource_definitions[uart].clock_id);

	return id;
}

void hw_uart_deassign(hw_assignment_id_t id) {
	hw_usart_struct_t * uarts = (hw_usart_struct_t *)hw_resource_get_user(id);

	if(uarts->started) {
		HAL_UART_DeInit(&uarts->uart_handle);
	}
}

void hw_uart_configure_format(hw_assignment_id_t id, int32_t baud_rate,
		hw_uart_word_length_t word_length, hw_uart_stop_length_t stop_length,
		hw_uart_parity_t parity) {
	hw_usart_struct_t * uarts = (hw_usart_struct_t *)hw_resource_get_user(id);

	if(uarts->tx == HW_ASSIGNMENT_ID_NULL &&
			uarts->rx == HW_ASSIGNMENT_ID_NULL) {
		cu_error("uarts in invalid state?");
	} else if(uarts->tx != HW_ASSIGNMENT_ID_NULL &&
			uarts->rx == HW_ASSIGNMENT_ID_NULL) {
		uarts->uart_handle.Init.Mode = UART_MODE_TX;
	} else if(uarts->tx == HW_ASSIGNMENT_ID_NULL &&
			uarts->rx != HW_ASSIGNMENT_ID_NULL) {
		uarts->uart_handle.Init.Mode = UART_MODE_RX;
	} else if(uarts->tx != HW_ASSIGNMENT_ID_NULL &&
			uarts->rx != HW_ASSIGNMENT_ID_NULL) {
		uarts->uart_handle.Init.Mode = UART_MODE_TX_RX;
	}
	uarts->uart_handle.Init.BaudRate = (uint32_t)baud_rate;
	if(uarts->cts == HW_ASSIGNMENT_ID_NULL &&
			uarts->rts == HW_ASSIGNMENT_ID_NULL) {
		uarts->uart_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	} else if(uarts->cts != HW_ASSIGNMENT_ID_NULL &&
			uarts->rts == HW_ASSIGNMENT_ID_NULL) {
		uarts->uart_handle.Init.HwFlowCtl = UART_HWCONTROL_CTS;
	} else if(uarts->cts == HW_ASSIGNMENT_ID_NULL &&
			uarts->rts != HW_ASSIGNMENT_ID_NULL) {
		uarts->uart_handle.Init.HwFlowCtl = UART_HWCONTROL_RTS;
	} else if(uarts->cts != HW_ASSIGNMENT_ID_NULL &&
			uarts->rts != HW_ASSIGNMENT_ID_NULL) {
		uarts->uart_handle.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
	}
	uarts->uart_handle.Init.WordLength = word_length;
	uarts->uart_handle.Init.StopBits = stop_length;
	uarts->uart_handle.Init.Parity = parity;
	uarts->uart_handle.Init.OverSampling = UART_OVERSAMPLING_8;
}

void hw_uart_configure_tx_buffer(hw_assignment_id_t id,
		hw_resource_id_t tx_dma_stream, uint8_t * tx_ring,
		size_t tx_ring_size) {
	hw_resource_id_t uart = hw_resource_get_resource_id(id);
	hw_usart_struct_t * uarts = (hw_usart_struct_t *)hw_resource_get_user(id);
	uint32_t tx_dma_channel = DMA_CHANNEL_0;

	(void)tx_ring;
	(void)tx_ring_size;

	cu_verify(uarts->tx_dma_handle == NULL);
	if(tx_dma_stream != HWR_NONE) {
		switch(uart) {
		case HWR_USART1:
			break;
		case HWR_USART2:
			break;
		case HWR_USART3:
			break;
		case HWR_UART4:
			break;
		case HWR_UART5:
			break;
		case HWR_USART6:
			break;
		default:
			cu_error("invalid uart resource");
			break;
		}

		uarts->tx_dma_stream = hw_resource_assign(tx_dma_stream, 0);
		uarts->tx_dma_handle = &hw_dma_streams[tx_dma_stream - HWR_DMA1_STREAM0];
		uarts->tx_dma_handle->Instance =
				hw_resource_definitions[tx_dma_stream].dma.stream;
		uarts->tx_dma_handle->Init.Channel = tx_dma_channel;
		uarts->tx_dma_handle->Init.Direction = DMA_MEMORY_TO_PERIPH;
		uarts->tx_dma_handle->Init.PeriphInc = DMA_PINC_DISABLE;
		uarts->tx_dma_handle->Init.MemInc = DMA_MINC_ENABLE;
		uarts->tx_dma_handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		uarts->tx_dma_handle->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		uarts->tx_dma_handle->Init.Mode = DMA_CIRCULAR;
		uarts->tx_dma_handle->Init.Priority = DMA_PRIORITY_HIGH;
		uarts->tx_dma_handle->Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		uarts->tx_dma_handle->Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
		uarts->tx_dma_handle->Init.MemBurst = DMA_PBURST_SINGLE;
		uarts->tx_dma_handle->Init.PeriphBurst = DMA_PBURST_SINGLE;
		uarts->tx_dma_handle->Parent = &uarts->uart_handle;
	} else {
	}
}

/*
void hw_uart_configure_rx_buffer(hw_assignment_id_t id, uint8_t * rx_ring,
		size_t rx_ring_size) {
	hw_usart_struct_t * uarts = (hw_usart_struct_t *)hw_resource_get_user(id);

	cu_verify(uarts->tx_dma_handle == NULL);
	cu_verify(tx_dma != HWR_NONE);


	if(rx_dma != HWR_NONE) {
		spis->rx_dma_stream = hw_resource_assign(rx_dma, 0);
		spis->rx_dma_handle = &hw_dma_streams[rx_dma - HWR_DMA1_STREAM0];
		spis->rx_dma_handle->Instance =
				hw_resource_definitions[rx_dma].dma.stream;
		spis->rx_dma_handle->Init.Channel = rx_dma_channel;
		spis->rx_dma_handle->Init.Direction = DMA_MEMORY_TO_PERIPH;
		spis->rx_dma_handle->Init.PeriphInc = DMA_PINC_DISABLE;
		spis->rx_dma_handle->Init.MemInc = DMA_MINC_ENABLE;
		spis->rx_dma_handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		spis->rx_dma_handle->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		spis->rx_dma_handle->Init.Mode = DMA_CIRCULAR;
		spis->rx_dma_handle->Init.Priority = DMA_PRIORITY_HIGH;
		spis->rx_dma_handle->Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		spis->rx_dma_handle->Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
		spis->rx_dma_handle->Init.MemBurst = DMA_PBURST_SINGLE;
		spis->rx_dma_handle->Init.PeriphBurst = DMA_PBURST_SINGLE;
		spis->rx_dma_handle->Parent = &spis->spi_handle;
	} else {
		spis->rx_dma_handle = NULL;
	}
}

void hw_uart_start(hw_assignment_id_t id) {
	hw_usart_struct_t * uarts = (hw_usart_struct_t *)hw_resource_get_user(id);

}

void hw_uart_transmit(hw_assignment_id_t id, uint8_t const * buf,
		size_t count) {
	hw_usart_struct_t * uarts = (hw_usart_struct_t *)hw_resource_get_user(id);

}

size_t hw_uart_transmit_nonblock(hw_assignment_id_t id, uint8_t const * buf,
		size_t count) {
	hw_usart_struct_t * uarts = (hw_usart_struct_t *)hw_resource_get_user(id);

}

void hw_uart_receive(hw_assignment_id_t id, uint8_t * buf, size_t count) {
	hw_usart_struct_t * uarts = (hw_usart_struct_t *)hw_resource_get_user(id);

}

size_t hw_uart_receive_nonblock(hw_assignment_id_t id, uint8_t * buf,
		size_t count) {
	hw_usart_struct_t * uarts = (hw_usart_struct_t *)hw_resource_get_user(id);

}

size_t hw_uart_get_receive_waiting(hw_assignment_id_t id) {
	hw_usart_struct_t * uarts = (hw_usart_struct_t *)hw_resource_get_user(id);

}

size_t hw_uart_get_transmit_space_left(hw_assignment_id_t id) {
	hw_usart_struct_t * uarts = (hw_usart_struct_t *)hw_resource_get_user(id);

}

size_t hw_uart_get_transmit_waiting(hw_assignment_id_t id) {
	hw_usart_struct_t * uarts = (hw_usart_struct_t *)hw_resource_get_user(id);

}
*/

