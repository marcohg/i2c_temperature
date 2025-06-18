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
static uint8_t out_image = 0;

int pcf8754_write(uint8_t add_3bit, uint8_t wr) {

  status_t reVal = kStatus_Fail;
  g_MasterNackFlag = false;
  g_MasterCompletionFlag = false;

//  master_transfer.slaveAddress | =
  master_transfer.direction = kLPI2C_Write;
  master_transfer.subaddressSize = 0;   // Single write (no sub-address field)
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
  }
  out_image = wr;
  return 0;
}

/*!
 * @brief Read IO expander
 *
 * - We put the all port pins in input mode by writing 0xFF (use subaddress field as data)
 * - read one byte
 */
int pcf8754_read(uint8_t add_3bit, uint8_t *rd) {

  status_t reVal = kStatus_Fail;
  g_MasterNackFlag = false;
  g_MasterCompletionFlag = false;

//  master_transfer.slaveAddress | =
  master_transfer.direction = kLPI2C_Read;
  master_transfer.dataSize = 1; // read two times
  // Write one's (fake it)
  master_transfer.subaddress = 0xFF;
  master_transfer.subaddressSize = 1;
  reVal = LPI2C_MasterTransferNonBlocking(LPI2C1, &lpi2c1_master_handle, &master_transfer);

  if (reVal != kStatus_Success) {
    return -1;  // unsuccesful
  }
  /*  Wait for transfer completed. */
  while ((!g_MasterCompletionFlag) && (!g_MasterNackFlag)) {
  }
  if (g_MasterNackFlag) {
    return -2;  // Nacked
  }
  *rd = buffer;
  return 0;
}

