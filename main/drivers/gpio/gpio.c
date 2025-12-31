#include "gpio.h"

void GPIO_Init(){
    gpio_config_t gpio_cfg = {
        .pin_bit_mask = ((1ULL << GPIO_L0_MINUS_PIN) | (1ULL << GPIO_L0_PLUS_PIN)),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&gpio_cfg));
}