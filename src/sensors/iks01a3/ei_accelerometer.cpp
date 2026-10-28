/* The Clear BSD License
 *
 * Copyright (c) 2026 EdgeImpulse Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 *   * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* Include ----------------------------------------------------------------- */
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include "sensors/ei_accelerometer.h"

LOG_MODULE_REGISTER(acc_sensor);

/* Constant defines -------------------------------------------------------- */
const struct device *lis2dw12;

static bool lis2dw12_config(const struct device *lis2dw12)
{
	struct sensor_value odr_attr, fs_attr;
	int ret;

	/* set LIS2DW12 accel/gyro sampling frequency to 200 Hz */
	odr_attr.val1 = 200;
	odr_attr.val2 = 0;

	ret = sensor_attr_set(lis2dw12, SENSOR_CHAN_ACCEL_XYZ,
			    SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr);
	if (ret < 0) {
		LOG_ERR("lis2dw12: failed to set %s (%d)", "sampling frequency", ret);
		return false;
	}

	sensor_g_to_ms2(2, &fs_attr);

	ret = sensor_attr_set(lis2dw12, SENSOR_CHAN_ACCEL_XYZ,
			    SENSOR_ATTR_FULL_SCALE, &fs_attr);
	if (ret < 0) {
		LOG_ERR("lis2dw12: failed to set %s (%d)", "full scale", ret);
		return false;
	}

	return true;
}

bool ei_accelerometer_init(void)
{
    lis2dw12 = DEVICE_DT_GET_ONE(st_lis2dw12);
    if (!device_is_ready(lis2dw12))
    {
        LOG_ERR("lis2dw12 is not ready");
        return false;
    }

    if (!lis2dw12_config(lis2dw12)) {
        LOG_ERR("lis2dw12: configuration failed");
        return false;
    }

    if(ei_add_sensor_to_fusion_list(accelerometer_sensor) == false) {
        LOG_ERR("ERR: failed to register inertial sensor!");
        return false;
    }

    return true;
}

static float acceleration_g[ACCEL_AXIS_SAMPLED];

float * ei_fusion_accelerometer_read_data(int n_samples)
{
    struct sensor_value accel2[ACCEL_AXIS_SAMPLED];

    memset(acceleration_g, 0, ACCEL_AXIS_SAMPLED * sizeof(float));

    int ret = sensor_sample_fetch(lis2dw12);
    if (ret < 0) {
        LOG_ERR("lis2dw12: sample fetch failed (%d)", ret);
    }
    else {
        ret = sensor_channel_get(lis2dw12, SENSOR_CHAN_ACCEL_XYZ, accel2);
        if (ret < 0) {
            LOG_ERR("lis2dw12: accel channel get failed (%d)", ret);
        }
        acceleration_g[0] = sensor_value_to_double(&accel2[0]);
        acceleration_g[1] = sensor_value_to_double(&accel2[1]);
        acceleration_g[2] = sensor_value_to_double(&accel2[2]);
    }

    return acceleration_g;
}
