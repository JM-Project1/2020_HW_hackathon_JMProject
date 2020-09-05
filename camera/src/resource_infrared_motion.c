/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <peripheral_io.h>
#include <stdlib.h>

#include "resource/resource_infrared_motion.h"
#include "log.h"

static peripheral_gpio_h g_sensor_h = NULL;
static int g_pin_num = -1;
static resource_infrared_motion_interrupted_data *g_interrupted_data = NULL;

int _resource_validate_infrared_motion(int pin_num)
{
	int ret = MOTION_HANDLE_ERROR_NONE;

	if (!g_sensor_h)
	{
		ret = MOTION_HANDLE_ERROR_NOT_OPEN;
	} else if (g_pin_num != pin_num) {
		ret = MOTION_HANDLE_ERROR_INVALID_PIN;
	}

	return ret;
}

int resource_open_infrared_motion(int pin_num)
{
	peripheral_gpio_h temp = NULL;

	int ret = peripheral_gpio_open(pin_num, &temp);
	if (ret) {
		peripheral_gpio_close(temp);
		_E("peripheral_gpio_open failed.");
		return -1;
	}

	ret = peripheral_gpio_set_direction(temp, PERIPHERAL_GPIO_DIRECTION_IN);
	if (ret) {
		peripheral_gpio_close(temp);
		_E("peripheral_gpio_set_direction failed.");
		return -1;
	}

	g_sensor_h = temp;
	g_pin_num = pin_num;

	return 0;
}

int resource_read_infrared_motion(int pin_num, uint32_t *out_value)
{
	int ret = PERIPHERAL_ERROR_NONE;

	if (_resource_validate_infrared_motion(pin_num) == MOTION_HANDLE_ERROR_NOT_OPEN) {
		ret = resource_open_infrared_motion(pin_num);
		retv_if(ret < 0, -1);
	}
	if (_resource_validate_infrared_motion(pin_num) == MOTION_HANDLE_ERROR_INVALID_PIN) {
		_E("Invalid pin number.");
		return -1;
	}

	ret = peripheral_gpio_read(g_sensor_h, out_value);
	retv_if(ret < 0, -1);

	return 0;
}

void resource_close_infrared_motion(void)
{
	if (!g_sensor_h) return;

	_I("Infrared Motion Sensor is finishing...");

	peripheral_gpio_close(g_sensor_h);

	if (g_interrupted_data != NULL) {
		free(g_interrupted_data);
		g_interrupted_data = NULL;
	}
	g_sensor_h = NULL;
	g_pin_num = -1;
}

void _resoucre_motion_interrupted_cb (peripheral_gpio_h gpio, peripheral_error_e error, void *user_data)
{
	int ret = PERIPHERAL_ERROR_NONE;

	if (!g_sensor_h) return;

	if (g_interrupted_data->is_called_first_time) {
		ret = peripheral_gpio_read(g_sensor_h, &g_interrupted_data->motion_value);
		if (ret) return;

		g_interrupted_data->is_called_first_time = 0;
	} else {
		// toggle g_interrupted_data->motion_value
		g_interrupted_data->motion_value = !g_interrupted_data->motion_value;
	}

	g_interrupted_data->interrupted_cb(g_interrupted_data->motion_value, g_interrupted_data->interrupted_cb_user_data);
}

int resource_set_interrupted_cb_infrared_motion(int pin_num, resource_infrared_motion_interrupted_cb interrupted_cb, void *user_data)
{
	int ret = PERIPHERAL_ERROR_NONE;
	if (g_interrupted_data == NULL) {
		g_interrupted_data = calloc(1, sizeof(resource_infrared_motion_interrupted_data));
	}

	if (_resource_validate_infrared_motion(pin_num) == MOTION_HANDLE_ERROR_NOT_OPEN) {
		ret = resource_open_infrared_motion(pin_num);
		retv_if(ret < 0, -1);
	}
	if (_resource_validate_infrared_motion(pin_num) == MOTION_HANDLE_ERROR_INVALID_PIN) {
		_E("Invalid pin number.");
		return -1;
	}

	g_interrupted_data->interrupted_cb = interrupted_cb;
	g_interrupted_data->interrupted_cb_user_data = user_data;
	g_interrupted_data->motion_value = 0;
	g_interrupted_data->is_called_first_time = 1;

	ret = peripheral_gpio_set_edge_mode(g_sensor_h, PERIPHERAL_GPIO_EDGE_BOTH);
	retv_if(ret < 0, -1);

	ret = peripheral_gpio_set_interrupted_cb(g_sensor_h, _resoucre_motion_interrupted_cb, g_interrupted_data);
	retv_if(ret < 0, -1);

	return 0;
}

int resource_unset_interrupted_cb_infrared_motion(int pin_num)
{
	int ret = PERIPHERAL_ERROR_NONE;

	if (g_interrupted_data != NULL) {
		free(g_interrupted_data);
		g_interrupted_data = NULL;
	}

	if (_resource_validate_infrared_motion(pin_num) == MOTION_HANDLE_ERROR_NOT_OPEN) {
		_E("No open handle.");
		return -1;
	}
	else if (_resource_validate_infrared_motion(pin_num) == MOTION_HANDLE_ERROR_INVALID_PIN) {
		_E("Invalid pin number.");
		return -1;
	}

	ret = peripheral_gpio_unset_interrupted_cb(g_sensor_h);
	retv_if(ret < 0, -1);

	return 0;
}
