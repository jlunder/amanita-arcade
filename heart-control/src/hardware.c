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
	I2S_HandleTypeDef i2s_handle;
	DMA_HandleTypeDef dma_handle;
	void * buf;
	size_t buf_len;
	bool running;
	uint8_t pad0[3];
} hw_i2s_struct_t;

static hw_resource_definition_t const hw_resource_definitions[HWR_COUNT] = {
		{.type = HWT_INVALID},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOA, .gpio = {.bank = GPIOA, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOB, .gpio = {.bank = GPIOB, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOC, .gpio = {.bank = GPIOC, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOD, .gpio = {.bank = GPIOD, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOE, .gpio = {.bank = GPIOE, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOF, .gpio = {.bank = GPIOF, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOG, .gpio = {.bank = GPIOG, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 11}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 12}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 13}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 14}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOH, .gpio = {.bank = GPIOH, .index = 15}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .gpio = {.bank = GPIOI, .index = 0}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .gpio = {.bank = GPIOI, .index = 1}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .gpio = {.bank = GPIOI, .index = 2}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .gpio = {.bank = GPIOI, .index = 3}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .gpio = {.bank = GPIOI, .index = 4}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .gpio = {.bank = GPIOI, .index = 5}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .gpio = {.bank = GPIOI, .index = 6}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .gpio = {.bank = GPIOI, .index = 7}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .gpio = {.bank = GPIOI, .index = 8}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .gpio = {.bank = GPIOI, .index = 9}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .gpio = {.bank = GPIOI, .index = 10}},
		{.type = HWT_GPIO, .clock_id = HWC_GPIOI, .gpio = {.bank = GPIOI, .index = 11}},
		{.type = HWT_TIM, .clock_id = HWC_TIM1, .tim = TIM1},
		{.type = HWT_TIM, .clock_id = HWC_TIM2, .tim = TIM2},
		{.type = HWT_TIM, .clock_id = HWC_TIM3, .tim = TIM3},
		{.type = HWT_TIM, .clock_id = HWC_TIM4, .tim = TIM4},
		{.type = HWT_TIM, .clock_id = HWC_TIM5, .tim = TIM5},
		{.type = HWT_TIM, .clock_id = HWC_TIM6, .tim = TIM6},
		{.type = HWT_TIM, .clock_id = HWC_TIM7, .tim = TIM7},
		{.type = HWT_TIM, .clock_id = HWC_TIM8, .tim = TIM8},
		{.type = HWT_TIM, .clock_id = HWC_TIM9, .tim = TIM9},
		{.type = HWT_TIM, .clock_id = HWC_TIM10, .tim = TIM10},
		{.type = HWT_TIM, .clock_id = HWC_TIM11, .tim = TIM11},
		{.type = HWT_TIM, .clock_id = HWC_TIM12, .tim = TIM12},
		{.type = HWT_TIM, .clock_id = HWC_TIM13, .tim = TIM13},
		{.type = HWT_TIM, .clock_id = HWC_TIM14, .tim = TIM14},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .dma = {.controller = DMA1, .stream = DMA1_Stream0, .channel = 0}},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .dma = {.controller = DMA1, .stream = DMA1_Stream1, .channel = 1}},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .dma = {.controller = DMA1, .stream = DMA1_Stream2, .channel = 2}},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .dma = {.controller = DMA1, .stream = DMA1_Stream3, .channel = 3}},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .dma = {.controller = DMA1, .stream = DMA1_Stream4, .channel = 4}},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .dma = {.controller = DMA1, .stream = DMA1_Stream5, .channel = 5}},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .dma = {.controller = DMA1, .stream = DMA1_Stream6, .channel = 6}},
		{.type = HWT_DMA, .clock_id = HWC_DMA1, .dma = {.controller = DMA1, .stream = DMA1_Stream7, .channel = 7}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .dma = {.controller = DMA2, .stream = DMA2_Stream0, .channel = 0}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .dma = {.controller = DMA2, .stream = DMA2_Stream1, .channel = 1}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .dma = {.controller = DMA2, .stream = DMA2_Stream2, .channel = 2}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .dma = {.controller = DMA2, .stream = DMA2_Stream3, .channel = 3}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .dma = {.controller = DMA2, .stream = DMA2_Stream4, .channel = 4}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .dma = {.controller = DMA2, .stream = DMA2_Stream5, .channel = 5}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .dma = {.controller = DMA2, .stream = DMA2_Stream6, .channel = 6}},
		{.type = HWT_DMA, .clock_id = HWC_DMA2, .dma = {.controller = DMA2, .stream = DMA2_Stream7, .channel = 7}},
		{.type = HWT_ADC, .clock_id = HWC_ADC1, .adc = ADC1},
		{.type = HWT_ADC, .clock_id = HWC_ADC2, .adc = ADC2},
		{.type = HWT_ADC, .clock_id = HWC_ADC3, .adc = ADC3},
		{.type = HWT_DAC, .clock_id = HWC_DAC, .dac = DAC},
		{.type = HWT_SPI, .clock_id = HWC_SPI1, .spi = SPI1},
		{.type = HWT_SPII2S, .clock_id = HWC_SPII2S2, .spi = SPI2},
		{.type = HWT_SPII2S, .clock_id = HWC_SPII2S3, .spi = SPI3},
		{.type = HWT_I2C, .clock_id = HWC_I2C1, .i2c = I2C1},
		{.type = HWT_I2C, .clock_id = HWC_I2C2, .i2c = I2C2},
		{.type = HWT_I2C, .clock_id = HWC_I2C3, .i2c = I2C3},
		{.type = HWT_CAN, .clock_id = HWC_CAN1, .can = CAN1},
		{.type = HWT_CAN, .clock_id = HWC_CAN2, .can = CAN2},
		{.type = HWT_USART, .clock_id = HWC_USART1, .usart = USART1},
		{.type = HWT_USART, .clock_id = HWC_USART2, .usart = USART2},
		{.type = HWT_USART, .clock_id = HWC_USART3, .usart = USART3},
		{.type = HWT_UART, .clock_id = HWC_UART4, .usart = UART4},
		{.type = HWT_UART, .clock_id = HWC_UART5, .usart = UART5},
		{.type = HWT_USART, .clock_id = HWC_USART6, .usart = USART6},
};

static size_t hw_clock_enable_counts[HWC_COUNT];

static hw_assignment_id_t hw_assignment_id_in_pool_count = 0;
static hw_assignment_id_t hw_assignment_id_total_count = 0;
static hw_assignment_id_t hw_assignment_id_next_id = HW_ASSIGNMENT_START;
static hw_assignment_id_t hw_assignment_id_pool[HW_ASSIGNMENT_MAX];

static hw_assignment_id_t hw_resource_assignments[HWR_COUNT];
static hw_resource_assignment_t hw_assignment_resources[HW_ASSIGNMENT_MAX];

static hw_i2s_struct_t hw_i2s_2;
static hw_i2s_struct_t hw_i2s_3;

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
	cu_verify(clock_id >= 0 && clock_id < HWC_COUNT);
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
	cu_verify(clock_id >= 0 && clock_id < HWC_COUNT);
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
	cu_verify(clock_id >= 0 && clock_id < HWC_COUNT);
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
		hw_resource_id_t dma_channel) {
	hw_assignment_id_t id;
	hw_i2s_struct_t * i2ss = NULL;

	cu_verify(i2s > HWR_NONE && i2s < HWR_COUNT);
	cu_verify(hw_resource_definitions[i2s].type == HWT_SPII2S);
	cu_verify(dma_channel > HWR_NONE && dma_channel < HWR_COUNT);
	cu_verify(hw_resource_definitions[dma_channel].type == HWT_DMA);

	switch(i2s) {
	case HWR_SPII2S2:
		i2ss = &hw_i2s_2;
		i2ss->i2s_handle.Instance = SPI2;
		break;
	case HWR_SPII2S3:
		i2ss = &hw_i2s_3;
		i2ss->i2s_handle.Instance = SPI3;
		break;
	default:
		cu_error("i2s does not identify an I2S peripheral");
		break;
	}

	id = hw_resource_assign(i2s, (intptr_t)i2ss);
	i2ss->ws = hw_pin_assign(sclk_pin);
	i2ss->sclk = hw_pin_assign(ws_pin);
	i2ss->sd = hw_pin_assign(sd_pin);
	i2ss->mclk = HW_ASSIGNMENT_ID_NULL;
	if(mclk_pin != HWR_NONE) {
		i2ss->mclk = hw_pin_assign(mclk_pin);
		//hw_pin_configure(mclk_pin, HWPM_AF_PP);
	}
	i2ss->buf = NULL;
	i2ss->buf_len = 0;

	i2ss->dma_handle.Instance =
			hw_resource_definitions[dma_channel].dma.stream;
	i2ss->dma_handle.Init.Channel =
			hw_resource_definitions[dma_channel].dma.channel;

	i2ss->running = false;

	return id;
}

void hw_i2s_deassign(hw_assignment_id_t id) {
	hw_i2s_struct_t * i2ss = (hw_i2s_struct_t *)hw_resource_get_user(id);

	if(i2ss->running) {
		hw_i2s_stop(id);
	}
	hw_pin_deassign(i2ss->ws);
	hw_pin_deassign(i2ss->sclk);
	hw_pin_deassign(i2ss->sd);
	if(i2ss->mclk != HW_ASSIGNMENT_ID_NULL) {
		hw_pin_deassign(i2ss->mclk);
	}
	hw_resource_deassign(id);
}

void hw_i2s_start_output(hw_assignment_id_t id, hw_i2s_rate_t rate,
		void * buf, size_t buf_len, hw_i2s_fill_func_t fill_func) {
	hw_i2s_struct_t * i2ss = (hw_i2s_struct_t *)hw_resource_get_user(id);

	(void)i2ss;
	(void)rate;
	(void)buf;
	(void)buf_len;
	(void)fill_func;
	/*
	i2ss->i2s_handle.Init.
	i2ss->dma_handle.Instance =
			hw_resource_definitions[dma_channel].dma.controller;
	i2ss->dma_handle.Init.Channel =
	*/
}
void hw_i2s_stop(hw_assignment_id_t id) {
	(void)id;
}

void SysTick_Handler(void) {
	HAL_IncTick();
}

