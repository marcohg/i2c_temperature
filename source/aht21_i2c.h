#ifndef _AHT21_I2C_RT1010_I2C_
#define _AHT21_I2C_RT1010_I2C_
#include <stdint.h>
#include "fsl_lpi2c.h"
#include "i2c.h"

// State
typedef struct aht21_tag {
  uint8_t state;
  uint8_t crc;
  float temperature;
  float relative_humidity;
} aht21_t;

#define AHT21_SLAVE_ADDR_7BIT 0x38U
#define POW_2_20 (0x100000)	// 2^20

//#define B2B_DATA_LENGTH            33U

//extern lpi2c_master_handle_t g_m_handle;
extern volatile bool g_MasterCompletionFlag;
extern volatile bool g_MasterNackFlag;

// lpi2c_master_transfer_callback_t
//void lpi2c_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle, status_t completionStatus, void *userData);
int aht21_measurement(aht21_t *aht21);
#endif
