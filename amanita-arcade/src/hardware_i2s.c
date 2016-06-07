/*
 * hardware_i2s.c
 *
 *  Created on: Jun 4, 2016
 *      Author: jlunder
 */

#include "hardware.h"

#include "hardware_internal.h"

static size_t hw_clock_i2s_pll_enable_count = 0;

static hw_i2s_struct_t hw_i2s_2;
static hw_i2s_struct_t hw_i2s_3;

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
				sclk_pin == HWR_PI1 || sclk_pin == HWR_NONE);
		cu_verify(ws_pin == HWR_PB9 || ws_pin == HWR_PB12 ||
				ws_pin == HWR_PI0 || ws_pin == HWR_NONE);
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
		cu_verify(sclk_pin == HWR_PB3 || sclk_pin == HWR_PC10 ||
				sclk_pin == HWR_NONE);
		cu_verify(ws_pin == HWR_PA4 || ws_pin == HWR_PA15 ||
				ws_pin == HWR_NONE);
		cu_verify(sd_pin == HWR_PB5 || sd_pin == HWR_PC12);
		cu_verify(mclk_pin == HWR_NONE || mclk_pin == HWR_PC7);
		switch(dma_stream) {
		case HWR_DMA1_STREAM5: dma_channel = DMA_CHANNEL_0; break;
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
	i2ss->mclk = HW_ASSIGNMENT_ID_NULL;
	if(sclk_pin != HWR_NONE) {
		i2ss->sclk = hw_pin_assign(sclk_pin);
	}
	i2ss->mclk = HW_ASSIGNMENT_ID_NULL;
	if(ws_pin != HWR_NONE) {
		i2ss->ws = hw_pin_assign(ws_pin);
	}
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
		if(i2ss->sclk != HW_ASSIGNMENT_ID_NULL) {
			hw_pin_configure(i2ss->sclk, HWPM_AF_PP | HWPM_ALT_5);
		}
		if(i2ss->ws != HW_ASSIGNMENT_ID_NULL) {
			hw_pin_configure(i2ss->ws, HWPM_AF_PP | HWPM_ALT_5);
		}
		hw_pin_configure(i2ss->sd, HWPM_AF_PP | HWPM_ALT_5);
		if(i2ss->mclk != HW_ASSIGNMENT_ID_NULL) {
			hw_pin_configure(i2ss->mclk, HWPM_AF_PP | HWPM_ALT_5);
		}
		break;
	case HWR_SPII2S3:
		if(i2ss->sclk != HW_ASSIGNMENT_ID_NULL) {
			hw_pin_configure(i2ss->sclk, HWPM_AF_PP | HWPM_ALT_6);
		}
		if(i2ss->ws != HW_ASSIGNMENT_ID_NULL) {
			hw_pin_configure(i2ss->ws, HWPM_AF_PP | HWPM_ALT_6);
		}
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

void hw_i2s_start_output(hw_assignment_id_t id, uint32_t rate,
		hw_i2s_format_t format, void * buf, size_t buf_len,
		hw_i2s_fill_func_t fill_func) {
	hw_i2s_struct_t * i2ss = (hw_i2s_struct_t *)hw_resource_get_user(id);
	uint16_t tx_size;

	cu_verify(fill_func != NULL);
	cu_verify(buf_len < 0x10000);

	i2ss->i2s_handle.Init.AudioFreq = rate;
	switch(format & HWI2SF_FRAME_SIZE_MASK) {
	case HWI2SF_16B: i2ss->i2s_handle.Init.DataFormat =
			I2S_DATAFORMAT_16B; break;
	case HWI2SF_16B_EXT_32B: i2ss->i2s_handle.Init.DataFormat =
			I2S_DATAFORMAT_16B_EXTENDED; break;
	case HWI2SF_24B_EXT_32B: i2ss->i2s_handle.Init.DataFormat =
			I2S_DATAFORMAT_24B; break;
	case HWI2SF_32B: i2ss->i2s_handle.Init.DataFormat =
			I2S_DATAFORMAT_32B; break;
	default:
		cu_error("invalid frame size?");
		break;
	}
	switch(format & HWI2SF_STANDARD_MASK) {
	case HWI2SF_LSB_JUSTIFIED: i2ss->i2s_handle.Init.Standard =
			I2S_STANDARD_LSB; break;
	case HWI2SF_MSB_JUSTIFIED: i2ss->i2s_handle.Init.Standard =
			I2S_STANDARD_MSB; break;
	case HWI2SF_PCM_SHORT: i2ss->i2s_handle.Init.Standard =
			I2S_STANDARD_PCM_SHORT; break;
	case HWI2SF_PCM_LONG: i2ss->i2s_handle.Init.Standard =
			I2S_STANDARD_PCM_LONG; break;
	case HWI2SF_PHILIPS: i2ss->i2s_handle.Init.Standard =
			I2S_STANDARD_PHILIPS; break;
	default:
		cu_error("invalid signaling standard?");
		break;
	}
	i2ss->fill_func = fill_func;
	i2ss->buf = buf;
	i2ss->buf_len = buf_len;

	switch(format & HWI2SF_FRAME_SIZE_MASK) {
	case HWI2SF_24B_EXT_32B:
	case HWI2SF_32B:
		i2ss->dma_handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		i2ss->dma_handle->Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
		tx_size = (uint16_t)(buf_len / sizeof (uint32_t));
		break;
	default:
		i2ss->dma_handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		i2ss->dma_handle->Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
		tx_size = (uint16_t)(buf_len / sizeof (uint16_t));
		break;
	}

	hw_irq_enable(
			hw_resource_definitions[hw_resource_get_resource_id(id)].irq_id);
	hw_irq_enable(hw_resource_definitions[
			hw_resource_get_resource_id(i2ss->dma_stream)].irq_id);

	cu_verify(HAL_DMA_Init(i2ss->dma_handle) == HAL_OK);
	cu_verify(HAL_I2S_Init(&i2ss->i2s_handle) == HAL_OK);

	fill_func(buf, buf_len / 2);
	fill_func((uint8_t *)buf + buf_len / 2, buf_len / 2);

	cu_verify(HAL_I2S_Transmit_DMA(&i2ss->i2s_handle, buf, tx_size) ==
			HAL_OK);
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

