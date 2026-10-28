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

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include "sensors/ei_inertial.h"

LOG_MODULE_REGISTER(inertial_sensor);

const struct device *const lsm6dso = DEVICE_DT_GET_ONE(st_lsm6dso);

static bool _is_present = false;

static bool lsm6dso_config(const struct device *lsm6dso)
{
	struct sensor_value odr_attr, fs_attr;
	int ret;

	/* set LSM6DSO accel sampling frequency to 208 Hz */
	odr_attr.val1 = 208;
	odr_attr.val2 = 0;

	ret = sensor_attr_set(lsm6dso, SENSOR_CHAN_ACCEL_XYZ,
			    SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr);
	if (ret < 0) {
		LOG_ERR("lsm6dso: failed to set %s (%d)", "accel sampling frequency", ret);
		return false;
	}

	sensor_g_to_ms2(16, &fs_attr);

	ret = sensor_attr_set(lsm6dso, SENSOR_CHAN_ACCEL_XYZ,
			    SENSOR_ATTR_FULL_SCALE, &fs_attr);
	if (ret < 0) {
		LOG_ERR("lsm6dso: failed to set %s (%d)", "accel full scale", ret);
		return false;
	}

	/* set LSM6DSO gyro sampling frequency to 208 Hz */
	odr_attr.val1 = 208;
	odr_attr.val2 = 0;

	ret = sensor_attr_set(lsm6dso, SENSOR_CHAN_GYRO_XYZ,
			    SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr);
	if (ret < 0) {
		LOG_ERR("lsm6dso: failed to set %s (%d)", "gyro sampling frequency", ret);
		return false;
	}

	sensor_degrees_to_rad(250, &fs_attr);

	ret = sensor_attr_set(lsm6dso, SENSOR_CHAN_GYRO_XYZ,
			    SENSOR_ATTR_FULL_SCALE, &fs_attr);
	if (ret < 0) {
		LOG_ERR("lsm6dso: failed to set %s (%d)", "gyro full scale", ret);
		return false;
	}

	return true;
}

bool ei_inertial_init(void)
{
	if (!device_is_ready(lsm6dso)) {
		LOG_ERR("lsm6dso is not ready");
		return false;
	}

	_is_present = true;
	if (!lsm6dso_config(lsm6dso)) {
		LOG_ERR("lsm6dso: configuration failed");
		return false;
	}

	if(ei_add_sensor_to_fusion_list(inertial_sensor) == false) {
        LOG_ERR("ERR: failed to register inertial sensor!");
        return false;
    }

	return true;
}

static float inertial[INERTIAL_AXIS_SAMPLED];

float *ei_fusion_inertial_read_data(int n_samples)
{
    struct sensor_value accel[3];
    struct sensor_value gyro[3];
    
    memset(inertial, 0, INERTIAL_AXIS_SAMPLED * sizeof(float));

    int ret = sensor_sample_fetch(lsm6dso);
    if (ret < 0) {
        LOG_ERR("lsm6dso: sample fetch failed (%d)", ret);
    }
    else {
        ret = sensor_channel_get(lsm6dso, SENSOR_CHAN_ACCEL_XYZ, accel);
        if (ret < 0) {
            LOG_ERR("lsm6dso: accel channel get failed (%d)", ret);
            return inertial;
        }
        ret = sensor_channel_get(lsm6dso, SENSOR_CHAN_GYRO_XYZ, gyro);
        if (ret < 0) {
            LOG_ERR("lsm6dso: gyro channel get failed (%d)", ret);
            return inertial;
        }
        inertial[0] = sensor_value_to_double(&accel[0]);
        inertial[1] = sensor_value_to_double(&accel[1]);
        inertial[2] = sensor_value_to_double(&accel[2]);
        inertial[3] = sensor_value_to_double(&gyro[0]);
        inertial[4] = sensor_value_to_double(&gyro[1]);
        inertial[5] = sensor_value_to_double(&gyro[2]);
    }

    return inertial;
}
