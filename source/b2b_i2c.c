#include "b2b_i2c.h"
#include "fsl_debug_console.h"
#include "peripherals.h"

static lpi2c_master_transfer_t master_transfer = {
    .slaveAddress = B2B_SLAVE_ADDR_7BIT,
    .direction = kLPI2C_Write,
    .subaddress = 0x01,
    .subaddressSize = 1,
    .data = lpi2c1_master_buffer,
    .dataSize = B2B_DATA_LENGTH,
    .flags = kLPI2C_TransferDefaultFlag,
};

int b2b_test() {
  status_t reVal = kStatus_Fail;

  PRINTF("\r\nLPI2C board2board interrupt example -- Master transfer.\r\n");

  /* Set up i2c master to send data to slave*/
  /* First data in txBuff is data length of the transmiting data. */
  uint8_t g_master_txBuff[B2B_DATA_LENGTH];
  g_master_txBuff[0] = lpi2c1_master_buffer[0] = B2B_DATA_LENGTH - 1U;
  for (uint32_t i = 1U; i < B2B_DATA_LENGTH; i++) {
    lpi2c1_master_buffer[i] = g_master_txBuff[i] = i - 1;
  }

  PRINTF("Master will send data :");
  for (uint32_t i = 0U; i < B2B_DATA_LENGTH - 1; i++) {
    if (i % 8 == 0) {
      PRINTF("\r\n");
    }
    PRINTF("0x%2x  ", g_master_txBuff[i + 1]);
  }
  PRINTF("\r\n\r\n");

  /* Send master non-blocking data to slave */
  master_transfer.direction = kLPI2C_Write;
  master_transfer.dataSize = B2B_DATA_LENGTH;
  reVal = LPI2C_MasterTransferNonBlocking(LPI2C1, &lpi2c1_master_handle, &master_transfer);

  if (reVal != kStatus_Success) {
    return -1;
  }
  /*  Wait for transfer completed. */
  while ((!g_MasterCompletionFlag) && (!g_MasterNackFlag)) {
  }
  if (g_MasterNackFlag) {
    PRINTF("Master nacked by slave!");
    g_MasterNackFlag = false;
  }
  g_MasterCompletionFlag = false;

  PRINTF("Receive sent data from slave :");

  /* subAddress = 0x01, data = g_master_rxBuff - read from slave.
   start + slaveaddress(w) + subAddress + repeated start + slaveaddress(r) + rx data buffer + stop */
  master_transfer.direction = kLPI2C_Read;
  master_transfer.dataSize = B2B_DATA_LENGTH - 1;
  reVal = LPI2C_MasterTransferNonBlocking(LPI2C1, &lpi2c1_master_handle,&master_transfer);
  if (reVal != kStatus_Success) {
    return -1;
  }

  /*  Wait for transfer completed. */
  while ((!g_MasterCompletionFlag) && (!g_MasterNackFlag)) {
  }
  if (g_MasterNackFlag) {
    PRINTF("Master nacked by slave!");
    g_MasterNackFlag = false;
  }
  g_MasterCompletionFlag = false;

  for (uint32_t i = 0U; i < B2B_DATA_LENGTH - 1; i++) {
    if (i % 8 == 0) {
      PRINTF("\r\n");
    }
    PRINTF("0x%2x  ", lpi2c1_master_buffer[i]);
  }
  PRINTF("\r\n\r\n");

  /* Transfer completed. Check the data.*/
  for (uint32_t i = 0U; i < B2B_DATA_LENGTH - 1; i++) {
    if (lpi2c1_master_buffer[i] != g_master_txBuff[i + 1]) {
      PRINTF("\r\nError occurred in the transfer ! \r\n");
      break;
    }
  }
  PRINTF("\r\nEnd of LPI2C example .\r\n");
  return 0;
}
