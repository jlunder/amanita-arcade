/*
 * hardware.h
 *
 *  Created on: Oct 13, 2015
 *      Author: jlunder
 */

#ifndef HARDWARE_H_
#define HARDWARE_H_


#include "heart_control.h"

#include "resource_assignment.h"

#define HW_ASSIGNMENT_MAX 256
#define HW_ASSIGNMENT_START 1

#define HW_ASSIGNMENT_ID_NULL 0

typedef enum {
	HWR_NONE,

	HWR_PA0,
	HWR_PA1,
	HWR_PA2,
	HWR_PA3,
	HWR_PA4,
	HWR_PA5,
	HWR_PA6,
	HWR_PA7,
	HWR_PA8,
	HWR_PA9,
	HWR_PA10,
	HWR_PA11,
	HWR_PA12,
	HWR_PA13,
	HWR_PA14,
	HWR_PA15,
	HWR_PB0,
	HWR_PB1,
	HWR_PB2,
	HWR_PB3,
	HWR_PB4,
	HWR_PB5,
	HWR_PB6,
	HWR_PB7,
	HWR_PB8,
	HWR_PB9,
	HWR_PB10,
	HWR_PB11,
	HWR_PB12,
	HWR_PB13,
	HWR_PB14,
	HWR_PB15,
	HWR_PC0,
	HWR_PC1,
	HWR_PC2,
	HWR_PC3,
	HWR_PC4,
	HWR_PC5,
	HWR_PC6,
	HWR_PC7,
	HWR_PC8,
	HWR_PC9,
	HWR_PC10,
	HWR_PC11,
	HWR_PC12,
	HWR_PC13,
	HWR_PC14,
	HWR_PC15,
	HWR_PD0,
	HWR_PD1,
	HWR_PD2,
	HWR_PD3,
	HWR_PD4,
	HWR_PD5,
	HWR_PD6,
	HWR_PD7,
	HWR_PD8,
	HWR_PD9,
	HWR_PD10,
	HWR_PD11,
	HWR_PD12,
	HWR_PD13,
	HWR_PD14,
	HWR_PD15,
	HWR_PE0,
	HWR_PE1,
	HWR_PE2,
	HWR_PE3,
	HWR_PE4,
	HWR_PE5,
	HWR_PE6,
	HWR_PE7,
	HWR_PE8,
	HWR_PE9,
	HWR_PE10,
	HWR_PE11,
	HWR_PE12,
	HWR_PE13,
	HWR_PE14,
	HWR_PE15,
	HWR_PF0,
	HWR_PF1,
	HWR_PF2,
	HWR_PF3,
	HWR_PF4,
	HWR_PF5,
	HWR_PF6,
	HWR_PF7,
	HWR_PF8,
	HWR_PF9,
	HWR_PF10,
	HWR_PF11,
	HWR_PF12,
	HWR_PF13,
	HWR_PF14,
	HWR_PF15,
	HWR_PG0,
	HWR_PG1,
	HWR_PG2,
	HWR_PG3,
	HWR_PG4,
	HWR_PG5,
	HWR_PG6,
	HWR_PG7,
	HWR_PG8,
	HWR_PG9,
	HWR_PG10,
	HWR_PG11,
	HWR_PG12,
	HWR_PG13,
	HWR_PG14,
	HWR_PG15,
	HWR_PH0,
	HWR_PH1,
	HWR_PH2,
	HWR_PH3,
	HWR_PH4,
	HWR_PH5,
	HWR_PH6,
	HWR_PH7,
	HWR_PH8,
	HWR_PH9,
	HWR_PH10,
	HWR_PH11,
	HWR_PH12,
	HWR_PH13,
	HWR_PH14,
	HWR_PH15,
	HWR_PI0,
	HWR_PI1,
	HWR_PI2,
	HWR_PI3,
	HWR_PI4,
	HWR_PI5,
	HWR_PI6,
	HWR_PI7,
	HWR_PI8,
	HWR_PI9,
	HWR_PI10,
	HWR_PI11,

	HWR_TIM1,
	HWR_TIM2,
	HWR_TIM3,
	HWR_TIM4,
	HWR_TIM5,
	HWR_TIM6,
	HWR_TIM7,
	HWR_TIM8,
	HWR_TIM9,
	HWR_TIM10,
	HWR_TIM11,
	HWR_TIM12,
	HWR_TIM13,
	HWR_TIM14,

	HWR_DMA1_CH0,
	HWR_DMA1_CH1,
	HWR_DMA1_CH2,
	HWR_DMA1_CH3,
	HWR_DMA1_CH4,
	HWR_DMA1_CH5,
	HWR_DMA1_CH6,
	HWR_DMA1_CH7,
	HWR_DMA2_CH0,
	HWR_DMA2_CH1,
	HWR_DMA2_CH2,
	HWR_DMA2_CH3,
	HWR_DMA2_CH4,
	HWR_DMA2_CH5,
	HWR_DMA2_CH6,
	HWR_DMA2_CH7,

	HWR_ADC1,
	HWR_ADC2,
	HWR_ADC3,

	HWR_DAC,

	HWR_SPI1,
	HWR_SPII2S2,
	HWR_SPII2S3,

	HWR_I2C1,
	HWR_I2C2,
	HWR_I2C3,

	HWR_CAN1,
	HWR_CAN2,

	HWR_USART1,
	HWR_USART2,
	HWR_USART3,
	HWR_UART4,
	HWR_UART5,
	HWR_USART6,

	HWR_COUNT,
	HWR_INVALID = 0x7FFFFFFF,
} hw_resource_id_t;

typedef uint32_t hw_assignment_id_t;

typedef enum {
	HWT_GPIO,
	HWT_TIM,
	HWT_DMA,
	HWT_ADC,
	HWT_DAC,
	HWT_SPI,
	HWT_SPII2S,
	HWT_I2C,
	HWT_CAN,
	HWT_USART,
	HWT_UART,
	HWT_INVALID = 0x7FFFFFFF,
} hw_type_t;

typedef enum {
	HWC_GPIOA,
	HWC_GPIOB,
	HWC_GPIOC,
	HWC_GPIOD,
	HWC_GPIOE,
	HWC_GPIOF,
	HWC_GPIOG,
	HWC_GPIOH,
	HWC_GPIOI,
	HWC_TIM1,
	HWC_TIM2,
	HWC_TIM3,
	HWC_TIM4,
	HWC_TIM5,
	HWC_TIM6,
	HWC_TIM7,
	HWC_TIM8,
	HWC_TIM9,
	HWC_TIM10,
	HWC_TIM11,
	HWC_TIM12,
	HWC_TIM13,
	HWC_TIM14,
	HWC_DMA1,
	HWC_DMA2,
	HWC_ADC1,
	HWC_ADC2,
	HWC_ADC3,
	HWC_DAC,
	HWC_SPI1,
	HWC_SPII2S2,
	HWC_SPII2S3,
	HWC_I2C1,
	HWC_I2C2,
	HWC_I2C3,
	HWC_CAN1,
	HWC_CAN2,
	HWC_USART1,
	HWC_USART2,
	HWC_USART3,
	HWC_UART4,
	HWC_UART5,
	HWC_USART6,

	HWC_COUNT,
	HWC_INVALID = 0x7FFFFFFF,
} hw_clock_id_t;

typedef struct {
	hw_type_t type;
	hw_clock_id_t clock_id;
	union {
		struct {
			GPIO_TypeDef * bank;
			uint32_t index;
		} gpio;
		TIM_TypeDef * tim;
		struct {
			DMA_TypeDef * controller;
			DMA_Stream_TypeDef * stream;
			uint32_t channel;
		} dma;
		ADC_TypeDef * adc;
		DAC_TypeDef * dac;
		CAN_TypeDef * can;
		SPI_TypeDef * spi;
		I2C_TypeDef * i2c;
		USART_TypeDef * usart;
	};
} hw_resource_definition_t;

typedef enum {
	HWPM_ALT_MASK = 0x000F,
	HWPM_ALT_0    = 0x0000,
	HWPM_ALT_1    = 0x0001,
	HWPM_ALT_2    = 0x0002,
	HWPM_ALT_3    = 0x0003,
	HWPM_ALT_4    = 0x0004,
	HWPM_ALT_5    = 0x0005,
	HWPM_ALT_6    = 0x0006,
	HWPM_ALT_7    = 0x0007,
	HWPM_ALT_8    = 0x0008,
	HWPM_ALT_9    = 0x0009,
	HWPM_ALT_10   = 0x000A,
	HWPM_ALT_11   = 0x000B,
	HWPM_ALT_12   = 0x000C,
	HWPM_ALT_13   = 0x000D,
	HWPM_ALT_14   = 0x000E,
	HWPM_ALT_15   = 0x000F,

	HWPM_MODE_MASK               = 0x0030,
	HWPM_MODE_INPUT              = 0x0000,
	HWPM_MODE_OUTPUT             = 0x0010,
	HWPM_MODE_ALTERNATE_FUNCTION = 0x0020,
	HWPM_MODE_ANALOG             = 0x0030,

	HWPM_PULL_MASK = 0x00C0,
	HWPM_PULL_NONE = 0x0000,
	HWPM_PULL_UP   = 0x0040,
	HWPM_PULL_DOWN = 0x0080,

	HWPM_DRIVE_MASK       = 0x0100,
	HWPM_DRIVE_PUSH_PULL  = 0x0000,
	HWPM_DRIVE_OPEN_DRAIN = 0x0100,

	HWPM_SPEED_MASK    = 0x0600,
	HWPM_SPEED_DEFAULT = 0x0000,
	HWPM_SPEED_MIN     = 0x0200,
	HWPM_SPEED_FAST1   = 0x0000,
	HWPM_SPEED_FAST2   = 0x0400,
	HWPM_SPEED_MAX     = 0x0600,

	HWPM_AF_PP     = HWPM_MODE_ALTERNATE_FUNCTION | HWPM_PULL_NONE | HWPM_DRIVE_PUSH_PULL  | HWPM_SPEED_DEFAULT,
	HWPM_AF_OD_PU  = HWPM_MODE_ALTERNATE_FUNCTION | HWPM_PULL_UP   | HWPM_DRIVE_OPEN_DRAIN | HWPM_SPEED_DEFAULT,
	HWPM_AF_OD_PD  = HWPM_MODE_ALTERNATE_FUNCTION | HWPM_PULL_DOWN | HWPM_DRIVE_OPEN_DRAIN | HWPM_SPEED_DEFAULT,
	HWPM_OUT_PP    = HWPM_MODE_OUTPUT             | HWPM_PULL_NONE | HWPM_DRIVE_PUSH_PULL  | HWPM_SPEED_DEFAULT,
	HWPM_OUT_OD    = HWPM_MODE_OUTPUT             | HWPM_PULL_NONE | HWPM_DRIVE_OPEN_DRAIN | HWPM_SPEED_DEFAULT,
	HWPM_OUT_OD_PU = HWPM_MODE_OUTPUT             | HWPM_PULL_NONE | HWPM_DRIVE_OPEN_DRAIN | HWPM_SPEED_DEFAULT,
	HWPM_OUT_OD_PD = HWPM_MODE_OUTPUT             | HWPM_PULL_NONE | HWPM_DRIVE_OPEN_DRAIN | HWPM_SPEED_DEFAULT,
	HWPM_IN        = HWPM_MODE_INPUT              | HWPM_PULL_NONE | HWPM_DRIVE_PUSH_PULL  | HWPM_SPEED_DEFAULT,
	HWPM_IN_PU     = HWPM_MODE_INPUT              | HWPM_PULL_NONE | HWPM_DRIVE_PUSH_PULL  | HWPM_SPEED_DEFAULT,
	HWPM_IN_PD     = HWPM_MODE_INPUT              | HWPM_PULL_NONE | HWPM_DRIVE_PUSH_PULL  | HWPM_SPEED_DEFAULT,
	HWPM_ANALOG    = HWPM_MODE_ANALOG             | HWPM_PULL_NONE | HWPM_DRIVE_PUSH_PULL  | HWPM_SPEED_DEFAULT,
} hw_pin_mode_t;

typedef enum {
	HWI2SR_8KHZ,
	HWI2SR_22_05KHZ,
	HWI2SR_24KHZ,
	HWI2SR_44_1KHZ,
	HWI2SR_48KHZ,
	HWI2SR_96KHZ,
	HWI2SR_192KHZ,
} hw_i2s_rate_t;

typedef void (*hw_i2s_fill_func_t)(void * buf, size_t buf_len);

hw_assignment_id_t hw_assignment_alloc(void);
void hw_assignment_free(hw_assignment_id_t id);

hw_resource_definition_t const * hw_get_resource_definition(
		hw_resource_id_t id);

hw_assignment_id_t hw_resource_assign(hw_resource_id_t resource_id, intptr_t user);
void hw_resource_deassign(hw_assignment_id_t assignment_id);
hw_resource_id_t hw_resource_get_resource_id(hw_assignment_id_t assignment_id);
intptr_t hw_resource_get_user(hw_assignment_id_t assignment_id);

void hw_clock_enable(hw_clock_id_t clock_id);
void hw_clock_disable(hw_clock_id_t clock_id);
void hw_clock_force_reset(hw_clock_id_t clock_id);

hw_assignment_id_t hw_pin_assign(hw_resource_id_t pin_id);
void hw_pin_deassign(hw_assignment_id_t id);
void hw_pin_configure(hw_assignment_id_t id, hw_pin_mode_t mode);

hw_assignment_id_t hw_i2s_assign(hw_resource_id_t i2s,
		hw_resource_id_t sclk_pin, hw_resource_id_t ws_pin,
		hw_resource_id_t sd_pin, hw_resource_id_t mclk_pin,
		hw_resource_id_t dma_channel);
void hw_i2s_deassign(hw_assignment_id_t id);
void hw_i2s_start_output(hw_assignment_id_t id, hw_i2s_rate_t rate,
		void * buf, size_t buf_len, hw_i2s_fill_func_t fill_func);
void hw_i2s_stop(hw_assignment_id_t id);

//hw_assignment_id_t hw_spi_assign(pins, dma)
//void hw_spi_output(buf, buf_len)
//bool hw_spi_is_busy()

//hw_button_monitor

//eh_setup()
//eh_tick_1ms()
//eh_loop()


#endif /* HARDWARE_H_ */
