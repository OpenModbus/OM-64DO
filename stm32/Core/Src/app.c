#include "app.h"
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "mcp23s17.h"
#include "modbus_slave.h"
#include "modbus_bytes.h"
#include "config.h"

/* ---------- private variables --------------------------------------------- */
static MCP23S17          mcp[4];
static ModbusSlave       modbus;
static uint8_t           uart_rx_buf[1];
static volatile uint8_t  timer_counter;
static uint8_t           coil_state[8];
static uint8_t           mcp_porta[4];
static uint8_t           mcp_portb[4];
static Config            cfg;
static volatile bool     pending_reboot;

typedef struct { uint8_t chip, porta, portb; } ChMap;

/* Output channel mapping.
   MCPs 0 and 1 are wired in reverse order (channel 15→0).
   MCPs 2 and 3 are wired in normal order  (channel 0→15). */
static const ChMap ch_map[64] = {
  /* MCP 0 — reversed */
  {0,0x00,0x80},{0,0x00,0x40},{0,0x00,0x20},{0,0x00,0x10},
  {0,0x00,0x08},{0,0x00,0x04},{0,0x00,0x02},{0,0x00,0x01},
  {0,0x80,0x00},{0,0x40,0x00},{0,0x20,0x00},{0,0x10,0x00},
  {0,0x08,0x00},{0,0x04,0x00},{0,0x02,0x00},{0,0x01,0x00},
  /* MCP 1 — reversed */
  {1,0x00,0x80},{1,0x00,0x40},{1,0x00,0x20},{1,0x00,0x10},
  {1,0x00,0x08},{1,0x00,0x04},{1,0x00,0x02},{1,0x00,0x01},
  {1,0x80,0x00},{1,0x40,0x00},{1,0x20,0x00},{1,0x10,0x00},
  {1,0x08,0x00},{1,0x04,0x00},{1,0x02,0x00},{1,0x01,0x00},
  /* MCP 2 — normal */
  {2,0x01,0x00},{2,0x02,0x00},{2,0x04,0x00},{2,0x08,0x00},
  {2,0x10,0x00},{2,0x20,0x00},{2,0x40,0x00},{2,0x80,0x00},
  {2,0x00,0x01},{2,0x00,0x02},{2,0x00,0x04},{2,0x00,0x08},
  {2,0x00,0x10},{2,0x00,0x20},{2,0x00,0x40},{2,0x00,0x80},
  /* MCP 3 — normal */
  {3,0x01,0x00},{3,0x02,0x00},{3,0x04,0x00},{3,0x08,0x00},
  {3,0x10,0x00},{3,0x20,0x00},{3,0x40,0x00},{3,0x80,0x00},
  {3,0x00,0x01},{3,0x00,0x02},{3,0x00,0x04},{3,0x00,0x08},
  {3,0x00,0x10},{3,0x00,0x20},{3,0x00,0x40},{3,0x00,0x80},
};

/* ---------- output refresh ------------------------------------------------ */
static void outputs_refresh(void)
{
  uint8_t porta[4] = {0}, portb[4] = {0};

  for (uint8_t ch = 0; ch < 64; ch++)
  {
    uint8_t byte_idx = ch / 8;
    uint8_t bit_mask = (uint8_t)(1u << (ch % 8));
    if (coil_state[byte_idx] & bit_mask)
    {
      porta[ch_map[ch].chip] |= ch_map[ch].porta;
      portb[ch_map[ch].chip] |= ch_map[ch].portb;
    }
  }

  for (uint8_t i = 0; i < 4; i++)
  {
    if (porta[i] == mcp_porta[i] && portb[i] == mcp_portb[i]) continue;
    mcp23s17_write_olat(&mcp[i], porta[i], portb[i]);
    mcp_porta[i] = porta[i];
    mcp_portb[i] = portb[i];
  }
}

/* ---------- Modbus callbacks ---------------------------------------------- */
static ModbusExceptionCode mb_read_coils(uint16_t addr, uint16_t count, uint8_t *dest)
{
  if (addr + count > 64) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;

  uint8_t byte_count = (uint8_t)((count + 7) / 8);
  for (uint8_t i = 0; i < byte_count; i++) dest[i] = 0;

  for (uint16_t i = 0; i < count; i++)
  {
    uint16_t coil = addr + i;
    if (coil_state[coil / 8] & (1u << (coil % 8)))
      dest[i / 8] |= (uint8_t)(1u << (i % 8));
  }
  return MODBUS_EX_NONE;
}

static ModbusExceptionCode mb_write_single_coil(uint16_t addr, uint16_t value)
{
  if (addr >= 64) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;

  if (value)
    coil_state[addr / 8] |=  (uint8_t)(1u << (addr % 8));
  else
    coil_state[addr / 8] &= ~(uint8_t)(1u << (addr % 8));

  outputs_refresh();
  return MODBUS_EX_NONE;
}

static ModbusExceptionCode mb_write_multiple_coils(uint16_t addr, uint16_t count, const uint8_t *src)
{
  if (addr + count > 64) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;

  for (uint16_t i = 0; i < count; i++)
  {
    uint16_t coil = addr + i;
    if (src[i / 8] & (1u << (i % 8)))
      coil_state[coil / 8] |=  (uint8_t)(1u << (coil % 8));
    else
      coil_state[coil / 8] &= ~(uint8_t)(1u << (coil % 8));
  }
  outputs_refresh();
  return MODBUS_EX_NONE;
}

static ModbusExceptionCode mb_read_holding_registers(uint16_t addr, uint16_t count, uint8_t *dest)
{
  if (addr + count > CONFIG_NUM_REGS) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;

  for (uint16_t i = 0; i < count; i++)
    modbus_be16_set(&dest[i * 2], config_get_reg(&cfg, addr + i));

  return MODBUS_EX_NONE;
}

static ModbusExceptionCode mb_write_single_register(uint16_t addr, uint16_t value)
{
  if (addr >= CONFIG_NUM_REGS)           return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
  if (!config_validate_reg(addr, value)) return MODBUS_EX_ILLEGAL_DATA_VALUE;
  config_set_reg(&cfg, addr, value);
  if (!config_save(&cfg))                return MODBUS_EX_SLAVE_DEVICE_FAILURE;
  pending_reboot = true;
  return MODBUS_EX_NONE;
}

static ModbusExceptionCode mb_write_multiple_registers(uint16_t addr, uint16_t count, const uint8_t *src)
{
  if (addr + count > CONFIG_NUM_REGS) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;

  for (uint16_t i = 0; i < count; i++)
  {
    if (!config_validate_reg(addr + i, modbus_be16_get(&src[i * 2])))
      return MODBUS_EX_ILLEGAL_DATA_VALUE;
  }
  for (uint16_t i = 0; i < count; i++)
    config_set_reg(&cfg, addr + i, modbus_be16_get(&src[i * 2]));

  if (!config_save(&cfg)) return MODBUS_EX_SLAVE_DEVICE_FAILURE;
  pending_reboot = true;
  return MODBUS_EX_NONE;
}

/* ---------- RS-485 transmit ------------------------------------------------ */
static void mb_transmit(const uint8_t *data, uint16_t length)
{
  HAL_UART_AbortReceive(&huart2);
  HAL_GPIO_WritePin(USART2_DIR_GPIO_Port, USART2_DIR_Pin, GPIO_PIN_SET);
  HAL_UART_Transmit(&huart2, (uint8_t *)data, length, HAL_MAX_DELAY);
  while (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC) == RESET);
  HAL_GPIO_WritePin(USART2_DIR_GPIO_Port, USART2_DIR_Pin, GPIO_PIN_RESET);
  __HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_RXNE);
  HAL_UART_Receive_IT(&huart2, uart_rx_buf, 1);
}

/* ---------- HAL callbacks ------------------------------------------------- */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance != USART2) return;

  __HAL_TIM_SET_COUNTER(&htim1, 0);
  timer_counter = 0;

  modbus_slave_rx_byte(&modbus, uart_rx_buf[0]);

  HAL_UART_Receive_IT(&huart2, uart_rx_buf, 1);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance != USART2) return;
  HAL_UART_Receive_IT(&huart2, uart_rx_buf, 1);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance != TIM1) return;

  if (modbus.state != RECEPTION && modbus.state != CONTROL_AND_WAITING) return;

  timer_counter++;

  if (timer_counter == 3) modbus_slave_1_5t_elapsed(&modbus);
  if (timer_counter == 7) modbus_slave_3_5t_elapsed(&modbus);
}

/* ---------- SPI/CS helpers for MCP23S17 ----------------------------------- */
static void mcp_cs_assert(void)
{
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
}

static void mcp_cs_deassert(void)
{
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
}

static void mcp_spi_txrx(const uint8_t *tx, uint8_t *rx, uint16_t len)
{
  if (rx == NULL)
    HAL_SPI_Transmit(&hspi1, (uint8_t *)tx, len, HAL_MAX_DELAY);
  else
    HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)tx, rx, len, HAL_MAX_DELAY);
}

/* ---------- public API ---------------------------------------------------- */
void app_init(void)
{
  cfg = config_load();
  config_apply(&cfg, &huart2, &htim1);

  static const uint8_t mcp_addrs[4] = { 0, 1, 3, 7 };

  MCP23S17Config mcp_cfg = {
    .cs_assert   = mcp_cs_assert,
    .cs_deassert = mcp_cs_deassert,
    .spi_txrx    = mcp_spi_txrx,
  };

  /* Broadcast HAEN enable to all chips while hardware addressing is still
     disabled (HAEN=0). All chips respond to any address, so addr=0 reaches
     all four. After this, each chip only responds to its own hw_addr. */
  MCP23S17 mcp_broadcast;
  mcp_cfg.hw_addr = 0;
  mcp23s17_init(&mcp_broadcast, &mcp_cfg);
  HAL_Delay(1);

  for (uint8_t i = 0; i < 4; i++)
  {
    mcp_cfg.hw_addr = mcp_addrs[i];
    mcp23s17_init(&mcp[i], &mcp_cfg);
    mcp23s17_set_direction(&mcp[i], MCP23S17_ALL_OUTPUT, MCP23S17_ALL_OUTPUT);
    mcp23s17_write_olat(&mcp[i], 0x00, 0x00);
  }

  HAL_GPIO_WritePin(USART2_DIR_GPIO_Port, USART2_DIR_Pin, GPIO_PIN_RESET);

  ModbusSlaveConfig modbus_cfg = {
    .address                  = cfg.slave_addr,
    .write                    = mb_transmit,
    .read_coils               = mb_read_coils,
    .write_single_coil        = mb_write_single_coil,
    .write_multiple_coils     = mb_write_multiple_coils,
    .read_holding_registers   = mb_read_holding_registers,
    .write_single_register    = mb_write_single_register,
    .write_multiple_registers = mb_write_multiple_registers,
  };
  modbus_slave_init(&modbus, &modbus_cfg);

  HAL_TIM_Base_Start_IT(&htim1);
  HAL_UART_Receive_IT(&huart2, uart_rx_buf, 1);
}

void app_poll(void)
{
  if (modbus.frame_available)
    modbus_slave_poll(&modbus);

  if (pending_reboot)
    NVIC_SystemReset();
}
