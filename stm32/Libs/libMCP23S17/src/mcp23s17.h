#ifndef MCP23S17_H
#define MCP23S17_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==============================
    Register addresses (BANK=0)
==============================*/
#define MCP23S17_IODIRA   0x00
#define MCP23S17_IODIRB   0x01
#define MCP23S17_IPOLA    0x02
#define MCP23S17_IPOLB    0x03
#define MCP23S17_GPINTENA 0x04
#define MCP23S17_GPINTENB 0x05
#define MCP23S17_DEFVALA  0x06
#define MCP23S17_DEFVALB  0x07
#define MCP23S17_INTCONA  0x08
#define MCP23S17_INTCONB  0x09
#define MCP23S17_IOCON    0x0A
#define MCP23S17_GPPUA    0x0C
#define MCP23S17_GPPUB    0x0D
#define MCP23S17_INTFA    0x0E
#define MCP23S17_INTFB    0x0F
#define MCP23S17_INTCAPA  0x10
#define MCP23S17_INTCAPB  0x11
#define MCP23S17_GPIOA    0x12
#define MCP23S17_GPIOB    0x13
#define MCP23S17_OLATA    0x14
#define MCP23S17_OLATB    0x15

/*==============================
    IOCON bits
==============================*/
#define MCP23S17_IOCON_HAEN   (1 << 3) /* Hardware address enable */
#define MCP23S17_IOCON_SEQOP  (1 << 5) /* Sequential operation disable */
#define MCP23S17_IOCON_MIRROR (1 << 6) /* INT pin mirror */
#define MCP23S17_IOCON_BANK   (1 << 7) /* Register bank layout */

/*==============================
    Direction constants
==============================*/
#define MCP23S17_ALL_OUTPUT 0x00
#define MCP23S17_ALL_INPUT  0xFF

/*==============================
    Configuration
==============================*/
typedef struct {
    uint8_t hw_addr; /* Hardware address A2:A0 (0–7) */

    void (*cs_assert)(void);
    void (*cs_deassert)(void);

    /* Full-duplex SPI: transmit tx[len], receive into rx[len].
       rx may be NULL for write-only transfers. */
    void (*spi_txrx)(const uint8_t *tx, uint8_t *rx, uint16_t len);
} MCP23S17Config;

/*==============================
    Device handle
==============================*/
typedef struct {
    MCP23S17Config config;
} MCP23S17;

/*==============================
    Public API
==============================*/

/* Initialize the device handle and enable hardware address decoding (HAEN).
   Returns 0 on success, -1 if config is invalid. */
int mcp23s17_init(MCP23S17 *dev, const MCP23S17Config *cfg);

/* Write a single register. */
void mcp23s17_write_reg(MCP23S17 *dev, uint8_t reg, uint8_t val);

/* Read a single register. */
uint8_t mcp23s17_read_reg(MCP23S17 *dev, uint8_t reg);

/* Write two consecutive registers as a 16-bit word (low byte first). */
void mcp23s17_write_word(MCP23S17 *dev, uint8_t reg, uint16_t val);

/* Read two consecutive registers as a 16-bit word (low byte first). */
uint16_t mcp23s17_read_word(MCP23S17 *dev, uint8_t reg);

/* Set I/O direction for both ports.
   1 = input, 0 = output (per MCP23S17 datasheet). */
void mcp23s17_set_direction(MCP23S17 *dev, uint8_t porta_dir, uint8_t portb_dir);

/* Write output latch registers for both ports. */
void mcp23s17_write_olat(MCP23S17 *dev, uint8_t porta, uint8_t portb);

/* Read GPIO registers for both ports. */
void mcp23s17_read_gpio(MCP23S17 *dev, uint8_t *porta, uint8_t *portb);

#ifdef __cplusplus
}
#endif

#endif /* MCP23S17_H */
