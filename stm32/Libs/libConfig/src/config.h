#ifndef CONFIG_H
#define CONFIG_H

#include "stm32g0xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define CONFIG_MAGIC        0x3634444FUL
#define CONFIG_FLASH_PAGE   ((FLASH_SIZE / FLASH_PAGE_SIZE) - 1U)
#define CONFIG_FLASH_ADDR   (FLASH_BASE + CONFIG_FLASH_PAGE * FLASH_PAGE_SIZE)

#define CONFIG_NUM_BAUDS    11U
#define CONFIG_NUM_REGS     5U

extern const uint32_t config_baud_table[CONFIG_NUM_BAUDS];

typedef struct {
    uint32_t magic;
    uint8_t  slave_addr;  /* 1–247                         */
    uint8_t  baud_code;   /* 0–10 → config_baud_table      */
    uint8_t  parity;      /* 0=none, 1=even, 2=odd         */
    uint8_t  data_bits;   /* 5–9                           */
    uint8_t  stop_bits;   /* 1–2                           */
    uint8_t  _pad[7];     /* pad to 16 bytes (2 doublewords) */
} Config;

Config   config_defaults(void);
Config   config_load(void);
bool     config_save(const Config *cfg);
void     config_apply(const Config *cfg, UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim);
bool     config_validate_reg(uint16_t addr, uint16_t value);
void     config_set_reg(Config *cfg, uint16_t addr, uint16_t value);
uint16_t config_get_reg(const Config *cfg, uint16_t addr);

#endif /* CONFIG_H */
