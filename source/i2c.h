#ifndef _rt1010_i2c_
#define _rt1010_i2c_
#include <stdint.h>
#include "fsl_lpi2c.h"


extern volatile bool g_MasterCompletionFlag;
extern volatile bool g_MasterNackFlag;

void lpi2c_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle, status_t completionStatus, void *userData);
#endif
