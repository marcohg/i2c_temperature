#ifndef _rt1010_i2c_
#define _rt1010_i2c_
#include <stdint.h>
//#include <stdbool.h>
#include "fsl_lpi2c.h"

#define EXAMPLE_I2C_MASTER_BASE (LPI2C1_BASE)
/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#define LPI2C_CLOCK_FREQUENCY ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))
#define LPI2C_MASTER_CLOCK_FREQUENCY LPI2C_CLOCK_FREQUENCY

#define EXAMPLE_I2C_MASTER ((LPI2C_Type *)EXAMPLE_I2C_MASTER_BASE)

#define I2C_MASTER_SLAVE_ADDR_7BIT 0x7EU
#define I2C_BAUDRATE               100000U
#define I2C_DATA_LENGTH            33U

extern uint8_t g_master_txBuff[];
extern uint8_t g_master_rxBuff[];
extern lpi2c_master_handle_t g_m_handle;
extern volatile bool g_MasterCompletionFlag;
extern volatile bool g_MasterNackFlag;

// lpi2c_master_transfer_callback_t
void lpi2c_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle, status_t completionStatus, void *userData);
int i2c_test();
#endif
