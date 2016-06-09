/*
 * heart_control_peripherals.c
 *
 *  Created on: Oct 17, 2015
 *      Author: jlunder
 */


#include "peripherals.h"

#define CS43L22_ADDRESS (0x4A << 1)

#define CS43L22_MAP_CHIP_ID 0x01
#define CS43L22_MAP_PWR_CTRL1 0x02
#define CS43L22_MAP_PWR_CTRL2 0x04
#define CS43L22_MAP_CLK_CTRL 0x05
#define CS43L22_MAP_IF_CTRL1 0x06
#define CS43L22_MAP_IF_CTRL2 0x07
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

#define MPR121_ADDRESS (0x5A << 1)

#define MPR121_MAP_EXTS 0x00
#define MPR121_MAP_EXOOR 0x02
#define MPR121_MAP_EXFDL 0x04
#define MPR121_MAP_EXBV 0x1E
#define MPR121_MAP_MHDR 0x2B
#define MPR121_MAP_NHDR 0x2C
#define MPR121_MAP_NCLR 0x2D
#define MPR121_MAP_FDLR 0x2E
#define MPR121_MAP_MHDF 0x2F
#define MPR121_MAP_NHDF 0x30
#define MPR121_MAP_NCLF 0x31
#define MPR121_MAP_FDLF 0x32
#define MPR121_MAP_EXTTH 0x41
#define MPR121_MAP_EXRTH 0x42
#define MPR121_MAP_AFEC1 0x5C
#define MPR121_MAP_AFEC2 0x5D
#define MPR121_MAP_EC 0x5E
#define MPR121_MAP_ACCR0 0x7B
#define MPR121_MAP_ACCR1 0x7C
#define MPR121_MAP_ACUSL 0x7D
#define MPR121_MAP_ACLSL 0x7E
#define MPR121_MAP_ACTL 0x7F
#define MPR121_THRESH_TOUCH 0x20
#define MPR121_THRESH_RELEASE 0x10

#define WS2811_BIT_RATE 800000L
#define WS2811_CLOCKS_PER_BIT 4
#define WS2811_RESET_US 50
#define WS2811_RESET_CLOCKS ((WS2811_BIT_RATE * WS2811_CLOCKS_PER_BIT * \
		WS2811_RESET_US + 999999L) / 1000000L)

static hw_assignment_id_t cs43l22_i2s = HW_ASSIGNMENT_ID_NULL;
static hw_assignment_id_t cs43l22_i2c = HW_ASSIGNMENT_ID_NULL;
static I2C_HandleTypeDef * cs43l22_i2c_handle = NULL;
static hw_assignment_id_t cs43l22_reset = HW_ASSIGNMENT_ID_NULL;

static short cs43l22_buf[128];

static hw_assignment_id_t mpr121_power = HW_ASSIGNMENT_ID_NULL;
static hw_assignment_id_t mpr121_i2c = HW_ASSIGNMENT_ID_NULL;
static I2C_HandleTypeDef * mpr121_i2c_handle = NULL;

static hw_assignment_id_t ws2801_spi = HW_ASSIGNMENT_ID_NULL;

static hw_assignment_id_t ws2811_i2s;
static uint32_t ws2811_buf[512];
static uint8_t const * ws2811_data_buf = NULL;
static size_t ws2811_data_buf_len = 0;
static size_t ws2811_data_buf_sent = 0;
static int32_t ws2811_reset_counter = 0;

static void ws2811_fill(void * buf, size_t buf_len);

void per_init(void) {
	HAL_NVIC_SetPriority(I2C3_EV_IRQn, 4, 0);
	HAL_NVIC_SetPriority(I2C3_ER_IRQn, 4, 0);
	HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 10, 0);
	HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 10, 0);
	HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 10, 0);
	HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 10, 0);
	HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 10, 0);
	HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 10, 0);
	HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 10, 0);
	HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 10, 0);

	cs43l22_init();
	mpr121_init();
	ws2811_init();
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
	cs43l22_i2c_handle = hw_i2c_get_handle(cs43l22_i2c);

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
	hw_i2s_start_output(cs43l22_i2s, HWI2SR_8KHZ,
			HWI2SF_16B | HWI2SF_LSB_JUSTIFIED, cs43l22_buf, sizeof cs43l22_buf,
			fill_func);
}

void cs43l22_stop(void) {
	hw_i2s_stop(cs43l22_i2s);
}

void cs43l22_write_register(uint8_t address, uint8_t value) {
	cu_verify(HAL_I2C_Mem_Write(cs43l22_i2c_handle, CS43L22_ADDRESS, address,
			I2C_MEMADD_SIZE_8BIT, &value, 1, 50) == HAL_OK);
}

uint8_t cs43l22_read_register(uint8_t address) {
	uint8_t value;
	cu_verify(HAL_I2C_Mem_Read(cs43l22_i2c_handle, CS43L22_ADDRESS, address,
			I2C_MEMADD_SIZE_8BIT, &value, 1, 50) == HAL_OK);
	return value;
}

void mpr121_init(void) {
	mpr121_power = hw_pin_assign(HWR_PC0);
	hw_pin_configure(mpr121_power, HWPM_OUT_PP);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0);
	HAL_Delay(2);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 1);
	HAL_Delay(50);
	mpr121_i2c = hw_i2c_assign(HWR_I2C3, HWR_PA8, HWR_PC9);
	hw_i2c_configure(mpr121_i2c, 0x33);
	hw_irq_enable(HWI_I2C3);
	mpr121_i2c_handle = hw_i2c_get_handle(mpr121_i2c);
}

void mpr121_write_register(uint8_t address, uint8_t value) {
	cu_verify(HAL_I2C_Mem_Write(mpr121_i2c_handle, MPR121_ADDRESS, address,
			I2C_MEMADD_SIZE_8BIT, &value, 1, 50) == HAL_OK);
}

void mpr121_write_registers(uint8_t address, void const * value,
		size_t size) {
	cu_verify(HAL_I2C_Mem_Write(mpr121_i2c_handle, MPR121_ADDRESS, address,
			I2C_MEMADD_SIZE_8BIT, (void *)value, (uint16_t)size, 50)
			== HAL_OK);
}

static HAL_StatusTypeDef mpr121_read_status = HAL_ERROR;

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
	if(hi2c == mpr121_i2c_handle) {
		mpr121_read_status = HAL_OK;
		__sync_synchronize();
	}
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	if(hi2c == mpr121_i2c_handle) {
		mpr121_read_status = HAL_OK;
		__sync_synchronize();
	}
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
	if(hi2c == mpr121_i2c_handle) {
		mpr121_read_status = HAL_ERROR;
		__sync_synchronize();
	}
}

void mpr121_read_registers(uint8_t address, void * value, size_t size) {
	mpr121_read_status = HAL_BUSY;

	cu_verify(HAL_I2C_Mem_Read_IT(mpr121_i2c_handle, MPR121_ADDRESS, address,
			I2C_MEMADD_SIZE_8BIT, value, (uint16_t)size) == HAL_OK);
	while(mpr121_read_status == HAL_BUSY) {
		__sync_synchronize();
	}
}

void mpr121_auto_configure(void) {
	//mpr121_write_register(MPR121_MAP_EC, 0x00);
	mpr121_write_register(MPR121_MAP_EC, 0x04); // Enable 4 sensors
	mpr121_write_register(MPR121_MAP_MHDR, 0x01);
	mpr121_write_register(MPR121_MAP_NHDR, 0x01);
	mpr121_write_register(MPR121_MAP_NCLR, 0x00);
	mpr121_write_register(MPR121_MAP_FDLR, 0x00);
	mpr121_write_register(MPR121_MAP_MHDF, 0x01);
	mpr121_write_register(MPR121_MAP_NHDF, 0x01);
	mpr121_write_register(MPR121_MAP_NCLF, 0xFF);
	mpr121_write_register(MPR121_MAP_FDLF, 0x02);
	for(uint8_t i = 0; i < 12; ++i) {
		mpr121_write_register((uint8_t)(MPR121_MAP_EXTTH + i * 2),
				MPR121_THRESH_TOUCH);
		mpr121_write_register((uint8_t)(MPR121_MAP_EXRTH + i * 2),
				MPR121_THRESH_RELEASE);
	}
	mpr121_write_register(MPR121_MAP_AFEC2, 0x20);
	mpr121_write_register(MPR121_MAP_ACUSL, 0xC9);  // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V
	mpr121_write_register(MPR121_MAP_ACLSL, 0x82);  // LSL = 0.65*USL = 0x82 @3.3V
	mpr121_write_register(MPR121_MAP_ACTL, 0xB5);  // Target = 0.9*USL = 0xB5 @3.3V
	mpr121_write_register(MPR121_MAP_ACCR0, 0x0B);

	HAL_Delay(50);

	//mpr121_write_register(MPR121_MAP_EC, 0x04); // Enable 4 sensors
}

void mpr121_set_thresholds(uint8_t start, uint8_t * thresholds,
		size_t count) {
	mpr121_write_registers((uint8_t)(MPR121_MAP_EXTTH + start), thresholds,
			count);
}

uint16_t mpr121_get_touch_states(void) {
	uint16_t states = 0xFEFE;
	mpr121_read_registers(MPR121_MAP_EXTS, &states, sizeof states);
	return states;
}

void mpr121_get_analog_baselines(uint8_t start, uint16_t * baselines,
		size_t count) {
	memset(baselines, 0xFE, count * sizeof (baselines[0]));
	mpr121_read_registers((uint8_t)(MPR121_MAP_EXBV + start * 2), baselines,
			count * sizeof (baselines[0]));
}

void mpr121_get_analog_values(uint8_t start, uint16_t * values,
		size_t count) {
	memset(values, 0xFE, count * sizeof (values[0]));
	mpr121_read_registers((uint8_t)(MPR121_MAP_EXFDL + start * 2), values,
			count * sizeof (values[0]));
}

void ws2801_init(void) {
	ws2801_spi = hw_spi_assign(HWR_SPI1, HWR_PA5, HWR_NONE, HWR_PA7,
			HWR_NONE, HWR_NONE);
			//HWR_DMA2_STREAM3, HWR_NONE);
}

void ws2801_output(void const * buf, size_t buf_len) {
	hw_spi_transmit(ws2801_spi, buf, buf_len);
	HAL_Delay(2);
}

void ws2811_init(void) {
	ws2811_data_buf = NULL;
	ws2811_reset_counter = 0;
	__sync_synchronize();

	ws2811_i2s = hw_i2s_assign(HWR_SPII2S2, HWR_PB13, HWR_PB12, HWR_PB15,
			HWR_NONE, HWR_DMA1_STREAM4);
	hw_i2s_start_output(ws2811_i2s,
			WS2811_BIT_RATE * WS2811_CLOCKS_PER_BIT / 32,
			HWI2SF_16B | HWI2SF_LSB_JUSTIFIED, ws2811_buf, sizeof ws2811_buf,
			ws2811_fill);
}

void ws2811_output_nb(void const * buf, size_t buf_len) {
	__disable_irq();
	ws2811_data_buf = NULL;
	if(ws2811_reset_counter < WS2811_RESET_CLOCKS) {
		ws2811_reset_counter -= WS2811_RESET_CLOCKS;
	}
	__sync_synchronize();
	__enable_irq();

	ws2811_data_buf_len = buf_len;
	ws2811_data_buf_sent = 0;
	__sync_synchronize();

	ws2811_data_buf = (uint8_t const *)buf;
	__sync_synchronize();
}

bool ws2811_get_outputting(void) {
	__sync_synchronize();
	return ws2811_data_buf != NULL;
}

void ws2811_fill(void * buf, size_t buf_len) {
	uint32_t * buf32 = (uint32_t *)buf;
	size_t i = 0;
	uint8_t const * data = ws2811_data_buf;
	size_t data_len = ws2811_data_buf_len;
	size_t data_sent = ws2811_data_buf_sent;
	int32_t reset_counter = ws2811_reset_counter;

	cu_verify(buf_len % 4 == 0);

	while(i < buf_len) {
		if((reset_counter < 0) || (data == NULL)) {
			*(buf32++) = 0;
			reset_counter += 32;
			if(reset_counter > WS2811_RESET_CLOCKS) {
				reset_counter = WS2811_RESET_CLOCKS;
			}
			i += 4;
		} else if(data_sent < data_len) {
			uint8_t d = data[data_sent];
			uint32_t accum = 0x88888888;

			if(d & 0x80) accum |= 0x00006000;
			if(d & 0x40) accum |= 0x00000600;
			if(d & 0x20) accum |= 0x00000060;
			if(d & 0x10) accum |= 0x00000006;
			if(d & 0x08) accum |= 0x60000000;
			if(d & 0x04) accum |= 0x06000000;
			if(d & 0x02) accum |= 0x00600000;
			if(d & 0x01) accum |= 0x00060000;
			*(buf32++) = accum;
			++data_sent;
			i += 4;

			if(data_sent >= data_len) {
				data = NULL;
				ws2811_data_buf = NULL;
			}

			reset_counter = 0;
		}
	}

	ws2811_reset_counter = reset_counter;
	ws2811_data_buf_sent = data_sent;
}
