#include "config.h"
#include <string.h>

const uint32_t config_baud_table[CONFIG_NUM_BAUDS] = {
    1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600
};

Config config_defaults(void)
{
    Config d = {
        .magic      = CONFIG_MAGIC,
        .slave_addr = 1,
        .baud_code  = 3,  /* 9600  */
        .parity     = 0,  /* none  */
        .data_bits  = 8,
        .stop_bits  = 2,
    };
    return d;
}

Config config_load(void)
{
    Config loaded;
    memcpy(&loaded, (const void *)CONFIG_FLASH_ADDR, sizeof(Config));
    if (loaded.magic      != CONFIG_MAGIC                        ||
        loaded.slave_addr  < 1 || loaded.slave_addr > 247        ||
        loaded.baud_code   >= CONFIG_NUM_BAUDS                   ||
        loaded.parity      > 2                                   ||
        loaded.data_bits   < 5 || loaded.data_bits  > 9          ||
        loaded.stop_bits   < 1 || loaded.stop_bits  > 2)
        return config_defaults();
    return loaded;
}

bool config_save(const Config *cfg)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase = {
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .Banks     = FLASH_BANK_1,
        .Page      = CONFIG_FLASH_PAGE,
        .NbPages   = 1,
    };
    uint32_t page_error;
    if (HAL_FLASHEx_Erase(&erase, &page_error) != HAL_OK) {
        HAL_FLASH_Lock();
        return false;
    }

    uint64_t words[2];
    memcpy(words, cfg, sizeof(Config));
    for (uint8_t i = 0; i < 2U; i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,
                              CONFIG_FLASH_ADDR + i * 8U, words[i]) != HAL_OK) {
            HAL_FLASH_Lock();
            return false;
        }
    }

    HAL_FLASH_Lock();
    return true;
}

void config_apply(const Config *cfg, UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim)
{
    uint32_t baud    = config_baud_table[cfg->baud_code];
    bool     has_par = (cfg->parity != 0);

    /* Word length: STM32 HAL includes the parity bit in the count */
    uint32_t word_len;
    switch (cfg->data_bits) {
        case 7:  word_len = has_par ? UART_WORDLENGTH_8B : UART_WORDLENGTH_7B; break;
        case 9:  word_len = UART_WORDLENGTH_9B;                                 break;
        default: word_len = has_par ? UART_WORDLENGTH_9B : UART_WORDLENGTH_8B; break;
    }

    huart->Init.BaudRate   = baud;
    huart->Init.WordLength = word_len;
    huart->Init.Parity     = (cfg->parity == 1) ? UART_PARITY_EVEN :
                              (cfg->parity == 2) ? UART_PARITY_ODD  : UART_PARITY_NONE;
    huart->Init.StopBits   = (cfg->stop_bits == 2) ? UART_STOPBITS_2 : UART_STOPBITS_1;
    HAL_UART_Init(huart);

    /* TIM period = 0.5 × char_time (tick 3 → 1.5t, tick 7 → 3.5t).
       Above 19200 baud the Modbus spec mandates fixed silent intervals:
       t1.5 = 750 µs, t3.5 = 1750 µs → period = 250 µs. */
    uint32_t period;
    if (baud > 19200) {
        period = SystemCoreClock / 4000U;  /* 250 µs */
    } else {
        uint8_t char_bits = 1U + cfg->data_bits + cfg->stop_bits + (has_par ? 1U : 0U);
        period = (SystemCoreClock * char_bits + baud) / (baud * 2U);
    }
    __HAL_TIM_SET_AUTORELOAD(htim, period);
}

bool config_validate_reg(uint16_t addr, uint16_t value)
{
    switch (addr) {
        case 0: return value >= 1 && value <= 247;
        case 1: return value < CONFIG_NUM_BAUDS;
        case 2: return value <= 2;
        case 3: return value >= 5 && value <= 9;
        case 4: return value >= 1 && value <= 2;
        default: return false;
    }
}

void config_set_reg(Config *cfg, uint16_t addr, uint16_t value)
{
    switch (addr) {
        case 0: cfg->slave_addr = (uint8_t)value; break;
        case 1: cfg->baud_code  = (uint8_t)value; break;
        case 2: cfg->parity     = (uint8_t)value; break;
        case 3: cfg->data_bits  = (uint8_t)value; break;
        case 4: cfg->stop_bits  = (uint8_t)value; break;
    }
}

uint16_t config_get_reg(const Config *cfg, uint16_t addr)
{
    switch (addr) {
        case 0: return cfg->slave_addr;
        case 1: return cfg->baud_code;
        case 2: return cfg->parity;
        case 3: return cfg->data_bits;
        case 4: return cfg->stop_bits;
        default: return 0;
    }
}
