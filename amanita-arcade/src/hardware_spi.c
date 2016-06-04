/*
 * hardware_spi.h
 *
 *  Created on: Jun 4, 2016
 *      Author: jlunder
 */

#include "hardware.h"

#include "hardware_internal.h"

void SPI1_IRQHandler(void);
void SPI2_IRQHandler(void);
void SPI3_IRQHandler(void);

static hw_spi_struct_t hw_spi_1;
static hw_spi_struct_t hw_spi_2;
static hw_spi_struct_t hw_spi_3;

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
	} else {
		spis->tx_dma_handle = NULL;
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
	} else {
		spis->rx_dma_handle = NULL;
	}

	spis->spi_handle.Init.Mode = SPI_MODE_MASTER;
	spis->spi_handle.Init.Direction = SPI_DIRECTION_2LINES;
	spis->spi_handle.Init.DataSize = SPI_DATASIZE_8BIT;
	spis->spi_handle.Init.CLKPolarity = SPI_POLARITY_LOW;
	spis->spi_handle.Init.CLKPhase = SPI_PHASE_1EDGE;
	spis->spi_handle.Init.NSS = SPI_NSS_SOFT;
	spis->spi_handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
	spis->spi_handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
	spis->spi_handle.Init.TIMode = SPI_TIMODE_DISABLE;
	spis->spi_handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	spis->spi_handle.Init.CRCPolynomial = SPI_CRCPR_CRCPOLY;
	spis->spi_handle.hdmatx = spis->tx_dma_handle;
	spis->spi_handle.hdmarx = spis->rx_dma_handle;

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
	hw_irq_enable(hw_resource_definitions[spi].irq_id);

	if(spis->tx_dma_handle != NULL) {
		hw_clock_enable(hw_resource_definitions[tx_dma].clock_id);
		hw_irq_enable(hw_resource_definitions[tx_dma].irq_id);
		cu_verify(HAL_DMA_Init(spis->tx_dma_handle) == HAL_OK);
	}
	if(spis->rx_dma_handle != NULL) {
		hw_clock_enable(hw_resource_definitions[rx_dma].clock_id);
		hw_irq_enable(hw_resource_definitions[rx_dma].irq_id);
		cu_verify(HAL_DMA_Init(spis->rx_dma_handle) == HAL_OK);
	}
	cu_verify(HAL_SPI_Init(&spis->spi_handle) == HAL_OK);

	return id;
}

void hw_spi_deassign(hw_assignment_id_t id) {
	hw_spi_struct_t * spis = (hw_spi_struct_t *)hw_resource_get_user(id);

	hw_clock_disable(hw_resource_definitions[hw_resource_get_resource_id(id)].clock_id);

	cu_verify(HAL_SPI_DeInit(&spis->spi_handle) == HAL_OK);
	spis->spi_handle.hdmatx = NULL;
	spis->spi_handle.hdmarx = NULL;

	if(spis->tx_dma_stream != HW_ASSIGNMENT_ID_NULL) {
		cu_verify(HAL_DMA_DeInit(spis->tx_dma_handle) == HAL_OK);
		hw_clock_disable(hw_resource_definitions[
				hw_resource_get_resource_id(spis->tx_dma_stream)].clock_id);
		hw_irq_disable(hw_resource_definitions[
				hw_resource_get_resource_id(spis->tx_dma_stream)].irq_id);
		hw_resource_deassign(spis->tx_dma_stream);
		spis->tx_dma_stream = HW_ASSIGNMENT_ID_NULL;
		spis->tx_dma_handle->Parent = NULL;
		spis->tx_dma_handle = NULL;
	}

	if(spis->rx_dma_stream != HW_ASSIGNMENT_ID_NULL) {
		cu_verify(HAL_DMA_DeInit(spis->rx_dma_handle) == HAL_OK);
		hw_clock_disable(hw_resource_definitions[
				hw_resource_get_resource_id(spis->rx_dma_stream)].clock_id);
		hw_irq_disable(hw_resource_definitions[
				hw_resource_get_resource_id(spis->rx_dma_stream)].irq_id);
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

void hw_spi_transmit(hw_assignment_id_t id, void * buf, size_t buf_len) {
	hw_spi_struct_t * spis = (hw_spi_struct_t *)hw_resource_get_user(id);

	/*
	if(spis->tx_dma_handle != NULL) {
		cu_verify(HAL_SPI_Transmit_DMA(&spis->spi_handle, buf,
				(uint16_t)buf_len) == HAL_OK);
	} else {
		cu_verify(HAL_SPI_Transmit_IT(&spis->spi_handle, buf,
				(uint16_t)buf_len) == HAL_OK);
	}
	*/
	cu_verify(HAL_SPI_Transmit(&spis->spi_handle, buf,
			(uint16_t)buf_len, 100) == HAL_OK);
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

