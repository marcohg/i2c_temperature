#include "i2c.h"
#include "fsl_debug_console.h"
#include "peripherals.h"



//uint8_t g_master_rxBuff[I2C_DATA_LENGTH];
//lpi2c_master_handle_t g_m_handle;
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

int i2c_test() {
  lpi2c_master_transfer_t masterXfer = { 0 }; // use peripheral settings
  status_t reVal = kStatus_Fail;

  /*Clock setting for LPI2C*/
//  CLOCK_SetMux(kCLOCK_Lpi2cMux, LPI2C_CLOCK_SOURCE_SELECT);   // not set by peripheral yet
//  CLOCK_SetDiv(kCLOCK_Lpi2cDiv, LPI2C_CLOCK_SOURCE_DIVIDER);

  PRINTF("\r\nLPI2C board2board interrupt example -- Master transfer.\r\n");

  /* Set up i2c master to send data to slave*/
  /* First data in txBuff is data length of the transmiting data. */
  uint8_t g_master_txBuff[I2C_DATA_LENGTH];
  g_master_txBuff[0] = lpi2c1_master_buffer[0] = I2C_DATA_LENGTH - 1U;
  for (uint32_t i = 1U; i < I2C_DATA_LENGTH; i++) {
    lpi2c1_master_buffer[i] = g_master_txBuff[i] = i - 1;
  }

  PRINTF("Master will send data :");
  for (uint32_t i = 0U; i < I2C_DATA_LENGTH - 1; i++) {
    if (i % 8 == 0) {
      PRINTF("\r\n");
    }
    PRINTF("0x%2x  ", g_master_txBuff[i + 1]);
  }
  PRINTF("\r\n\r\n");

//  lpi2c_master_config_t masterConfig;

  /*
   * masterConfig.debugEnable = false;
   * masterConfig.ignoreAck = false;
   * masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
   * masterConfig.baudRate_Hz = 100000U;
   * masterConfig.busIdleTimeout_ns = 0;
   * masterConfig.pinLowTimeout_ns = 0;
   * masterConfig.sdaGlitchFilterWidth_ns = 0;
   * masterConfig.sclGlitchFilterWidth_ns = 0;
   */
//  LPI2C_MasterGetDefaultConfig(&masterConfig);

  /* Change the default baudrate configuration */
//  masterConfig.baudRate_Hz = I2C_BAUDRATE;

  /* Initialize the LPI2C master peripheral */
//  LPI2C_MasterInit(EXAMPLE_I2C_MASTER, &masterConfig,  LPI2C_MASTER_CLOCK_FREQUENCY);

  /* Create the LPI2C handle for the non-blocking transfer */
//  LPI2C_MasterTransferCreateHandle(EXAMPLE_I2C_MASTER, &g_m_handle,lpi2c_master_callback, NULL);

  /* subAddress = 0x01, data = g_master_txBuff - write to slave.
   start + slaveaddress(w) + subAddress + length of data buffer + data buffer + stop*/
  uint8_t deviceAddress = 0x01U;
  masterXfer.slaveAddress = I2C_MASTER_SLAVE_ADDR_7BIT;
  masterXfer.direction = kLPI2C_Write;
  masterXfer.subaddress = (uint32_t) deviceAddress;
  masterXfer.subaddressSize = 1;
  masterXfer.data = lpi2c1_master_buffer;
  masterXfer.dataSize = I2C_DATA_LENGTH;
  masterXfer.flags = kLPI2C_TransferDefaultFlag;

  /* Send master non-blocking data to slave */
//  reVal = LPI2C_MasterTransferNonBlocking(EXAMPLE_I2C_MASTER, &g_m_handle, &masterXfer);
  reVal = LPI2C_MasterTransferNonBlocking(LPI2C1, &lpi2c1_master_handle, &masterXfer);

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
  masterXfer.slaveAddress = I2C_MASTER_SLAVE_ADDR_7BIT;
  masterXfer.direction = kLPI2C_Read;
  masterXfer.subaddress = (uint32_t) deviceAddress;
  masterXfer.subaddressSize = 1;
  masterXfer.data = lpi2c1_master_buffer;
  masterXfer.dataSize = I2C_DATA_LENGTH - 1;
  masterXfer.flags = kLPI2C_TransferDefaultFlag;

//  reVal = LPI2C_MasterTransferNonBlocking(EXAMPLE_I2C_MASTER, &lpi2c1_master_handle,&masterXfer);
  reVal = LPI2C_MasterTransferNonBlocking(EXAMPLE_I2C_MASTER, &lpi2c1_master_handle,&masterXfer);
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

  for (uint32_t i = 0U; i < I2C_DATA_LENGTH - 1; i++) {
    if (i % 8 == 0) {
      PRINTF("\r\n");
    }
    PRINTF("0x%2x  ", lpi2c1_master_buffer[i]);
  }
  PRINTF("\r\n\r\n");

  /* Transfer completed. Check the data.*/
  for (uint32_t i = 0U; i < I2C_DATA_LENGTH - 1; i++) {
    if (lpi2c1_master_buffer[i] != g_master_txBuff[i + 1]) {
      PRINTF("\r\nError occurred in the transfer ! \r\n");
      break;
    }
  }
  PRINTF("\r\nEnd of LPI2C example .\r\n");
  return 0;
}
