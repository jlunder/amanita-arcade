/*
 * hardware_i2c.c
 *
 *  Created on: Jun 4, 2016
 *      Author: jlunder
 */

#include "hardware.h"

#include "hardware_internal.h"

static hw_i2c_struct_t hw_i2c_1;
static hw_i2c_struct_t hw_i2c_2;
static hw_i2c_struct_t hw_i2c_3;

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

	i2cs->i2c_handle.Init.ClockSpeed = 40000;
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

