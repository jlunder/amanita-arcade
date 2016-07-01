/*
 * hardware.c
 *
 *  Created on: Jun 27, 2016
 *      Author: jlunder
 */

#include "hardware.h"

#include "hardware_internal.h"

static hw_timer_struct_t hw_tim_1; // 168MHz, 16bit
static hw_timer_struct_t hw_tim_2; // 84MHz, 32bit
static hw_timer_struct_t hw_tim_3; // 84MHz, 16bit
static hw_timer_struct_t hw_tim_4; // 84MHz, 16bit
static hw_timer_struct_t hw_tim_5; // 84MHz, 32bit
static hw_timer_struct_t hw_tim_6; // 84MHz, 16bit
static hw_timer_struct_t hw_tim_7; // 84MHz, 16bit
static hw_timer_struct_t hw_tim_8; // 168MHz, 16bit
static hw_timer_struct_t hw_tim_9; // 168MHz, 16bit
static hw_timer_struct_t hw_tim_10; // 168MHz, 16bit
static hw_timer_struct_t hw_tim_11; // 168MHz, 16bit
static hw_timer_struct_t hw_tim_12; // 84MHz, 16bit

hw_assignment_id_t hw_timer_assign(hw_resource_id_t timer) {
	hw_assignment_id_t id;
	hw_timer_struct_t * tims = NULL;

	cu_verify(timer > HWR_NONE && timer < HWR_COUNT);
	cu_verify(hw_resource_definitions[timer].type == HWT_TIM);

	switch(timer) {
	case HWR_TIM1:
		tims = &hw_tim_1;
		tims->frequency = 168000000;
		tims->tim_handle.Instance = TIM1;
		break;
	case HWR_TIM2:
		tims = &hw_tim_2;
		tims->frequency = 84000000;
		tims->tim_handle.Instance = TIM2;
		break;
	case HWR_TIM3:
		tims = &hw_tim_3;
		tims->frequency = 84000000;
		tims->tim_handle.Instance = TIM3;
		break;
	case HWR_TIM4:
		tims = &hw_tim_4;
		tims->frequency = 84000000;
		tims->tim_handle.Instance = TIM4;
		break;
	case HWR_TIM5:
		tims = &hw_tim_5;
		tims->frequency = 84000000;
		tims->tim_handle.Instance = TIM5;
		break;
	case HWR_TIM6:
		tims = &hw_tim_6;
		tims->frequency = 84000000;
		tims->tim_handle.Instance = TIM6;
		break;
	case HWR_TIM7:
		tims = &hw_tim_7;
		tims->frequency = 84000000;
		tims->tim_handle.Instance = TIM7;
		break;
	case HWR_TIM8:
		tims = &hw_tim_8;
		tims->frequency = 168000000;
		tims->tim_handle.Instance = TIM8;
		break;
	case HWR_TIM9:
		tims = &hw_tim_9;
		tims->frequency = 168000000;
		tims->tim_handle.Instance = TIM9;
		break;
	case HWR_TIM10:
		tims = &hw_tim_10;
		tims->frequency = 168000000;
		tims->tim_handle.Instance = TIM10;
		break;
	case HWR_TIM11:
		tims = &hw_tim_11;
		tims->frequency = 168000000;
		tims->tim_handle.Instance = TIM11;
		break;
	case HWR_TIM12:
		tims = &hw_tim_12;
		tims->frequency = 84000000;
		tims->tim_handle.Instance = TIM12;
		break;
	default:
		cu_error("timer does not identify a TIM peripheral");
		break;
	}

	id = hw_resource_assign(timer, (intptr_t)tims);
	for(size_t i = 0; i < 4; ++i) {
		tims->pwm_channels[i] = HW_ASSIGNMENT_ID_NULL;
	}

	hw_clock_enable(hw_resource_definitions[timer].clock_id);

	return id;
}

void hw_timer_deassign(hw_assignment_id_t id) {
	hw_resource_deassign(id);
}

uint32_t hw_timer_get_frequency(hw_assignment_id_t id) {
	hw_timer_struct_t * tims = (hw_timer_struct_t *)hw_resource_get_user(id);
	return tims->frequency;
}

void hw_timer_configure_clock(hw_assignment_id_t id, uint32_t resolution,
		uint32_t rollover) {
	hw_timer_struct_t * tims = (hw_timer_struct_t *)hw_resource_get_user(id);
	tims->tim_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tims->tim_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
	tims->tim_handle.Init.Period = rollover;
	tims->tim_handle.Init.Prescaler = resolution;
	tims->tim_handle.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&tims->tim_handle);
	HAL_TIM_Base_Start(&tims->tim_handle);
}

uint32_t hw_timer_read(hw_assignment_id_t id) {
	hw_timer_struct_t * tims = (hw_timer_struct_t *)hw_resource_get_user(id);
	return tims->tim_handle.Instance->CNT;
}

//void hw_timer_configure_periodic_interrupt(hw_assignment_id_t id, uint32_t rate);
