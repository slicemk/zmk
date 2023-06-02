#include <zephyr/device.h>
#include <zephyr/sys/reboot.h>

#include <drivers/behavior.h>

#include <zmk/behavior.h>
#include <dt-bindings/zmk/gpregret2.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define DT_DRV_COMPAT zmk_behavior_gpregret2
#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct behavior_gpregret2_config {
    int type;
};

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_gpregret2_config *cfg = dev->config;
#ifdef CONFIG_SOC_SERIES_NRF52X
    NRF_POWER->GPREGRET2 = cfg->type;
    sys_reboot(SYS_REBOOT_WARM);
#endif
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_gpregret2_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
};

#define GPREGRET2_INST(n)                                                                          \
    static const struct behavior_gpregret2_config behavior_gpregret2_config_##n = {                \
        .type = DT_INST_PROP(n, type)};                                                            \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, &behavior_gpregret2_config_##n, POST_KERNEL,      \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_gpregret2_driver_api);

DT_INST_FOREACH_STATUS_OKAY(GPREGRET2_INST)

static int gpregret2_init_prekernel(void) {
#ifdef CONFIG_SOC_SERIES_NRF52X
    uint32_t const gpregret = NRF_POWER->GPREGRET2;
    if (gpregret == 0) {
        NRF_POWER->GPREGRET2 = GPREGRET2_RESET;
        sys_reboot(SYS_REBOOT_WARM);
    } else if (gpregret == GPREGRET2_RESET) {
        NRF_POWER->GPREGRET2 = 0;
    } else if (gpregret == GPREGRET2_SYSOFF) {
        NRF_POWER->GPREGRET2 = 0;
        NRF_POWER->SYSTEMOFF = 1;
    }
#endif
    return 0;
}

SYS_INIT(gpregret2_init_prekernel, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
