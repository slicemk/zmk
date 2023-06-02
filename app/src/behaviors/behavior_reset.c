/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_reset

#include <zephyr/device.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>

#include <zmk/behavior.h>
#include <dt-bindings/zmk/reset.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
struct behavior_reset_config {
    int type;
};

static int behavior_reset_init(const struct device *dev) { return 0; };

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_reset_config *cfg = dev->config;

    // TODO: Correct magic code for going into DFU?
    // See
    // https://github.com/adafruit/Adafruit_nRF52_Bootloader/blob/d6b28e66053eea467166f44875e3c7ec741cb471/src/main.c#L107
    sys_reboot(cfg->type);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_reset_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
};

#define RST_INST(n)                                                                                \
    static const struct behavior_reset_config behavior_reset_config_##n = {                        \
        .type = DT_INST_PROP(n, type)};                                                            \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_reset_init, NULL, NULL, &behavior_reset_config_##n,        \
                            APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                      \
                            &behavior_reset_driver_api);

DT_INST_FOREACH_STATUS_OKAY(RST_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */

static int sysoff_init_prekernel(const struct device *dev) {
#ifdef CONFIG_SOC_SERIES_NRF52X
    uint32_t const gpregret = NRF_POWER->GPREGRET;
    if (gpregret == RST_SYSOFF) {
        NRF_POWER->GPREGRET = 0;
        NRF_POWER->SYSTEMOFF = 1;
    }
#endif
    return 0;
}

SYS_INIT(sysoff_init_prekernel, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
