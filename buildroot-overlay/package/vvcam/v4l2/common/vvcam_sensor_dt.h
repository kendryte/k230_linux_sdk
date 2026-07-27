/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __VVCAM_SENSOR_DT_H__
#define __VVCAM_SENSOR_DT_H__

#include <linux/of.h>
#include <linux/errno.h>
#include <linux/kernel.h>

#ifndef VVCAM_DT_MAX_SENSORS
#define VVCAM_DT_MAX_SENSORS 3
#endif

/**
 * Return true if DT node mipi.N exists and is available (status okay).
 */
static inline bool vvcam_mipi_slot_enabled(unsigned int slot)
{
	char name[16];
	struct device_node *mipi_np;
	bool enabled;

	snprintf(name, sizeof(name), "mipi.%u", slot);
	mipi_np = of_find_node_by_name(NULL, name);
	if (!mipi_np)
		return false;

	enabled = of_device_is_available(mipi_np);
	of_node_put(mipi_np);
	return enabled;
}

/**
 * Count continuous enabled sensors starting at N=0.
 * A slot counts only when both:
 *   - devN-sensor-name exists on v4l2isp, and
 *   - mipi.N is present and status=okay
 * Stops at the first missing name or disabled mipi.
 * Returns 1..VVCAM_DT_MAX_SENSORS on success, -EINVAL if none / invalid np.
 */
static inline int vvcam_count_sensors_from_dt(struct device_node *np)
{
	char prop[32];
	const char *name;
	int n;

	if (!np)
		return -EINVAL;

	for (n = 0; n < VVCAM_DT_MAX_SENSORS; n++) {
		snprintf(prop, sizeof(prop), "dev%d-sensor-name", n);
		if (of_property_read_string(np, prop, &name))
			break;
		if (!vvcam_mipi_slot_enabled(n))
			break;
	}

	if (n == 0)
		return -EINVAL;

	return n;
}

static inline struct device_node *vvcam_find_v4l2isp_node(void)
{
	return of_find_compatible_node(NULL, NULL, "verisilicon,isp-v4l2");
}

#endif /* __VVCAM_SENSOR_DT_H__ */
