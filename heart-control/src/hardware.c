/*
 * hardware.c
 *
 *  Created on: Oct 13, 2015
 *      Author: jlunder
 */

#include "hardware.h"

typedef struct {
	hw_resource_id_t resource_id;
	intptr_t user;
} hw_resource_assignment_t;

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

static hw_resource_definition_t const hw_resource_definitions[HWR_COUNT] = {
		{.type = HWT_INVALID},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI0_IRQn,     .gpio = {.bank = GPIOA, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI1_IRQn,     .gpio = {.bank = GPIOA, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI2_IRQn,     .gpio = {.bank = GPIOA, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI3_IRQn,     .gpio = {.bank = GPIOA, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI4_IRQn,     .gpio = {.bank = GPIOA, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOA, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOA, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOA, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOA, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOA, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOA, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOA, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOA, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOA, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOA, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOA, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI0_IRQn,     .gpio = {.bank = GPIOB, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI1_IRQn,     .gpio = {.bank = GPIOB, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI2_IRQn,     .gpio = {.bank = GPIOB, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI3_IRQn,     .gpio = {.bank = GPIOB, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI4_IRQn,     .gpio = {.bank = GPIOB, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOB, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOB, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOB, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOB, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOB, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOB, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOB, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOB, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOB, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOB, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOB, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI0_IRQn,     .gpio = {.bank = GPIOC, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI1_IRQn,     .gpio = {.bank = GPIOC, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI2_IRQn,     .gpio = {.bank = GPIOC, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI3_IRQn,     .gpio = {.bank = GPIOC, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI4_IRQn,     .gpio = {.bank = GPIOC, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOC, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOC, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOC, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOC, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOC, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOC, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOC, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOC, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOC, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOC, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOC, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI0_IRQn,     .gpio = {.bank = GPIOD, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI1_IRQn,     .gpio = {.bank = GPIOD, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI2_IRQn,     .gpio = {.bank = GPIOD, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI3_IRQn,     .gpio = {.bank = GPIOD, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI4_IRQn,     .gpio = {.bank = GPIOD, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOD, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOD, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOD, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOD, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOD, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOD, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOD, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOD, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOD, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOD, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOD, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI0_IRQn,     .gpio = {.bank = GPIOE, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI1_IRQn,     .gpio = {.bank = GPIOE, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI2_IRQn,     .gpio = {.bank = GPIOE, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI3_IRQn,     .gpio = {.bank = GPIOE, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI4_IRQn,     .gpio = {.bank = GPIOE, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOE, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOE, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOE, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOE, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOE, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOE, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOE, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOE, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOE, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOE, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOE, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI0_IRQn,     .gpio = {.bank = GPIOF, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI1_IRQn,     .gpio = {.bank = GPIOF, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI2_IRQn,     .gpio = {.bank = GPIOF, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI3_IRQn,     .gpio = {.bank = GPIOF, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI4_IRQn,     .gpio = {.bank = GPIOF, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOF, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOF, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOF, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOF, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOF, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOF, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOF, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOF, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOF, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOF, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOF, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI0_IRQn,     .gpio = {.bank = GPIOG, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI1_IRQn,     .gpio = {.bank = GPIOG, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI2_IRQn,     .gpio = {.bank = GPIOG, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI3_IRQn,     .gpio = {.bank = GPIOG, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI4_IRQn,     .gpio = {.bank = GPIOG, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOG, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOG, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOG, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOG, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOG, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOG, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOG, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOG, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOG, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOG, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOG, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI0_IRQn,     .gpio = {.bank = GPIOH, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI1_IRQn,     .gpio = {.bank = GPIOH, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI2_IRQn,     .gpio = {.bank = GPIOH, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI3_IRQn,     .gpio = {.bank = GPIOH, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI4_IRQn,     .gpio = {.bank = GPIOH, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOH, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOH, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOH, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOH, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOH, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOH, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOH, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOH, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOH, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOH, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOH, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .irq_id = EXTI0_IRQn,     .gpio = {.bank = GPIOI, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .irq_id = EXTI1_IRQn,     .gpio = {.bank = GPIOI, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .irq_id = EXTI2_IRQn,     .gpio = {.bank = GPIOI, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .irq_id = EXTI3_IRQn,     .gpio = {.bank = GPIOI, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .irq_id = EXTI4_IRQn,     .gpio = {.bank = GPIOI, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOI, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOI, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOI, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOI, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .irq_id = EXTI9_5_IRQn,   .gpio = {.bank = GPIOI, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOI, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .irq_id = EXTI15_10_IRQn, .gpio = {.bank = GPIOI, .index = 11}},
		{.type = HWT_TIM, .clock_id = HWC_TIM1, .irq_id = HWI_NONE, .tim = TIM1},
		{.type = HWT_TIM, .clock_id = HWC_TIM2, .irq_id = HWI_NONE, .tim = TIM2},
		{.type = HWT_TIM, .clock_id = HWC_TIM3, .irq_id = HWI_NONE, .tim = TIM3},
		{.type = HWT_TIM, .clock_id = HWC_TIM4, .irq_id = HWI_NONE, .tim = TIM4},
		{.type = HWT_TIM, .clock_id = HWC_TIM5, .irq_id = HWI_NONE, .tim = TIM5},
		{.type = HWT_TIM, .clock_id = HWC_TIM6, .irq_id = HWI_NONE, .tim = TIM6},
		{.type = HWT_TIM, .clock_id = HWC_TIM7, .irq_id = HWI_NONE, .tim = TIM7},
		{.type = HWT_TIM, .clock_id = HWC_TIM8, .irq_id = HWI_NONE, .tim = TIM8},
		{.type = HWT_TIM, .clock_id = HWC_TIM9, .irq_id = HWI_NONE, .tim = TIM9},
		{.type = HWT_TIM, .clock_id = HWC_TIM10, .irq_id = HWI_NONE, .tim = TIM10},
		{.type = HWT_TIM, .clock_id = HWC_TIM11, .irq_id = HWI_NONE, .tim = TIM11},
		{.type = HWT_TIM, .clock_id = HWC_TIM12, .irq_id = HWI_NONE, .tim = TIM12},
		{.type = HWT_TIM, .clock_id = HWC_TIM13, .irq_id = HWI_NONE, .tim = TIM13},
		{.type = HWT_TIM, .clock_id = HWC_TIM14, .irq_id = HWI_NONE, .tim = TIM14},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .irq_id = HWI_DMA1_STREAM0, .dma = {.controller = DMA1, .stream = DMA1_Stream0}},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .irq_id = HWI_DMA1_STREAM1, .dma = {.controller = DMA1, .stream = DMA1_Stream1}},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .irq_id = HWI_DMA1_STREAM2, .dma = {.controller = DMA1, .stream = DMA1_Stream2}},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .irq_id = HWI_DMA1_STREAM3, .dma = {.controller = DMA1, .stream = DMA1_Stream3}},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .irq_id = HWI_DMA1_STREAM4, .dma = {.controller = DMA1, .stream = DMA1_Stream4}},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .irq_id = HWI_DMA1_STREAM5, .dma = {.controller = DMA1, .stream = DMA1_Stream5}},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .irq_id = HWI_DMA1_STREAM6, .dma = {.controller = DMA1, .stream = DMA1_Stream6}},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .irq_id = HWI_DMA1_STREAM7, .dma = {.controller = DMA1, .stream = DMA1_Stream7}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .irq_id = HWI_DMA2_STREAM0, .dma = {.controller = DMA2, .stream = DMA2_Stream0}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .irq_id = HWI_DMA2_STREAM1, .dma = {.controller = DMA2, .stream = DMA2_Stream1}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .irq_id = HWI_DMA2_STREAM2, .dma = {.controller = DMA2, .stream = DMA2_Stream2}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .irq_id = HWI_DMA2_STREAM3, .dma = {.controller = DMA2, .stream = DMA2_Stream3}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .irq_id = HWI_DMA2_STREAM4, .dma = {.controller = DMA2, .stream = DMA2_Stream4}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .irq_id = HWI_DMA2_STREAM5, .dma = {.controller = DMA2, .stream = DMA2_Stream5}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .irq_id = HWI_DMA2_STREAM6, .dma = {.controller = DMA2, .stream = DMA2_Stream6}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .irq_id = HWI_DMA2_STREAM7, .dma = {.controller = DMA2, .stream = DMA2_Stream7}},
		{.type = HWT_ADC, .clock_id = HWC_ADC1, .irq_id = HWI_NONE, .adc = ADC1},
		{.type = HWT_ADC, .clock_id = HWC_ADC2, .irq_id = HWI_NONE, .adc = ADC2},
		{.type = HWT_ADC, .clock_id = HWC_ADC3, .irq_id = HWI_NONE, .adc = ADC3},
		{.type = HWT_DAC, .clock_id = HWC_DAC, .irq_id = HWI_NONE, .dac = DAC},
		{.type = HWT_SPI, .clock_id = HWC_SPI1, .irq_id = HWI_SPI1, .spi = SPI1},
		{.type = HWT_SPII2S, .clock_id = HWC_SPII2S2, .irq_id = HWI_SPII2S2, .spi = SPI2},
		{.type = HWT_SPII2S, .clock_id = HWC_SPII2S3, .irq_id = HWI_SPII2S3, .spi = SPI3},
		{.type = HWT_I2C, .clock_id = HWC_I2C1, .irq_id = HWI_I2C1, .i2c = I2C1},
		{.type = HWT_I2C, .clock_id = HWC_I2C2, .irq_id = HWI_I2C2, .i2c = I2C2},
		{.type = HWT_I2C, .clock_id = HWC_I2C3, .irq_id = HWI_I2C3, .i2c = I2C3},
		{.type = HWT_CAN, .clock_id = HWC_CAN1, .irq_id = HWI_NONE, .can = CAN1},
		{.type = HWT_CAN, .clock_id = HWC_CAN2, .irq_id = HWI_NONE, .can = CAN2},
		{.type = HWT_USART, .clock_id = HWC_USART1, .irq_id = HWI_NONE, .usart = USART1},
		{.type = HWT_USART, .clock_id = HWC_USART2, .irq_id = HWI_NONE, .usart = USART2},
		{.type = HWT_USART, .clock_id = HWC_USART3, .irq_id = HWI_NONE, .usart = USART3},
		{.type = HWT_UART, .clock_id = HWC_UART4, .irq_id = HWI_NONE, .usart = UART4},
		{.type = HWT_UART, .clock_id = HWC_UART5, .irq_id = HWI_NONE, .usart = UART5},
		{.type = HWT_USART, .clock_id = HWC_USART6, .irq_id = HWI_NONE, .usart = USART6},
};

static size_t hw_clock_enable_counts[HWC_COUNT]; // all init 0
static size_t hw_clock_i2s_pll_enable_count = 0;

static size_t hw_irq_enable_counts[HWI_COUNT]; // all init 0

static hw_assignment_id_t hw_assignment_id_in_pool_count = 0;
static hw_assignment_id_t hw_assignment_id_total_count = 0;
static hw_assignment_id_t hw_assignment_id_next_id = HW_ASSIGNMENT_START;
static hw_assignment_id_t hw_assignment_id_pool[HW_ASSIGNMENT_MAX];

static hw_assignment_id_t hw_resource_assignments[HWR_COUNT];
static hw_resource_assignment_t hw_assignment_resources[HW_ASSIGNMENT_MAX];

static hw_i2s_struct_t hw_i2s_2;
static hw_i2s_struct_t hw_i2s_3;

static hw_i2c_struct_t hw_i2c_1;
static hw_i2c_struct_t hw_i2c_2;
static hw_i2c_struct_t hw_i2c_3;

static hw_spi_struct_t hw_spi_1;
static hw_spi_struct_t hw_spi_2;
static hw_spi_struct_t hw_spi_3;

static DMA_HandleTypeDef hw_dma_streams[16];

hw_assignment_id_t hw_assignment_alloc(void) {
	if (hw_assignment_id_in_pool_count > 0) {
		return hw_assignment_id_pool[--hw_assignment_id_in_pool_count];
	} else {
		cu_verify(hw_assignment_id_total_count < HW_ASSIGNMENT_MAX);
		++hw_assignment_id_total_count;
		return hw_assignment_id_next_id++;
	}
}

void hw_assignment_free(hw_assignment_id_t id) {
	cu_verify(hw_assignment_id_in_pool_count < HW_ASSIGNMENT_MAX);
	hw_assignment_id_pool[hw_assignment_id_in_pool_count++] = id;
}

hw_resource_definition_t const * hw_get_resource_definition(
		hw_resource_id_t id) {
	cu_verify(id > HWR_NONE);
	cu_verify(id < HWR_COUNT);
	return &hw_resource_definitions[id];
}

hw_assignment_id_t hw_resource_assign(hw_resource_id_t resource_id,
		intptr_t user) {
	hw_assignment_id_t id;

	cu_verify(resource_id > HWR_NONE);
	cu_verify(resource_id < HWR_COUNT);
	cu_verify(hw_resource_assignments[resource_id] ==
			HW_ASSIGNMENT_ID_NULL);

	id = hw_assignment_alloc();
	hw_resource_assignments[resource_id] = id;
	hw_assignment_resources[id - HW_ASSIGNMENT_START].resource_id =
			resource_id;
	hw_assignment_resources[id - HW_ASSIGNMENT_START].user = user;

	cu_verify(id - HW_ASSIGNMENT_START < HW_ASSIGNMENT_MAX);

	return id;
}

void hw_resource_deassign(hw_assignment_id_t assignment_id) {
	size_t i = assignment_id - HW_ASSIGNMENT_START;
	hw_resource_id_t resource_id;

	cu_verify(assignment_id >= HW_ASSIGNMENT_START);
	cu_verify(i < HW_ASSIGNMENT_MAX);
	resource_id = hw_assignment_resources[i].resource_id;
	cu_verify(resource_id != HWR_NONE && resource_id < HWR_COUNT);
	cu_verify(hw_resource_assignments[resource_id] == assignment_id);

	hw_resource_assignments[resource_id] = HW_ASSIGNMENT_ID_NULL;
	hw_assignment_resources[i].resource_id = HWR_INVALID;
	hw_assignment_resources[i].user = 0;
	hw_assignment_free(assignment_id);
}

hw_resource_id_t hw_resource_get_resource_id(hw_assignment_id_t assignment_id) {
	size_t i = assignment_id - HW_ASSIGNMENT_START;
	hw_resource_id_t resource_id = hw_assignment_resources[i].resource_id;
	cu_verify(i < HW_ASSIGNMENT_MAX);
	cu_verify(resource_id > HWR_NONE && resource_id < HWR_COUNT);
	cu_verify(hw_resource_assignments[resource_id] == assignment_id);
	return resource_id;
}

intptr_t hw_resource_get_user(hw_assignment_id_t assignment_id) {
	size_t i = assignment_id - HW_ASSIGNMENT_START;
	hw_resource_id_t resource_id = hw_assignment_resources[i].resource_id;
	cu_verify(i < HW_ASSIGNMENT_MAX);
	cu_verify(resource_id > HWR_NONE && resource_id < HWR_COUNT);
	cu_verify(hw_resource_assignments[resource_id] == assignment_id);
	return hw_assignment_resources[i].user;
}

void hw_clock_enable(hw_clock_id_t clock_id) {
	cu_verify(clock_id > HWC_NONE && clock_id < HWC_COUNT);
	if(hw_clock_enable_counts[clock_id] == 0) {
		switch(clock_id) {
		case HWC_GPIOA: __HAL_RCC_GPIOA_CLK_ENABLE(); break;
		case HWC_GPIOB: __HAL_RCC_GPIOB_CLK_ENABLE(); break;
		case HWC_GPIOC: __HAL_RCC_GPIOC_CLK_ENABLE(); break;
		case HWC_GPIOD: __HAL_RCC_GPIOD_CLK_ENABLE(); break;
		case HWC_GPIOE: __HAL_RCC_GPIOE_CLK_ENABLE(); break;
		case HWC_GPIOF: __HAL_RCC_GPIOF_CLK_ENABLE(); break;
		case HWC_GPIOG: __HAL_RCC_GPIOG_CLK_ENABLE(); break;
		case HWC_GPIOH: __HAL_RCC_GPIOH_CLK_ENABLE(); break;
		case HWC_GPIOI: __HAL_RCC_GPIOI_CLK_ENABLE(); break;
		case HWC_TIM1: __HAL_RCC_TIM1_CLK_ENABLE(); break;
		case HWC_TIM2: __HAL_RCC_TIM2_CLK_ENABLE(); break;
		case HWC_TIM3: __HAL_RCC_TIM3_CLK_ENABLE(); break;
		case HWC_TIM4: __HAL_RCC_TIM4_CLK_ENABLE(); break;
		case HWC_TIM5: __HAL_RCC_TIM5_CLK_ENABLE(); break;
		case HWC_TIM6: __HAL_RCC_TIM6_CLK_ENABLE(); break;
		case HWC_TIM7: __HAL_RCC_TIM7_CLK_ENABLE(); break;
		case HWC_TIM8: __HAL_RCC_TIM8_CLK_ENABLE(); break;
		case HWC_TIM9: __HAL_RCC_TIM9_CLK_ENABLE(); break;
		case HWC_TIM10: __HAL_RCC_TIM10_CLK_ENABLE(); break;
		case HWC_TIM11: __HAL_RCC_TIM11_CLK_ENABLE(); break;
		case HWC_TIM12: __HAL_RCC_TIM12_CLK_ENABLE(); break;
		case HWC_TIM13: __HAL_RCC_TIM13_CLK_ENABLE(); break;
		case HWC_TIM14: __HAL_RCC_TIM14_CLK_ENABLE(); break;
		case HWC_DMA1: __HAL_RCC_DMA1_CLK_ENABLE(); break;
		case HWC_DMA2: __HAL_RCC_DMA2_CLK_ENABLE(); break;
		case HWC_ADC1: __HAL_RCC_ADC1_CLK_ENABLE(); break;
		case HWC_ADC2: __HAL_RCC_ADC2_CLK_ENABLE(); break;
		case HWC_ADC3: __HAL_RCC_ADC3_CLK_ENABLE(); break;
		case HWC_DAC: __HAL_RCC_DAC_CLK_ENABLE(); break;
		case HWC_SPI1: __HAL_RCC_SPI1_CLK_ENABLE(); break;
		case HWC_SPII2S2: __HAL_RCC_SPI2_CLK_ENABLE(); break;
		case HWC_SPII2S3: __HAL_RCC_SPI3_CLK_ENABLE(); break;
		case HWC_I2C1: __HAL_RCC_I2C1_CLK_ENABLE(); break;
		case HWC_I2C2: __HAL_RCC_I2C2_CLK_ENABLE(); break;
		case HWC_I2C3: __HAL_RCC_I2C3_CLK_ENABLE(); break;
		case HWC_CAN1: __HAL_RCC_CAN1_CLK_ENABLE(); break;
		case HWC_CAN2: __HAL_RCC_CAN2_CLK_ENABLE(); break;
		case HWC_USART1: __HAL_RCC_USART1_CLK_ENABLE(); break;
		case HWC_USART2: __HAL_RCC_USART2_CLK_ENABLE(); break;
		case HWC_USART3: __HAL_RCC_USART3_CLK_ENABLE(); break;
		case HWC_UART4: __HAL_RCC_UART4_CLK_ENABLE(); break;
		case HWC_UART5: __HAL_RCC_UART5_CLK_ENABLE(); break;
		case HWC_USART6: __HAL_RCC_USART6_CLK_ENABLE(); break;
		default: break;
		}
	}
	hw_clock_enable_counts[clock_id]++;
}

void hw_clock_disable(hw_clock_id_t clock_id) {
	cu_verify(clock_id > HWC_NONE && clock_id < HWC_COUNT);
	cu_verify(hw_clock_enable_counts[clock_id] > 0);
	--hw_clock_enable_counts[clock_id];
	if(hw_clock_enable_counts[clock_id] == 0) {
		switch(clock_id) {
		case HWC_GPIOA: __HAL_RCC_GPIOA_CLK_DISABLE(); break;
		case HWC_GPIOB: __HAL_RCC_GPIOB_CLK_DISABLE(); break;
		case HWC_GPIOC: __HAL_RCC_GPIOC_CLK_DISABLE(); break;
		case HWC_GPIOD: __HAL_RCC_GPIOD_CLK_DISABLE(); break;
		case HWC_GPIOE: __HAL_RCC_GPIOE_CLK_DISABLE(); break;
		case HWC_GPIOF: __HAL_RCC_GPIOF_CLK_DISABLE(); break;
		case HWC_GPIOG: __HAL_RCC_GPIOG_CLK_DISABLE(); break;
		case HWC_GPIOH: __HAL_RCC_GPIOH_CLK_DISABLE(); break;
		case HWC_GPIOI: __HAL_RCC_GPIOI_CLK_DISABLE(); break;
		case HWC_TIM1: __HAL_RCC_TIM1_CLK_DISABLE(); break;
		case HWC_TIM2: __HAL_RCC_TIM2_CLK_DISABLE(); break;
		case HWC_TIM3: __HAL_RCC_TIM3_CLK_DISABLE(); break;
		case HWC_TIM4: __HAL_RCC_TIM4_CLK_DISABLE(); break;
		case HWC_TIM5: __HAL_RCC_TIM5_CLK_DISABLE(); break;
		case HWC_TIM6: __HAL_RCC_TIM6_CLK_DISABLE(); break;
		case HWC_TIM7: __HAL_RCC_TIM7_CLK_DISABLE(); break;
		case HWC_TIM8: __HAL_RCC_TIM8_CLK_DISABLE(); break;
		case HWC_TIM9: __HAL_RCC_TIM9_CLK_DISABLE(); break;
		case HWC_TIM10: __HAL_RCC_TIM10_CLK_DISABLE(); break;
		case HWC_TIM11: __HAL_RCC_TIM11_CLK_DISABLE(); break;
		case HWC_TIM12: __HAL_RCC_TIM12_CLK_DISABLE(); break;
		case HWC_TIM13: __HAL_RCC_TIM13_CLK_DISABLE(); break;
		case HWC_TIM14: __HAL_RCC_TIM14_CLK_DISABLE(); break;
		case HWC_DMA1: __HAL_RCC_DMA1_CLK_DISABLE(); break;
		case HWC_DMA2: __HAL_RCC_DMA2_CLK_DISABLE(); break;
		case HWC_ADC1: __HAL_RCC_ADC1_CLK_DISABLE(); break;
		case HWC_ADC2: __HAL_RCC_ADC2_CLK_DISABLE(); break;
		case HWC_ADC3: __HAL_RCC_ADC3_CLK_DISABLE(); break;
		case HWC_DAC: __HAL_RCC_DAC_CLK_DISABLE(); break;
		case HWC_SPI1: __HAL_RCC_SPI1_CLK_DISABLE(); break;
		case HWC_SPII2S2: __HAL_RCC_SPI2_CLK_DISABLE(); break;
		case HWC_SPII2S3: __HAL_RCC_SPI3_CLK_DISABLE(); break;
		case HWC_I2C1: __HAL_RCC_I2C1_CLK_DISABLE(); break;
		case HWC_I2C2: __HAL_RCC_I2C2_CLK_DISABLE(); break;
		case HWC_I2C3: __HAL_RCC_I2C3_CLK_DISABLE(); break;
		case HWC_CAN1: __HAL_RCC_CAN1_CLK_DISABLE(); break;
		case HWC_CAN2: __HAL_RCC_CAN2_CLK_DISABLE(); break;
		case HWC_USART1: __HAL_RCC_USART1_CLK_DISABLE(); break;
		case HWC_USART2: __HAL_RCC_USART2_CLK_DISABLE(); break;
		case HWC_USART3: __HAL_RCC_USART3_CLK_DISABLE(); break;
		case HWC_UART4: __HAL_RCC_UART4_CLK_DISABLE(); break;
		case HWC_UART5: __HAL_RCC_UART5_CLK_DISABLE(); break;
		case HWC_USART6: __HAL_RCC_USART6_CLK_DISABLE(); break;
		default: break;
		}
	}
}

void hw_clock_force_reset(hw_clock_id_t clock_id) {
	cu_verify(clock_id > HWC_NONE && clock_id < HWC_COUNT);
	cu_verify(hw_clock_enable_counts[clock_id] == 0);
	switch(clock_id) {
	case HWC_GPIOA: __HAL_RCC_GPIOA_FORCE_RESET(); break;
	case HWC_GPIOB: __HAL_RCC_GPIOB_FORCE_RESET(); break;
	case HWC_GPIOC: __HAL_RCC_GPIOC_FORCE_RESET(); break;
	case HWC_GPIOD: __HAL_RCC_GPIOD_FORCE_RESET(); break;
	case HWC_GPIOE: __HAL_RCC_GPIOE_FORCE_RESET(); break;
	case HWC_GPIOF: __HAL_RCC_GPIOF_FORCE_RESET(); break;
	case HWC_GPIOG: __HAL_RCC_GPIOG_FORCE_RESET(); break;
	case HWC_GPIOH: __HAL_RCC_GPIOH_FORCE_RESET(); break;
	case HWC_GPIOI: __HAL_RCC_GPIOI_FORCE_RESET(); break;
	case HWC_TIM1: __HAL_RCC_TIM1_FORCE_RESET(); break;
	case HWC_TIM2: __HAL_RCC_TIM2_FORCE_RESET(); break;
	case HWC_TIM3: __HAL_RCC_TIM3_FORCE_RESET(); break;
	case HWC_TIM4: __HAL_RCC_TIM4_FORCE_RESET(); break;
	case HWC_TIM5: __HAL_RCC_TIM5_FORCE_RESET(); break;
	case HWC_TIM6: __HAL_RCC_TIM6_FORCE_RESET(); break;
	case HWC_TIM7: __HAL_RCC_TIM7_FORCE_RESET(); break;
	case HWC_TIM8: __HAL_RCC_TIM8_FORCE_RESET(); break;
	case HWC_TIM9: __HAL_RCC_TIM9_FORCE_RESET(); break;
	case HWC_TIM10: __HAL_RCC_TIM10_FORCE_RESET(); break;
	case HWC_TIM11: __HAL_RCC_TIM11_FORCE_RESET(); break;
	case HWC_TIM12: __HAL_RCC_TIM12_FORCE_RESET(); break;
	case HWC_TIM13: __HAL_RCC_TIM13_FORCE_RESET(); break;
	case HWC_TIM14: __HAL_RCC_TIM14_FORCE_RESET(); break;
	case HWC_DMA1: __HAL_RCC_DMA1_FORCE_RESET(); break;
	case HWC_DMA2: __HAL_RCC_DMA2_FORCE_RESET(); break;
	case HWC_ADC1: cu_error("Can't force reset ADC1"); break;
	case HWC_ADC2: cu_error("Can't force reset ADC2"); break;
	case HWC_ADC3: cu_error("Can't force reset ADC3"); break;
	case HWC_DAC: __HAL_RCC_DAC_FORCE_RESET(); break;
	case HWC_SPI1: __HAL_RCC_SPI1_FORCE_RESET(); break;
	case HWC_SPII2S2: __HAL_RCC_SPI2_FORCE_RESET(); break;
	case HWC_SPII2S3: __HAL_RCC_SPI3_FORCE_RESET(); break;
	case HWC_I2C1: __HAL_RCC_I2C1_FORCE_RESET(); break;
	case HWC_I2C2: __HAL_RCC_I2C2_FORCE_RESET(); break;
	case HWC_I2C3: __HAL_RCC_I2C3_FORCE_RESET(); break;
	case HWC_CAN1: __HAL_RCC_CAN1_FORCE_RESET(); break;
	case HWC_CAN2: __HAL_RCC_CAN2_FORCE_RESET(); break;
	case HWC_USART1: __HAL_RCC_USART1_FORCE_RESET(); break;
	case HWC_USART2: __HAL_RCC_USART2_FORCE_RESET(); break;
	case HWC_USART3: __HAL_RCC_USART3_FORCE_RESET(); break;
	case HWC_UART4: __HAL_RCC_UART4_FORCE_RESET(); break;
	case HWC_UART5: __HAL_RCC_UART5_FORCE_RESET(); break;
	case HWC_USART6: __HAL_RCC_USART6_FORCE_RESET(); break;
	default: break;
	}
}

void hw_irq_enable(hw_irq_id_t irq_id) {
	cu_verify(irq_id > HWI_NONE && irq_id < HWI_COUNT);
	if(hw_irq_enable_counts[irq_id] == 0) {
		switch(irq_id) {
	    case HWI_EXTI0: HAL_NVIC_EnableIRQ(EXTI0_IRQn); break;
	    case HWI_EXTI1: HAL_NVIC_EnableIRQ(EXTI0_IRQn); break;
	    case HWI_EXTI2: HAL_NVIC_EnableIRQ(EXTI0_IRQn); break;
	    case HWI_EXTI3: HAL_NVIC_EnableIRQ(EXTI0_IRQn); break;
	    case HWI_EXTI4: HAL_NVIC_EnableIRQ(EXTI0_IRQn); break;
	    case HWI_EXTI9_5: HAL_NVIC_EnableIRQ(EXTI9_5_IRQn); break;
	    case HWI_EXTI15_10: HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); break;
	    case HWI_DMA1_STREAM0: HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn); break;
	    case HWI_DMA1_STREAM1: HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn); break;
	    case HWI_DMA1_STREAM2: HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn); break;
	    case HWI_DMA1_STREAM3: HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn); break;
	    case HWI_DMA1_STREAM4: HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn); break;
	    case HWI_DMA1_STREAM5: HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn); break;
	    case HWI_DMA1_STREAM6: HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn); break;
	    case HWI_DMA1_STREAM7: HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn); break;
	    case HWI_DMA2_STREAM0: HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn); break;
	    case HWI_DMA2_STREAM1: HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn); break;
	    case HWI_DMA2_STREAM2: HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn); break;
	    case HWI_DMA2_STREAM3: HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn); break;
	    case HWI_DMA2_STREAM4: HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn); break;
	    case HWI_DMA2_STREAM5: HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn); break;
	    case HWI_DMA2_STREAM6: HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn); break;
	    case HWI_DMA2_STREAM7: HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn); break;
	    case HWI_I2C1:
	        HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
	        HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
	        break;
	    case HWI_I2C2:
	        HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
	        HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
	        break;
	    case HWI_I2C3:
	        HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);
	        HAL_NVIC_EnableIRQ(I2C3_ER_IRQn);
	        break;
	    case HWI_SPI1: HAL_NVIC_EnableIRQ(SPI1_IRQn); break;
	    case HWI_SPII2S2: HAL_NVIC_EnableIRQ(SPI2_IRQn); break;
	    case HWI_SPII2S3: HAL_NVIC_EnableIRQ(SPI3_IRQn); break;
		default: break;
		}
	}
	hw_irq_enable_counts[irq_id]++;
}

void hw_irq_disable(hw_irq_id_t irq_id) {
	cu_verify(irq_id > HWI_NONE && irq_id < HWI_COUNT);
	cu_verify(hw_irq_enable_counts[irq_id] > 0);
	--hw_irq_enable_counts[irq_id];
	if(hw_irq_enable_counts[irq_id] == 0) {
		switch(irq_id) {
	    case HWI_EXTI0: HAL_NVIC_DisableIRQ(EXTI0_IRQn); break;
	    case HWI_EXTI1: HAL_NVIC_DisableIRQ(EXTI0_IRQn); break;
	    case HWI_EXTI2: HAL_NVIC_DisableIRQ(EXTI0_IRQn); break;
	    case HWI_EXTI3: HAL_NVIC_DisableIRQ(EXTI0_IRQn); break;
	    case HWI_EXTI4: HAL_NVIC_DisableIRQ(EXTI0_IRQn); break;
	    case HWI_EXTI9_5: HAL_NVIC_DisableIRQ(EXTI9_5_IRQn); break;
	    case HWI_EXTI15_10: HAL_NVIC_DisableIRQ(EXTI15_10_IRQn); break;
	    case HWI_DMA1_STREAM0: HAL_NVIC_DisableIRQ(DMA1_Stream0_IRQn); break;
	    case HWI_DMA1_STREAM1: HAL_NVIC_DisableIRQ(DMA1_Stream1_IRQn); break;
	    case HWI_DMA1_STREAM2: HAL_NVIC_DisableIRQ(DMA1_Stream2_IRQn); break;
	    case HWI_DMA1_STREAM3: HAL_NVIC_DisableIRQ(DMA1_Stream3_IRQn); break;
	    case HWI_DMA1_STREAM4: HAL_NVIC_DisableIRQ(DMA1_Stream4_IRQn); break;
	    case HWI_DMA1_STREAM5: HAL_NVIC_DisableIRQ(DMA1_Stream5_IRQn); break;
	    case HWI_DMA1_STREAM6: HAL_NVIC_DisableIRQ(DMA1_Stream6_IRQn); break;
	    case HWI_DMA1_STREAM7: HAL_NVIC_DisableIRQ(DMA1_Stream7_IRQn); break;
	    case HWI_DMA2_STREAM0: HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn); break;
	    case HWI_DMA2_STREAM1: HAL_NVIC_DisableIRQ(DMA2_Stream1_IRQn); break;
	    case HWI_DMA2_STREAM2: HAL_NVIC_DisableIRQ(DMA2_Stream2_IRQn); break;
	    case HWI_DMA2_STREAM3: HAL_NVIC_DisableIRQ(DMA2_Stream3_IRQn); break;
	    case HWI_DMA2_STREAM4: HAL_NVIC_DisableIRQ(DMA2_Stream4_IRQn); break;
	    case HWI_DMA2_STREAM5: HAL_NVIC_DisableIRQ(DMA2_Stream5_IRQn); break;
	    case HWI_DMA2_STREAM6: HAL_NVIC_DisableIRQ(DMA2_Stream6_IRQn); break;
	    case HWI_DMA2_STREAM7: HAL_NVIC_DisableIRQ(DMA2_Stream7_IRQn); break;
	    case HWI_I2C1:
	    	HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
	    	HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
	        break;
	    case HWI_I2C2:
	    	HAL_NVIC_DisableIRQ(I2C2_EV_IRQn);
	    	HAL_NVIC_DisableIRQ(I2C2_ER_IRQn);
	        break;
	    case HWI_I2C3:
	    	HAL_NVIC_DisableIRQ(I2C3_EV_IRQn);
	    	HAL_NVIC_DisableIRQ(I2C3_ER_IRQn);
	        break;
	    case HWI_SPI1: HAL_NVIC_DisableIRQ(SPI1_IRQn); break;
	    case HWI_SPII2S2: HAL_NVIC_DisableIRQ(SPI2_IRQn); break;
	    case HWI_SPII2S3: HAL_NVIC_DisableIRQ(SPI3_IRQn); break;
		default: break;
		}
	}
}

hw_assignment_id_t hw_pin_assign(hw_resource_id_t pin_id) {
	hw_assignment_id_t id;

	cu_verify(pin_id > HWR_NONE && pin_id <= cu_lengthof(hw_resource_definitions));
	cu_verify(hw_resource_definitions[pin_id].type == HWT_GPIO);
	id = hw_resource_assign(pin_id, 0);
	hw_clock_enable(hw_resource_definitions[pin_id].clock_id);
	return id;
}

void hw_pin_deassign(hw_assignment_id_t assignment_id) {
	hw_resource_id_t pin_id = hw_resource_get_resource_id(assignment_id);
	hw_resource_definition_t const * def = &hw_resource_definitions[pin_id];

	hw_clock_disable(def->clock_id);
	hw_assignment_free(assignment_id);
}

void hw_pin_configure(hw_assignment_id_t assignment_id, hw_pin_mode_t mode) {
	hw_resource_id_t pin_id = hw_resource_get_resource_id(assignment_id);
	hw_resource_definition_t const * def = &hw_resource_definitions[pin_id];
	GPIO_InitTypeDef init;

	init.Pin = 1U << def->gpio.index;

	init.Alternate = 0;

	switch(mode & HWPM_MODE_MASK) {
	default:
	case HWPM_MODE_INPUT:
		init.Mode = GPIO_MODE_INPUT;
		break;
	case HWPM_MODE_OUTPUT:
		if((mode & HWPM_DRIVE_MASK) == HWPM_DRIVE_PUSH_PULL) {
			init.Mode = GPIO_MODE_OUTPUT_PP;
		} else {
			init.Mode = GPIO_MODE_OUTPUT_OD;
		}
		break;
	case HWPM_MODE_ALTERNATE_FUNCTION:
		if((mode & HWPM_DRIVE_MASK) == HWPM_DRIVE_PUSH_PULL) {
			init.Mode = GPIO_MODE_AF_PP;
		} else {
			init.Mode = GPIO_MODE_AF_OD;
		}
		init.Alternate = mode & HWPM_ALT_MASK;
		break;
	case HWPM_MODE_ANALOG:
		init.Mode = GPIO_MODE_ANALOG;
		break;
	}

	switch(mode & HWPM_PULL_MASK) {
	default:
	case HWPM_PULL_NONE:
		init.Pull = GPIO_NOPULL;
		break;
	case HWPM_PULL_UP:
		init.Pull = GPIO_PULLUP;
		break;
	case HWPM_PULL_DOWN:
		init.Pull = GPIO_PULLDOWN;
		break;
	}

	switch(mode & HWPM_SPEED_MASK) {
	default:
	case HWPM_SPEED_MIN:
		init.Speed = GPIO_SPEED_LOW;
		break;
	case HWPM_SPEED_FAST1:
		init.Speed = GPIO_SPEED_MEDIUM;
		break;
	case HWPM_SPEED_FAST2:
		init.Speed = GPIO_SPEED_FAST;
		break;
	case HWPM_SPEED_MAX:
		init.Speed = GPIO_SPEED_HIGH;
		break;
	}

	HAL_GPIO_Init(def->gpio.bank, &init);
}

hw_assignment_id_t hw_i2s_assign(hw_resource_id_t i2s,
		hw_resource_id_t sclk_pin, hw_resource_id_t ws_pin,
		hw_resource_id_t sd_pin, hw_resource_id_t mclk_pin,
		hw_resource_id_t dma_stream) {
	hw_assignment_id_t id;
	hw_i2s_struct_t * i2ss = NULL;
	uint32_t dma_channel = DMA_CHANNEL_0;

	cu_verify(i2s > HWR_NONE && i2s < HWR_COUNT);
	cu_verify(hw_resource_definitions[i2s].type == HWT_SPII2S);
	cu_verify(dma_stream > HWR_NONE && dma_stream < HWR_COUNT);
	cu_verify(hw_resource_definitions[dma_stream].type == HWT_DMA);

	switch(i2s) {
	case HWR_SPII2S2:
		cu_verify(sclk_pin == HWR_PB10 || sclk_pin == HWR_PB13 ||
				sclk_pin == HWR_PI1);
		cu_verify(ws_pin == HWR_PB9 || ws_pin == HWR_PB12 ||
				ws_pin == HWR_PI0);
		cu_verify(sd_pin == HWR_PB15 || sd_pin == HWR_PC3 ||
				sd_pin == HWR_PI3);
		cu_verify(mclk_pin == HWR_NONE || mclk_pin == HWR_PC6);
		switch(dma_stream) {
		case HWR_DMA1_STREAM4: dma_channel = DMA_CHANNEL_0; break;
		default: cu_error("Invalid DMA stream for I2S2"); break;
		}
		i2ss = &hw_i2s_2;
		i2ss->i2s_handle.Instance = SPI2;
		break;
	case HWR_SPII2S3:
		cu_verify(sclk_pin == HWR_PB3 || sclk_pin == HWR_PC10);
		cu_verify(ws_pin == HWR_PA4 || ws_pin == HWR_PA15);
		cu_verify(sd_pin == HWR_PB5 || sd_pin == HWR_PC12);
		cu_verify(mclk_pin == HWR_NONE || mclk_pin == HWR_PC7);
		switch(dma_stream) {
		case HWR_DMA1_STREAM0: dma_channel = DMA_CHANNEL_0; break;
		case HWR_DMA1_STREAM7: dma_channel = DMA_CHANNEL_0; break;
		default: cu_error("Invalid DMA stream for I2S3"); break;
		}
		i2ss = &hw_i2s_3;
		i2ss->i2s_handle.Instance = SPI3;
		break;
	default:
		cu_error("i2s does not identify an I2S peripheral");
		break;
	}

	id = hw_resource_assign(i2s, (intptr_t)i2ss);
	i2ss->sclk = hw_pin_assign(sclk_pin);
	i2ss->ws = hw_pin_assign(ws_pin);
	i2ss->sd = hw_pin_assign(sd_pin);
	i2ss->mclk = HW_ASSIGNMENT_ID_NULL;
	if(mclk_pin != HWR_NONE) {
		i2ss->mclk = hw_pin_assign(mclk_pin);
		//hw_pin_configure(mclk_pin, HWPM_AF_PP);
	}
	i2ss->dma_stream = hw_resource_assign(dma_stream, 0);
	i2ss->buf = NULL;
	i2ss->buf_len = 0;

	i2ss->dma_handle = &hw_dma_streams[dma_stream - HWR_DMA1_STREAM0];
	i2ss->dma_handle->Instance =
			hw_resource_definitions[dma_stream].dma.stream;
	i2ss->dma_handle->Init.Channel = dma_channel;
	i2ss->dma_handle->Init.Direction = DMA_MEMORY_TO_PERIPH;
	i2ss->dma_handle->Init.PeriphInc = DMA_PINC_DISABLE;
	i2ss->dma_handle->Init.MemInc = DMA_MINC_ENABLE;
	i2ss->dma_handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	i2ss->dma_handle->Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	i2ss->dma_handle->Init.Mode = DMA_CIRCULAR;
	i2ss->dma_handle->Init.Priority = DMA_PRIORITY_VERY_HIGH;
	i2ss->dma_handle->Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	i2ss->dma_handle->Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
	i2ss->dma_handle->Init.MemBurst = DMA_PBURST_SINGLE;
	i2ss->dma_handle->Init.PeriphBurst = DMA_PBURST_SINGLE;
	i2ss->dma_handle->Parent = &i2ss->i2s_handle;

	i2ss->i2s_handle.Init.Mode = I2S_MODE_MASTER_TX;
	i2ss->i2s_handle.Init.Standard = I2S_STANDARD_PHILIPS;
	i2ss->i2s_handle.Init.DataFormat = I2S_DATAFORMAT_16B;
	i2ss->i2s_handle.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
	i2ss->i2s_handle.Init.AudioFreq = I2S_AUDIOFREQ_48K;
	i2ss->i2s_handle.Init.CPOL = I2S_CPOL_LOW;
	i2ss->i2s_handle.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
	i2ss->i2s_handle.hdmarx = NULL;
	i2ss->i2s_handle.hdmatx = i2ss->dma_handle;

	switch(i2s) {
	case HWR_SPII2S2:
		hw_pin_configure(i2ss->sclk, HWPM_AF_PP | HWPM_ALT_5);
		hw_pin_configure(i2ss->ws, HWPM_AF_PP | HWPM_ALT_5);
		hw_pin_configure(i2ss->sd, HWPM_AF_PP | HWPM_ALT_5);
		if(i2ss->mclk != HW_ASSIGNMENT_ID_NULL) {
			hw_pin_configure(i2ss->mclk, HWPM_AF_PP | HWPM_ALT_5);
		}
		break;
	case HWR_SPII2S3:
		hw_pin_configure(i2ss->sclk, HWPM_AF_PP | HWPM_ALT_6);
		hw_pin_configure(i2ss->ws, HWPM_AF_PP | HWPM_ALT_6);
		hw_pin_configure(i2ss->sd, HWPM_AF_PP | HWPM_ALT_6);
		if(i2ss->mclk != HW_ASSIGNMENT_ID_NULL) {
			hw_pin_configure(i2ss->mclk, HWPM_AF_PP | HWPM_ALT_6);
		}
		break;
	default:
		cu_error("i2s does not identify an I2S peripheral");
		break;
	}

	if(hw_clock_i2s_pll_enable_count == 0) {
		__HAL_RCC_PLLI2S_ENABLE();
	}
	++hw_clock_i2s_pll_enable_count;

	HAL_Delay(2);

	hw_clock_enable(hw_resource_definitions[i2s].clock_id);
	hw_clock_enable(hw_resource_definitions[dma_stream].clock_id);

	i2ss->running = false;

	return id;
}

void hw_i2s_deassign(hw_assignment_id_t id) {
	hw_i2s_struct_t * i2ss = (hw_i2s_struct_t *)hw_resource_get_user(id);

	if(i2ss->running) {
		hw_i2s_stop(id);
	}
	hw_clock_disable(hw_resource_definitions[hw_resource_get_resource_id(id)].clock_id);
	hw_clock_disable(hw_resource_definitions[i2ss->dma_stream].clock_id);
	cu_verify(hw_clock_i2s_pll_enable_count > 0);
	--hw_clock_i2s_pll_enable_count;
	if(hw_clock_i2s_pll_enable_count == 0) {
		__HAL_RCC_PLLI2S_DISABLE();
	}

	hw_resource_deassign(i2ss->dma_stream);
	i2ss->dma_stream = HW_ASSIGNMENT_ID_NULL;
	i2ss->dma_handle->Parent = NULL;
	i2ss->dma_handle = NULL;

	hw_pin_deassign(i2ss->ws);
	hw_pin_deassign(i2ss->sclk);
	hw_pin_deassign(i2ss->sd);
	if(i2ss->mclk != HW_ASSIGNMENT_ID_NULL) {
		hw_pin_deassign(i2ss->mclk);
	}

	i2ss->ws = HW_ASSIGNMENT_ID_NULL;
	i2ss->sclk = HW_ASSIGNMENT_ID_NULL;
	i2ss->sd = HW_ASSIGNMENT_ID_NULL;
	i2ss->mclk = HW_ASSIGNMENT_ID_NULL;

	hw_resource_deassign(id);
}

void hw_i2s_start_output(hw_assignment_id_t id, hw_i2s_rate_t rate,
		void * buf, size_t buf_len, hw_i2s_fill_func_t fill_func) {
	hw_i2s_struct_t * i2ss = (hw_i2s_struct_t *)hw_resource_get_user(id);

	cu_verify(fill_func != NULL);

	i2ss->i2s_handle.Init.AudioFreq = rate;
	i2ss->fill_func = fill_func;
	i2ss->buf = buf;
	i2ss->buf_len = buf_len;

	hw_irq_enable(
			hw_resource_definitions[hw_resource_get_resource_id(id)].irq_id);
	hw_irq_enable(hw_resource_definitions[
			hw_resource_get_resource_id(i2ss->dma_stream)].irq_id);

	cu_verify(HAL_DMA_Init(i2ss->dma_handle) == HAL_OK);
	cu_verify(HAL_I2S_Init(&i2ss->i2s_handle) == HAL_OK);

	fill_func(buf, buf_len / 2);
	fill_func((uint8_t *)buf + buf_len / 2, buf_len / 2);

	cu_verify(HAL_I2S_Transmit_DMA(&i2ss->i2s_handle, buf,
			(uint16_t)buf_len / sizeof (uint16_t)) == HAL_OK);
}

void hw_i2s_stop(hw_assignment_id_t id) {
	hw_i2s_struct_t * i2ss = (hw_i2s_struct_t *)hw_resource_get_user(id);

	HAL_DMA_DeInit(i2ss->dma_handle);
	HAL_I2S_DeInit(&i2ss->i2s_handle);

	hw_irq_disable(
			hw_resource_definitions[hw_resource_get_resource_id(id)].irq_id);
	hw_irq_disable(hw_resource_definitions[
			hw_resource_get_resource_id(i2ss->dma_stream)].irq_id);

	i2ss->buf = NULL;
	i2ss->buf_len = 0;
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef * i2sh) {
	hw_i2s_fill_func_t fill_func = NULL;
	uint8_t * buf = NULL;
	size_t buf_len;

	if(i2sh == &hw_i2s_2.i2s_handle) {
		buf = (uint8_t *)hw_i2s_2.buf;
		buf_len = hw_i2s_2.buf_len;
		fill_func = hw_i2s_2.fill_func;
	} else if(i2sh == &hw_i2s_3.i2s_handle) {
		buf = (uint8_t *)hw_i2s_3.buf;
		buf_len = hw_i2s_3.buf_len;
		fill_func = hw_i2s_3.fill_func;
	}
	if(fill_func != NULL) {
		fill_func(buf, buf_len / 2);
	}
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef * i2sh) {
	hw_i2s_fill_func_t fill_func = NULL;
	uint8_t * buf = NULL;
	size_t buf_len;

	if(i2sh == &hw_i2s_2.i2s_handle) {
		buf = (uint8_t *)hw_i2s_2.buf;
		buf_len = hw_i2s_2.buf_len;
		fill_func = hw_i2s_2.fill_func;
	} else if(i2sh == &hw_i2s_3.i2s_handle) {
		buf = (uint8_t *)hw_i2s_3.buf;
		buf_len = hw_i2s_3.buf_len;
		fill_func = hw_i2s_3.fill_func;
	}
	if(fill_func != NULL) {
		fill_func(buf + buf_len / 2, buf_len / 2);
	}
}

hw_assignment_id_t hw_spi_assign(hw_resource_id_t spi,
		hw_resource_id_t sck_pin, hw_resource_id_t miso_pin,
		hw_resource_id_t mosi_pin, hw_resource_id_t tx_dma,
		hw_resource_id_t rx_dma) {
	hw_assignment_id_t id;
	hw_spi_struct_t * spis = NULL;
	uint32_t tx_dma_channel = 0;
	uint32_t rx_dma_channel = 0;

	cu_verify(spi > HWR_NONE && spi < HWR_COUNT);
	cu_verify(hw_resource_definitions[spi].type == HWT_SPI);

	switch(spi) {
	case HWR_SPI1:
		// NSS = PA4, PA15
		cu_verify(sck_pin == HWR_PA5 || sck_pin == HWR_PB3);
		cu_verify(miso_pin == HWR_PA6 || miso_pin == HWR_PB4 ||
				miso_pin == HWR_NONE);
		cu_verify(mosi_pin == HWR_PA7 || mosi_pin == HWR_PB5 ||
				mosi_pin == HWR_NONE);
		switch(tx_dma) {
		case HWR_NONE: break;
		case HWR_DMA2_STREAM3: tx_dma_channel = DMA_CHANNEL_3; break;
		case HWR_DMA2_STREAM5: tx_dma_channel = DMA_CHANNEL_3; break;
		default: cu_error("Invalid DMA stream for SPI1"); break;
		}
		switch(rx_dma) {
		case HWR_NONE: break;
		case HWR_DMA2_STREAM0: rx_dma_channel = DMA_CHANNEL_3; break;
		case HWR_DMA2_STREAM2: rx_dma_channel = DMA_CHANNEL_3; break;
		default: cu_error("Invalid DMA stream for SPI1"); break;
		}
		spis = &hw_spi_1;
		spis->spi_handle.Instance = SPI1;
		break;
	case HWR_SPII2S2:
		// NSS = B9, B12, PI0
		cu_verify(sck_pin == HWR_PB10 || sck_pin == HWR_PB13 ||
				sck_pin == HWR_PI1);
		cu_verify(miso_pin == HWR_PB14 || miso_pin == HWR_PC2 ||
				miso_pin == HWR_PI2 || miso_pin == HWR_NONE);
		cu_verify(mosi_pin == HWR_PB15 || mosi_pin == HWR_PC3 ||
				mosi_pin == HWR_PI3 || mosi_pin == HWR_NONE);
		switch(tx_dma) {
		case HWR_NONE: break;
		case HWR_DMA1_STREAM4: tx_dma_channel = DMA_CHANNEL_3; break;
		default: cu_error("Invalid DMA stream for SPI2"); break;
		}
		switch(rx_dma) {
		case HWR_NONE: break;
		case HWR_DMA1_STREAM3: rx_dma_channel = DMA_CHANNEL_3; break;
		default: cu_error("Invalid DMA stream for SPI2"); break;
		}
		spis = &hw_spi_2;
		spis->spi_handle.Instance = SPI2;
		break;
	case HWR_SPII2S3:
		// NSS = PA4, PA15
		cu_verify(sck_pin == HWR_PB3 || sck_pin == HWR_PC10);
		cu_verify(miso_pin == HWR_PB4 || miso_pin == HWR_PC11 ||
				miso_pin == HWR_NONE);
		cu_verify(mosi_pin == HWR_PB5 || mosi_pin == HWR_PC12 ||
				mosi_pin == HWR_NONE);
		switch(tx_dma) {
		case HWR_NONE: break;
		case HWR_DMA1_STREAM5: tx_dma_channel = DMA_CHANNEL_0; break;
		case HWR_DMA1_STREAM7: tx_dma_channel = DMA_CHANNEL_0; break;
		default: cu_error("Invalid DMA stream for SPI3"); break;
		}
		switch(rx_dma) {
		case HWR_NONE: break;
		case HWR_DMA2_STREAM0: rx_dma_channel = DMA_CHANNEL_0; break;
		case HWR_DMA2_STREAM2: rx_dma_channel = DMA_CHANNEL_0; break;
		default: cu_error("Invalid DMA stream for SPI3"); break;
		}
		spis = &hw_spi_3;
		spis->spi_handle.Instance = SPI3;
		break;
	default:
		cu_error("spi does not identify an I2C peripheral");
		break;
	}

	id = hw_resource_assign(spi, (intptr_t)spis);
	spis->sck = hw_pin_assign(sck_pin);
	spis->miso = HW_ASSIGNMENT_ID_NULL;
	if(miso_pin != HWR_NONE) {
		spis->miso = hw_pin_assign(miso_pin);
	}
	spis->mosi = HW_ASSIGNMENT_ID_NULL;
	if(mosi_pin != HWR_NONE) {
		spis->mosi = hw_pin_assign(mosi_pin);
	}

	if(tx_dma != HWR_NONE) {
		spis->tx_dma_stream = hw_resource_assign(tx_dma, 0);
		spis->tx_dma_handle = &hw_dma_streams[tx_dma - HWR_DMA1_STREAM0];
		spis->tx_dma_handle->Instance =
				hw_resource_definitions[tx_dma].dma.stream;
		spis->tx_dma_handle->Init.Channel = tx_dma_channel;
		spis->tx_dma_handle->Init.Direction = DMA_MEMORY_TO_PERIPH;
		spis->tx_dma_handle->Init.PeriphInc = DMA_PINC_DISABLE;
		spis->tx_dma_handle->Init.MemInc = DMA_MINC_ENABLE;
		spis->tx_dma_handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		spis->tx_dma_handle->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		spis->tx_dma_handle->Init.Mode = DMA_CIRCULAR;
		spis->tx_dma_handle->Init.Priority = DMA_PRIORITY_HIGH;
		spis->tx_dma_handle->Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		spis->tx_dma_handle->Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
		spis->tx_dma_handle->Init.MemBurst = DMA_PBURST_SINGLE;
		spis->tx_dma_handle->Init.PeriphBurst = DMA_PBURST_SINGLE;
		spis->tx_dma_handle->Parent = &spis->spi_handle;
	}
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
	}

	switch(spi) {
	case HWR_SPI1:
	case HWR_SPII2S2:
		hw_pin_configure(spis->sck, HWPM_AF_PP | HWPM_ALT_5);
		if(spis->miso != HW_ASSIGNMENT_ID_NULL) {
			hw_pin_configure(spis->miso, HWPM_AF_PP | HWPM_ALT_5);
		}
		if(spis->mosi != HW_ASSIGNMENT_ID_NULL) {
			hw_pin_configure(spis->mosi, HWPM_AF_PP | HWPM_ALT_5);
		}
		break;
	case HWR_SPII2S3:
		hw_pin_configure(spis->sck, HWPM_AF_PP | HWPM_ALT_6);
		if(spis->miso != HW_ASSIGNMENT_ID_NULL) {
			hw_pin_configure(spis->miso, HWPM_AF_PP | HWPM_ALT_6);
		}
		if(spis->mosi != HW_ASSIGNMENT_ID_NULL) {
			hw_pin_configure(spis->mosi, HWPM_AF_PP | HWPM_ALT_6);
		}
		break;
	default:
		cu_error("spi does not identify an SPI peripheral");
		break;
	}

	hw_clock_enable(hw_resource_definitions[spi].clock_id);

	return id;
}

void hw_spi_deassign(hw_assignment_id_t id) {
	hw_spi_struct_t * spis = (hw_spi_struct_t *)hw_resource_get_user(id);

	hw_clock_disable(hw_resource_definitions[hw_resource_get_resource_id(id)].clock_id);

	cu_verify(HAL_SPI_DeInit(&spis->spi_handle) == HAL_OK);

	if(spis->tx_dma_stream != HW_ASSIGNMENT_ID_NULL) {
		cu_verify(HAL_DMA_DeInit(spis->tx_dma_handle) == HAL_OK);
		hw_resource_deassign(spis->tx_dma_stream);
		spis->tx_dma_stream = HW_ASSIGNMENT_ID_NULL;
		spis->tx_dma_handle->Parent = NULL;
		spis->tx_dma_handle = NULL;
	}

	if(spis->rx_dma_stream != HW_ASSIGNMENT_ID_NULL) {
		cu_verify(HAL_DMA_DeInit(spis->rx_dma_handle) == HAL_OK);
		hw_resource_deassign(spis->rx_dma_stream);
		spis->rx_dma_stream = HW_ASSIGNMENT_ID_NULL;
		spis->rx_dma_handle->Parent = NULL;
		spis->rx_dma_handle = NULL;
	}

	hw_pin_deassign(spis->sck);
	if(spis->miso != HW_ASSIGNMENT_ID_NULL) {
		hw_pin_deassign(spis->miso);
	}
	if(spis->mosi != HW_ASSIGNMENT_ID_NULL) {
		hw_pin_deassign(spis->mosi);
	}

	spis->sck = HW_ASSIGNMENT_ID_NULL;
	spis->miso = HW_ASSIGNMENT_ID_NULL;
	spis->mosi = HW_ASSIGNMENT_ID_NULL;

	hw_resource_deassign(id);
}

hw_assignment_id_t hw_i2c_assign(hw_resource_id_t i2c,
		hw_resource_id_t scl_pin, hw_resource_id_t sda_pin) {
	hw_assignment_id_t id;
	hw_i2c_struct_t * i2cs = NULL;

	cu_verify(i2c > HWR_NONE && i2c < HWR_COUNT);
	cu_verify(hw_resource_definitions[i2c].type == HWT_I2C);

	switch(i2c) {
	case HWR_I2C1:
		cu_verify(scl_pin == HWR_PB6 || scl_pin == HWR_PB8);
		cu_verify(sda_pin == HWR_PB7 || sda_pin == HWR_PB9);
		i2cs = &hw_i2c_1;
		i2cs->i2c_handle.Instance = I2C1;
		break;
	case HWR_I2C2:
		cu_verify(scl_pin == HWR_PB10 || scl_pin == HWR_PF0 ||
				scl_pin == HWR_PH4);
		cu_verify(sda_pin == HWR_PB11 || sda_pin == HWR_PF1 ||
				sda_pin == HWR_PH5);
		i2cs = &hw_i2c_2;
		i2cs->i2c_handle.Instance = I2C2;
		break;
	case HWR_I2C3:
		cu_verify(scl_pin == HWR_PA8 || scl_pin == HWR_PH7);
		cu_verify(sda_pin == HWR_PC9 || sda_pin == HWR_PH8);
		i2cs = &hw_i2c_3;
		i2cs->i2c_handle.Instance = I2C3;
		break;
	default:
		cu_error("i2c does not identify an I2C peripheral");
		break;
	}

	id = hw_resource_assign(i2c, (intptr_t)i2cs);
	i2cs->scl = hw_pin_assign(scl_pin);
	i2cs->sda = hw_pin_assign(sda_pin);

	hw_pin_configure(i2cs->scl, HWPM_AF_OD_PU | HWPM_ALT_4);
	hw_pin_configure(i2cs->sda, HWPM_AF_OD_PU | HWPM_ALT_4);

	hw_clock_enable(hw_resource_definitions[i2c].clock_id);

	return id;
}

void hw_i2c_deassign(hw_assignment_id_t id) {
	hw_i2c_struct_t * i2cs = (hw_i2c_struct_t *)hw_resource_get_user(id);

	hw_clock_disable(hw_resource_definitions[hw_resource_get_resource_id(id)].clock_id);

	hw_pin_deassign(i2cs->scl);
	hw_pin_deassign(i2cs->sda);

	i2cs->scl = HW_ASSIGNMENT_ID_NULL;
	i2cs->sda = HW_ASSIGNMENT_ID_NULL;

	hw_resource_deassign(id);
}

void hw_i2c_configure(hw_assignment_id_t id, uint8_t master_address) {
	hw_i2c_struct_t * i2cs = (hw_i2c_struct_t *)hw_resource_get_user(id);

	i2cs->i2c_handle.Init.ClockSpeed = 100000;
	i2cs->i2c_handle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	i2cs->i2c_handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	i2cs->i2c_handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	i2cs->i2c_handle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	i2cs->i2c_handle.Init.DutyCycle = I2C_DUTYCYCLE_2;
	i2cs->i2c_handle.Init.OwnAddress1 = master_address;
	i2cs->i2c_handle.Init.OwnAddress2 = 0;

	HAL_I2C_Init(&i2cs->i2c_handle);
}

I2C_HandleTypeDef * hw_i2c_get_handle(hw_assignment_id_t id) {
	hw_i2c_struct_t * i2cs = (hw_i2c_struct_t *)hw_resource_get_user(id);
	return &i2cs->i2c_handle;
}

// ----------------------------------------------------------------------------

void __initialize_hardware(void);
static void hw_system_clock_config(void);

// This is the application hardware initialisation routine,
// redefined to add more inits.
//
// Called early from _start(), right after data & bss init, before
// constructors.
//
// After Reset the Cortex-M processor is in Thread mode,
// priority is Privileged, and the Stack is set to Main.
//
// Warning: The HAL requires the system timer, running at 1000 Hz
// and calling HAL_IncTick().

void __initialize_hardware(void)
{
  // Initialise the HAL Library; it must be the first function
  // to be executed before the call of any HAL function.
  HAL_Init();

  // Enable HSE Oscillator and activate PLL with HSE as source
  hw_system_clock_config();

  // Call the CSMSIS system clock routine to store the clock frequency
  // in the SystemCoreClock global RAM location.
  SystemCoreClockUpdate();

  HAL_ResumeTick();
}

// ----------------------------------------------------------------------------

/**
 * @brief  System Clock Configuration
 * @param  None
 * @retval None
 */
void hw_system_clock_config(void)
{
  // Enable Power Control clock
  __PWR_CLK_ENABLE();

  // The voltage scaling allows optimizing the power consumption when the
  // device is clocked below the maximum system frequency, to update the
  // voltage scaling value regarding system frequency refer to product
  // datasheet.
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitTypeDef RCC_OscInitStruct;

#if defined(HSE_VALUE) && (HSE_VALUE != 0)
  // Enable HSE Oscillator and activate PLL with HSE as source.
  // This is tuned for STM32F4-DISCOVERY; update it for your board.
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  // This assumes the HSE_VALUE is a multiple of 1 MHz. If this is not
  // your case, you have to recompute these PLL constants.
  RCC_OscInitStruct.PLL.PLLM = (HSE_VALUE/1000000u);
#else
  // Use HSI and activate PLL with HSI as source.
  // This is tuned for NUCLEO-F411; update it for your board.
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  // 16 is the average calibration value, adjust for your own board.
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  // This assumes the HSI_VALUE is a multiple of 1 MHz. If this is not
  // your case, you have to recompute these PLL constants.
  RCC_OscInitStruct.PLL.PLLM = (HSI_VALUE/1000000u);
#endif

  RCC_OscInitStruct.PLL.PLLN = 336;
#if defined(STM32F401xC) || defined(STM32F401xE) || defined(STM32F411xE)
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4; /* 84 MHz */
#elif defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx) || defined(STM32F439xx)
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; /* 168 MHz */
#elif defined(STM32F405xx) || defined(STM32F415xx) || defined(STM32F407xx) || defined(STM32F417xx)
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; /* 168 MHz */
#elif defined(STM32F446xx)
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; /* 168 MHz */
#else
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4; /* 84 MHz, conservative */
#endif
  RCC_OscInitStruct.PLL.PLLQ = 7; /* To make USB work. */
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  // Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
  // clocks dividers
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
      | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
#if defined(STM32F401xC) || defined(STM32F401xE) || defined(STM32F411xE)
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
#else
  // This is expected to work for most large cores.
  // Check and update it for your own configuration.
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
#endif

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

void SPI1_IRQHandler(void);
void SPI2_IRQHandler(void);
void SPI3_IRQHandler(void);
void DMA1_Stream0_IRQHandler(void);
void DMA1_Stream1_IRQHandler(void);
void DMA1_Stream2_IRQHandler(void);
void DMA1_Stream3_IRQHandler(void);
void DMA1_Stream4_IRQHandler(void);
void DMA1_Stream5_IRQHandler(void);
void DMA1_Stream6_IRQHandler(void);
void DMA1_Stream7_IRQHandler(void);
void DMA2_Stream0_IRQHandler(void);
void DMA2_Stream1_IRQHandler(void);
void DMA2_Stream2_IRQHandler(void);
void DMA2_Stream3_IRQHandler(void);
void DMA2_Stream4_IRQHandler(void);
void DMA2_Stream5_IRQHandler(void);
void DMA2_Stream6_IRQHandler(void);
void DMA2_Stream7_IRQHandler(void);

void __attribute__ ((section(".after_vectors"))) SysTick_Handler(void) {
	HAL_IncTick();
}

void __attribute__ ((section(".after_vectors"))) SPI1_IRQHandler(void) {
	HAL_SPI_IRQHandler(&hw_spi_1.spi_handle);
}

void __attribute__ ((section(".after_vectors"))) SPI2_IRQHandler(void) {
	HAL_SPI_IRQHandler(&hw_spi_2.spi_handle);
}

void __attribute__ ((section(".after_vectors"))) SPI3_IRQHandler(void) {
	HAL_SPI_IRQHandler(&hw_spi_3.spi_handle);
}

void __attribute__ ((section(".after_vectors")))
DMA1_Stream0_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[0]);
}

void __attribute__ ((section(".after_vectors")))
DMA1_Stream1_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[1]);
}

void __attribute__ ((section(".after_vectors")))
DMA1_Stream2_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[2]);
}

void __attribute__ ((section(".after_vectors")))
DMA1_Stream3_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[3]);
}

void __attribute__ ((section(".after_vectors")))
DMA1_Stream4_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[4]);
}

void __attribute__ ((section(".after_vectors")))
DMA1_Stream5_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[5]);
}

void __attribute__ ((section(".after_vectors")))
DMA1_Stream6_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[6]);
}

void __attribute__ ((section(".after_vectors")))
DMA1_Stream7_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[7]);
}

void __attribute__ ((section(".after_vectors")))
DMA2_Stream0_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[8]);
}

void __attribute__ ((section(".after_vectors")))
DMA2_Stream1_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[9]);
}

void __attribute__ ((section(".after_vectors")))
DMA2_Stream2_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[10]);
}

void __attribute__ ((section(".after_vectors")))
DMA2_Stream3_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[11]);
}

void __attribute__ ((section(".after_vectors")))
DMA2_Stream4_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[12]);
}

void __attribute__ ((section(".after_vectors")))
DMA2_Stream5_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[13]);
}

void __attribute__ ((section(".after_vectors")))
DMA2_Stream6_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[14]);
}

void __attribute__ ((section(".after_vectors")))
DMA2_Stream7_IRQHandler(void) {
	HAL_DMA_IRQHandler(&hw_dma_streams[15]);
}

