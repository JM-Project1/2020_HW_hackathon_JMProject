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

#include "resource/resource_led.h"
#include "log.h"

static peripheral_gpio_h g_sensor_h = NULL;
static int g_pin_num = -1;

int _resource_validate_led(int pin_num)
{
	int ret = LED_HANDLE_ERROR_NONE;

	if (!g_sensor_h)
	{
		ret = LED_HANDLE_ERROR_NOT_OPEN;
	} else if (g_pin_num != pin_num) {
		ret = LED_HANDLE_ERROR_INVALID_PIN;
	}

	return ret;
}

int resource_open_led(int pin_num)
{
	peripheral_gpio_h temp = NULL;

	int ret = peripheral_gpio_open(pin_num, &temp);
	if (ret) {
		peripheral_gpio_close(temp);
		_E("peripheral_gpio_open failed.");
		return -1;
	}

	ret = peripheral_gpio_set_direction(temp, PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	if (ret) {
		peripheral_gpio_close(temp);
		_E("peripheral_gpio_set_direction failed.");
		return -1;
	}

	g_sensor_h = temp;
	g_pin_num = pin_num;

	return 0;
}

void resource_close_led(void)
{
	if (!g_sensor_h) return;

	_I("LED is finishing...");

	peripheral_gpio_close(g_sensor_h);

	g_sensor_h = NULL;
	g_pin_num = -1;
}

int resource_write_led(int pin_num, int write_value)
{
	int ret = PERIPHERAL_ERROR_NONE;

	if (_resource_validate_led(pin_num) == LED_HANDLE_ERROR_NOT_OPEN) {
		ret = resource_open_led(pin_num);
		retv_if(ret < 0, -1);
	}
	if (_resource_validate_led(pin_num) == LED_HANDLE_ERROR_INVALID_PIN) {
		_E("Invalid pin number.");
		return -1;
	}

	ret = peripheral_gpio_write(g_sensor_h, write_value);
	retv_if(ret < 0, -1);

	return 0;
}
