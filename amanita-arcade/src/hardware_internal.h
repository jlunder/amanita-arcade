/*
 * hardware_internal.h
 *
 *  Created on: Jun 4, 2016
 *      Author: jlunder
 */

#ifndef HARDWARE_INTERNAL_H_
#define HARDWARE_INTERNAL_H_

#include "hardware.h"

typedef struct {
	hw_resource_id_t resource_id;
	intptr_t user;
} hw_resource_assignment_t;

typedef struct {
	hw_assignment_id_t pwm_channels[4];
	TIM_HandleTypeDef tim_handle;
	uint32_t frequency;
} hw_timer_struct_t;

typedef struct {
	hw_assignment_id_t ws;
	hw_assignment_id_t sclk;
	hw_assignment_id_t sd;
	hw_assignment_id_t mclk;
	hw_assignment_id_t dma_stream;
	I2S_HandleTypeDef i2s_handle;
	DMA_HandleTypeDef * dma_handle;
	void * buf;
	size_t buf_len;
	hw_i2s_fill_func_t fill_func;
	bool running;
	uint8_t pad0[3];
} hw_i2s_struct_t;

typedef struct {
	hw_assignment_id_t scl;
	hw_assignment_id_t sda;
	I2C_HandleTypeDef i2c_handle;
} hw_i2c_struct_t;

typedef struct {
	hw_assignment_id_t sck;
	hw_assignment_id_t miso;
	hw_assignment_id_t mosi;
	hw_assignment_id_t tx_dma_stream;
	hw_assignment_id_t rx_dma_stream;
	SPI_HandleTypeDef spi_handle;
	DMA_HandleTypeDef * tx_dma_handle;
	DMA_HandleTypeDef * rx_dma_handle;
} hw_spi_struct_t;

typedef struct {
	hw_assignment_id_t tx;
	hw_assignment_id_t rx;
	hw_assignment_id_t cts;
	hw_assignment_id_t rts;
	hw_assignment_id_t ck;
	hw_assignment_id_t tx_dma_stream;
	hw_assignment_id_t rx_dma_stream;
	union {
		UART_HandleTypeDef uart_handle;
		USART_HandleTypeDef usart_handle;
	};
	DMA_HandleTypeDef * tx_dma_handle;
	DMA_HandleTypeDef * rx_dma_handle;
	bool started;
	uint8_t pad0, pad1, pad2;
} hw_usart_struct_t;

extern hw_resource_definition_t const hw_resource_definitions[HWR_COUNT];
extern DMA_HandleTypeDef hw_dma_streams[16];

#endif /* HARDWARE_INTERNAL_H_ */
