/*
 * heart_control_peripherals.c
 *
 *  Created on: Oct 17, 2015
 *      Author: jlunder
 */


#include "heart_control_peripherals.h"

#define CS43L22_ADDRESS 0x94

#define CS43L22_MAP_CHIP_ID 0x01
#define CS43L22_MAP_PWR_CTRL1 0x02
#define CS43L22_MAP_PWR_CTRL2 0x04
#define CS43L22_MAP_CLK_CTRL  0x05
#define CS43L22_MAP_IF_CTRL1  0x06
#define CS43L22_MAP_IF_CTRL2  0x07
#define CS43L22_MAP_PASSTHROUGH_A_SELECT 0x08
#define CS43L22_MAP_PASSTHROUGH_B_SELECT 0x09
#define CS43L22_MAP_ANALOG_SET 0x0A
#define CS43L22_MAP_PASSTHROUGH_GANG_CTRL 0x0C
#define CS43L22_MAP_PLAYBACK_CTRL1 0x0D
#define CS43L22_MAP_MISC_CTRL 0x0E
#define CS43L22_MAP_PLAYBACK_CTRL2 0x0F
#define CS43L22_MAP_PASSTHROUGH_A_VOL 0x14
#define CS43L22_MAP_PASSTHROUGH_B_VOL 0x15
#define CS43L22_MAP_PCMA_VOL 0x1A
#define CS43L22_MAP_PCMB_VOL 0x1B
#define CS43L22_MAP_BEEP_FREQ_ONTIME 0x1C
#define CS43L22_MAP_BEEP_VOL_OFFTIME 0x1D
#define CS43L22_MAP_BEEP_TONE_CFG 0x1E
#define CS43L22_MAP_TONE_CTRL 0x1F
#define CS43L22_MAP_MASTER_A_VOL 0x20
#define CS43L22_MAP_MASTER_B_VOL 0x21
#define CS43L22_MAP_HP_A_VOL 0x22
#define CS43L22_MAP_HP_B_VOL 0x23
#define CS43L22_MAP_SPEAK_A_VOL 0x24
#define CS43L22_MAP_SPEAK_B_VOL 0x25
#define CS43L22_MAP_CH_MIX_SWAP 0x26
#define CS43L22_MAP_LIMIT_CTRL1 0x27
#define CS43L22_MAP_LIMIT_CTRL2 0x28
#define CS43L22_MAP_LIMIT_ATTACK 0x29
#define CS43L22_MAP_OVFL_CLK_STATUS 0x2E
#define CS43L22_MAP_BATT_COMP 0x2F
#define CS43L22_MAP_VP_BATT_LEVEL 0x30
#define CS43L22_MAP_SPEAK_STATUS 0x31
#define CS43L22_MAP_CHARGE_PUMP_FREQ 0x34

static hw_assignment_id_t cs43l22_i2s;
static hw_assignment_id_t cs43l22_i2c;
static hw_assignment_id_t cs43l22_reset;

static short cs43l22_buf[128];

static hw_assignment_id_t ws2801_spi;

void hcp_init(void) {
	cs43l22_init();
	ws2801_init();
}

void cs43l22_init(void) {
	uint8_t tmp;

	cs43l22_i2s = hw_i2s_assign(HWR_SPII2S3, HWR_PC10, HWR_PA4, HWR_PC12, HWR_PC7, HWR_DMA1_STREAM7);
	cs43l22_i2c = hw_i2c_assign(HWR_I2C1, HWR_PB6, HWR_PB9);
	cs43l22_reset = hw_pin_assign(HWR_PD4);

	hw_pin_configure(cs43l22_reset, HWPM_OUT_PP);
	// put cs43l22 into reset
	GPIOD->BSRR = GPIO_PIN_4 << 16;

	hw_i2c_configure(cs43l22_i2c, 0x33);

	// bring cs43l22 out of reset
	GPIOD->BSRR = GPIO_PIN_4;
	HAL_Delay(2);

	cs43l22_write_register(CS43L22_MAP_PLAYBACK_CTRL1, 0x01);
	// Init sequence
	cs43l22_write_register(0x00, 0x99);
	cs43l22_write_register(0x47, 0x80);
	tmp = cs43l22_read_register(0x32);
	cs43l22_write_register(0x32, tmp | 0x80);
	tmp = cs43l22_read_register(0x32);
	cs43l22_write_register(0x32, tmp & (uint8_t)~0x80);
	cs43l22_write_register(0x00, 0x00);

	cs43l22_write_register(CS43L22_MAP_PWR_CTRL2, 0xAF);
	cs43l22_write_register(CS43L22_MAP_PLAYBACK_CTRL1, 0x70);
	cs43l22_write_register(CS43L22_MAP_CLK_CTRL, 0x81);
	cs43l22_write_register(CS43L22_MAP_IF_CTRL1, 0x07);
	cs43l22_write_register(CS43L22_MAP_ANALOG_SET, 0x00);
	cs43l22_write_register(CS43L22_MAP_LIMIT_CTRL1, 0x00);
	cs43l22_write_register(CS43L22_MAP_PCMA_VOL, 0x0A);
	cs43l22_write_register(CS43L22_MAP_PCMB_VOL, 0x0A);
	cs43l22_write_register(CS43L22_MAP_TONE_CTRL, 0x0F);
	cs43l22_write_register(CS43L22_MAP_PWR_CTRL1, 0x9E);
}

void cs43l22_start(hw_i2s_fill_func_t fill_func) {
	hw_i2s_start_output(cs43l22_i2s, HWI2SR_8KHZ, cs43l22_buf,
			sizeof cs43l22_buf, fill_func);
}

void cs43l22_stop(void) {
	hw_i2s_stop(cs43l22_i2s);
}

void cs43l22_write_register(uint8_t address, uint8_t value) {
	I2C_HandleTypeDef * i2ch = hw_i2c_get_handle(cs43l22_i2c);
	cu_verify(HAL_I2C_Mem_Write(i2ch, CS43L22_ADDRESS, address,
			I2C_MEMADD_SIZE_8BIT, &value, 1, 50) == HAL_OK);
}

uint8_t cs43l22_read_register(uint8_t address) {
	uint8_t value;
	I2C_HandleTypeDef * i2ch = hw_i2c_get_handle(cs43l22_i2c);
	cu_verify(HAL_I2C_Mem_Read(i2ch, CS43L22_ADDRESS, address,
			I2C_MEMADD_SIZE_8BIT, &value, 1, 50) == HAL_OK);
	return value;
}

void ws2801_init(void) {
	ws2801_spi = hw_spi_assign(HWR_SPI1, HWR_PA5, HWR_NONE, HWR_PA7,
			HWR_DMA2_STREAM3, HWR_NONE);
}

void ws2801_output(void * buf, size_t buf_len) {
	hw_spi_transmit(ws2801_spi, buf, buf_len);
	HAL_Delay(2);
}

