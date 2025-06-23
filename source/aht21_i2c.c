#include "aht21_i2c.h"
#include "fsl_debug_console.h"
#include "peripherals.h"


static lpi2c_master_transfer_t master_transfer = {
    .slaveAddress = AHT21_SLAVE_ADDR_7BIT,
    .direction = kLPI2C_Write,
    .subaddress = 0x01,
    .subaddressSize = 1,
    .data = lpi2c1_master_buffer,
    .flags = kLPI2C_TransferDefaultFlag,
};
extern uint64_t g_msec;


static int run_initialization() {
  uint64_t msec_now = g_msec;
  while(g_msec - msec_now < 5000) {
    if(g_msec % 500 == 0)
      PRINTF(".");
  }
  return 0;
}

#define AHT21_GET_STATUS 0x71
#define AHT21_TRIGGER_MEASUREMENT 0xAC

/*!
 * Access AHT21
 * Return 0 if IC is resent and the status
 */
static int get_status(uint8_t *status) {
  master_transfer.direction = kLPI2C_Read;
  master_transfer.dataSize = 1;
  master_transfer.subaddress = AHT21_GET_STATUS;  // Get status
  master_transfer.subaddressSize = 1;
  g_MasterCompletionFlag = false;
  g_MasterNackFlag = false;

  status_t reVal;
  reVal = LPI2C_MasterTransferNonBlocking(LPI2C1, &lpi2c1_master_handle, &master_transfer);
  if( reVal != kStatus_Success) {
    return -1;
  }
  /*  Wait for transfer completed. */
  uint64_t msec = g_msec;
  uint16_t timeout = 1000;
  while ((!g_MasterCompletionFlag) && (!g_MasterNackFlag) ) {
    if( g_msec - msec > timeout)
      return -3;
  }
  if (g_MasterNackFlag) {
    PRINTF("Master nacked by slave!");
    g_MasterNackFlag = false;
    return -2;
  }
  *status = lpi2c1_master_buffer[0];
  return 0;
}

static bool detect(void) {
  uint64_t msec = g_msec;
  PRINTF("\r\nDetect AHT21\r\n");
  while (g_msec - msec < 100)
    ;  // wait 100ms
  uint8_t status = 0;
  int detect_error = 0;
  detect_error = get_status(&status);
  if ( detect_error == 0) {
    if ((status & 0x18) == 0x18) {
      PRINTF("AHT Found and ready!\r\n");
      return true;
    } else {
      PRINTF("ATHF Found, running initialization\r\n");
      return run_initialization() == 0 ? true : false;
    }
  }
  PRINTF("ATH Detect Error %d\r\n", detect_error);  //
  return false;
}

static int format_data(aht21_t *d, uint8_t *buffer) {
//  PRINTF("Fake Format Data\r\n");
  d->state = buffer[0];
  uint32_t s_rh = (uint32_t)buffer[1] << 12 | (uint32_t)buffer[2] << 4 | buffer[3] >> 4;
//  s_rh >>= 4;
  d->relative_humidity = 100*(float)s_rh/POW_2_20;
  uint32_t s_t = (uint32_t)buffer[3] << 16 | (uint32_t)buffer[4] << 8 | (uint32_t)buffer[5];
  s_t &= 0x0FFFFF;
  d->temperature = ((float)s_t/POW_2_20)*200 -50;
  d->crc = buffer[6];
  return 0;
}

int aht21_measurement(aht21_t *aht21) {
  status_t reVal = kStatus_Fail;
  static bool detected = false;

  if(detected) {
    master_transfer.direction = kLPI2C_Write;
    master_transfer.subaddress = AHT21_TRIGGER_MEASUREMENT;  // Get status
    master_transfer.subaddressSize = 1;
    master_transfer.dataSize = 2;
    lpi2c1_master_buffer[0] = 0x33;
    lpi2c1_master_buffer[0] = 0x00;
    g_MasterCompletionFlag = false;
    g_MasterNackFlag = false;
    reVal = LPI2C_MasterTransferNonBlocking(LPI2C1, &lpi2c1_master_handle, &master_transfer);
    if(reVal != kStatus_Success)
      return -1; // Unsuccessful

    for(uint64_t msec = g_msec; g_msec - msec < 80; );

    // Read
    master_transfer.direction = kLPI2C_Read;
    master_transfer.dataSize = 7; // State (1), Humidity+Temp (5), 1 CRC
    master_transfer.subaddressSize = 0;
    for (int try_number = 0; try_number < 3; ++try_number) {
      reVal = LPI2C_MasterTransferNonBlocking(LPI2C1, &lpi2c1_master_handle, &master_transfer);
      if (reVal != kStatus_Success)
        return -1;  // Unsuccessful
      if ((lpi2c1_master_buffer[0] & 0x80) == 0) {
        format_data(aht21, lpi2c1_master_buffer);
        return 0;
      }
    }
    return 2; //
  }
  else {  // Note detected: new detection or retry
    detected = detect();
    if(detected) {
      uint64_t msec = g_msec;
      while(g_msec - msec < 10);  // wait for start measuring
      return 1; // not a measurement but ready to start
    }
    return 2;  // No measurement after trying 3 times
  }
}
//    return 0;

//
//
//  /* Set up i2c master to send data to slave*/
//  /* First data in txBuff is data length of the transmiting data. */
//  uint8_t g_master_txBuff[B2B_DATA_LENGTH];
//  g_master_txBuff[0] = lpi2c1_master_buffer[0] = B2B_DATA_LENGTH - 1U;
//  for (uint32_t i = 1U; i < B2B_DATA_LENGTH; i++) {
//    lpi2c1_master_buffer[i] = g_master_txBuff[i] = i - 1;
//  }
//
//  PRINTF("Master will send data :");
//  for (uint32_t i = 0U; i < B2B_DATA_LENGTH - 1; i++) {
//    if (i % 8 == 0) {
//      PRINTF("\r\n");
//    }
//    PRINTF("0x%2x  ", g_master_txBuff[i + 1]);
//  }
//  PRINTF("\r\n\r\n");
//
//  /* Send master non-blocking data to slave */
//  master_transfer.direction = kLPI2C_Write;
//  master_transfer.dataSize = B2B_DATA_LENGTH;
//  reVal = LPI2C_MasterTransferNonBlocking(LPI2C1, &lpi2c1_master_handle, &master_transfer);
//
//  if (reVal != kStatus_Success) {
//    return -1;
//  }
//  /*  Wait for transfer completed. */
//  while ((!g_MasterCompletionFlag) && (!g_MasterNackFlag)) {
//  }
//  if (g_MasterNackFlag) {
//    PRINTF("Master nacked by slave!");
//    g_MasterNackFlag = false;
//  }
//  g_MasterCompletionFlag = false;
//
//  PRINTF("Receive sent data from slave :");
//
//  /* subAddress = 0x01, data = g_master_rxBuff - read from slave.
//   start + slaveaddress(w) + subAddress + repeated start + slaveaddress(r) + rx data buffer + stop */
//  master_transfer.direction = kLPI2C_Read;
//  master_transfer.dataSize = B2B_DATA_LENGTH - 1;
//  reVal = LPI2C_MasterTransferNonBlocking(LPI2C1, &lpi2c1_master_handle,&master_transfer);
//  if (reVal != kStatus_Success) {
//    return -1;
//  }
//
//  /*  Wait for transfer completed. */
//  while ((!g_MasterCompletionFlag) && (!g_MasterNackFlag)) {
//  }
//  if (g_MasterNackFlag) {
//    PRINTF("Master nacked by slave!");
//    g_MasterNackFlag = false;
//  }
//  g_MasterCompletionFlag = false;
//
//  for (uint32_t i = 0U; i < B2B_DATA_LENGTH - 1; i++) {
//    if (i % 8 == 0) {
//      PRINTF("\r\n");
//    }
//    PRINTF("0x%2x  ", lpi2c1_master_buffer[i]);
//  }
//  PRINTF("\r\n\r\n");
//
//  /* Transfer completed. Check the data.*/
//  for (uint32_t i = 0U; i < B2B_DATA_LENGTH - 1; i++) {
//    if (lpi2c1_master_buffer[i] != g_master_txBuff[i + 1]) {
//      PRINTF("\r\nError occurred in the transfer ! \r\n");
//      break;
//    }
//  }
//  PRINTF("\r\nEnd of LPI2C example .\r\n");
//  return 0;
//}
