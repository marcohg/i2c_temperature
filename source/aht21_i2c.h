#ifndef _b2b_rt1010_i2c_
#define _b2b_rt1010_i2c_
#include <stdint.h>
#include "fsl_lpi2c.h"
#include "i2c.h"


#define AHT21_SLAVE_ADDR_7BIT 0x38U
//#define B2B_DATA_LENGTH            33U

//extern lpi2c_master_handle_t g_m_handle;
extern volatile bool g_MasterCompletionFlag;
extern volatile bool g_MasterNackFlag;

// lpi2c_master_transfer_callback_t
//void lpi2c_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle, status_t completionStatus, void *userData);
int aht21_measurement();
#endif
