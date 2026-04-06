#include "mcp23s17.h"

#include <stddef.h>

/*==============================
    Internal helpers
==============================*/

/* SPI control byte: 0100[A2][A1][A0][R/W]
   R/W = 0 → write, 1 → read */
#define CTRL_WRITE(addr) ((uint8_t)(0x40 | (((addr) & 0x07) << 1)))
#define CTRL_READ(addr)  ((uint8_t)(0x41 | (((addr) & 0x07) << 1)))

static void spi_write_reg(MCP23S17 *dev, uint8_t reg, uint8_t val)
{
    uint8_t tx[3] = { CTRL_WRITE(dev->config.hw_addr), reg, val };

    dev->config.cs_assert();
    dev->config.spi_txrx(tx, NULL, sizeof(tx));
    dev->config.cs_deassert();
}

static uint8_t spi_read_reg(MCP23S17 *dev, uint8_t reg)
{
    uint8_t tx[3] = { CTRL_READ(dev->config.hw_addr), reg, 0x00 };
    uint8_t rx[3] = { 0 };

    dev->config.cs_assert();
    dev->config.spi_txrx(tx, rx, sizeof(tx));
    dev->config.cs_deassert();

    return rx[2];
}

static void spi_write_word(MCP23S17 *dev, uint8_t reg, uint16_t val)
{
    uint8_t tx[4] = {
        CTRL_WRITE(dev->config.hw_addr),
        reg,
        (uint8_t)(val & 0xFF),
        (uint8_t)(val >> 8),
    };

    dev->config.cs_assert();
    dev->config.spi_txrx(tx, NULL, sizeof(tx));
    dev->config.cs_deassert();
}

static uint16_t spi_read_word(MCP23S17 *dev, uint8_t reg)
{
    uint8_t tx[4] = { CTRL_READ(dev->config.hw_addr), reg, 0x00, 0x00 };
    uint8_t rx[4] = { 0 };

    dev->config.cs_assert();
    dev->config.spi_txrx(tx, rx, sizeof(tx));
    dev->config.cs_deassert();

    return (uint16_t)(rx[2] | ((uint16_t)rx[3] << 8));
}

/*==============================
    Public API
==============================*/

int mcp23s17_init(MCP23S17 *dev, const MCP23S17Config *cfg)
{
    if (dev == NULL || cfg == NULL) return -1;
    if (cfg->cs_assert == NULL || cfg->cs_deassert == NULL) return -1;
    if (cfg->spi_txrx == NULL) return -1;
    if (cfg->hw_addr > 7) return -1;

    dev->config = *cfg;

    /* Enable hardware address decoding so multiple devices can share the bus */
    spi_write_reg(dev, MCP23S17_IOCON, MCP23S17_IOCON_HAEN);

    return 0;
}

void mcp23s17_write_reg(MCP23S17 *dev, uint8_t reg, uint8_t val)
{
    spi_write_reg(dev, reg, val);
}

uint8_t mcp23s17_read_reg(MCP23S17 *dev, uint8_t reg)
{
    return spi_read_reg(dev, reg);
}

void mcp23s17_write_word(MCP23S17 *dev, uint8_t reg, uint16_t val)
{
    spi_write_word(dev, reg, val);
}

uint16_t mcp23s17_read_word(MCP23S17 *dev, uint8_t reg)
{
    return spi_read_word(dev, reg);
}

void mcp23s17_set_direction(MCP23S17 *dev, uint8_t porta_dir, uint8_t portb_dir)
{
    spi_write_word(dev, MCP23S17_IODIRA, (uint16_t)(porta_dir | ((uint16_t)portb_dir << 8)));
}

void mcp23s17_write_olat(MCP23S17 *dev, uint8_t porta, uint8_t portb)
{
    spi_write_word(dev, MCP23S17_OLATA, (uint16_t)(porta | ((uint16_t)portb << 8)));
}

void mcp23s17_read_gpio(MCP23S17 *dev, uint8_t *porta, uint8_t *portb)
{
    uint16_t val = spi_read_word(dev, MCP23S17_GPIOA);

    if (porta != NULL) *porta = (uint8_t)(val & 0xFF);
    if (portb != NULL) *portb = (uint8_t)(val >> 8);
}
