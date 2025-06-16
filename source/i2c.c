#include "i2c.h"
#include "fsl_debug_console.h"
#include "peripherals.h"

volatile bool g_MasterCompletionFlag = false;
volatile bool g_MasterNackFlag = false;

void lpi2c1_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle,
                           status_t status, void *userData) {
  if (status == kStatus_LPI2C_Nak) {
    g_MasterNackFlag = true;
  } else {
    g_MasterCompletionFlag = true;
    /* Display failure information when status is not success. */
    if (status != kStatus_Success) {
      PRINTF("Error occured during transfer!");
    }
  }
}

