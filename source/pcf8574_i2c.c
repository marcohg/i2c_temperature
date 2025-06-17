#include "b2b_i2c.h"
#include "fsl_debug_console.h"
#include "peripherals.h"
uint8_t buffer;
static lpi2c_master_transfer_t master_transfer =
{
    .slaveAddress = 0x20,
    .direction = kLPI2C_Write,
    .subaddress = 0x0,
    .subaddressSize = 0,
    .data = &buffer,
    .dataSize = 1,
    .flags = kLPI2C_TransferDefaultFlag
};


int pcf8754_write(uint8_t add_3bit, uint8_t wr, uint8_t *rd) {

  status_t reVal = kStatus_Fail;
  g_MasterNackFlag = false;
  g_MasterCompletionFlag = false;

  PRINTF("\r\nLPI2C PCF8754 -- Master transfer.\r\n");

//  master_transfer.slaveAddress | =
  master_transfer.direction = kLPI2C_Write;
  master_transfer.dataSize = 1;
  buffer = wr;
  reVal = LPI2C_MasterTransferNonBlocking(LPI2C1, &lpi2c1_master_handle, &master_transfer);

  if (reVal != kStatus_Success) {
    return -1;  // unsuccesful
  }
  /*  Wait for transfer completed. */
  while ((!g_MasterCompletionFlag) && (!g_MasterNackFlag)) {
  }
  if (g_MasterNackFlag) {
    return -2;  // Nacked
//    PRINTF("Master nacked by slave!");
//    g_MasterNackFlag = false;
  }
//  g_MasterCompletionFlag = false;
  PRINTF("Done");
  return 0;
}

