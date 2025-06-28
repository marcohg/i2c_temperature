#include "aht21_i2c.h"
#include "fsl_debug_console.h"
#include "peripherals.h"
#include "fsl_gpio.h"

extern uint64_t g_msec;


/*!
 * Access AHT21
 * Return 0 if IC is resent and the status
 */

static lpi2c_master_transfer_t get_status = {
    .flags = kLPI2C_TransferDefaultFlag,
    .slaveAddress = AHT21_SLAVE_ADDR_7BIT,
    .direction = kLPI2C_Read,
    .subaddress = AHT21_GET_STATUS,
    .subaddressSize = 1,
    .data = lpi2c1_master_buffer,
    .dataSize = 1             /*!< Number of bytes to transfer. */
};

static lpi2c_master_transfer_t measurement_trigger = {
    .flags = kLPI2C_TransferDefaultFlag,
    .slaveAddress = AHT21_SLAVE_ADDR_7BIT,
    .direction = kLPI2C_Write,
    .subaddress = AHT21_TRIGGER_MEASUREMENT,
    .subaddressSize = 1,
    .data = lpi2c1_master_buffer,
    .dataSize = 2             /*!< Number of bytes to transfer. */
};

static lpi2c_master_transfer_t measurement_request = {
    .flags = kLPI2C_TransferDefaultFlag,
    .slaveAddress = AHT21_SLAVE_ADDR_7BIT,
    .direction = kLPI2C_Read,
    .subaddress = 0,
    .subaddressSize = 0,
    .data = lpi2c1_master_buffer,
    .dataSize = 7 // State (1), Humidity+Temp (5), 1 CRC
};



static void ClearGlobalFlags(void) {
  g_MasterCompletionFlag = false;
  g_MasterNackFlag = false;

}

static int format_data(aht21_t *d, uint8_t *buffer) {
//  PRINTF("Fake Format Data\r\n");
  d->internal_state = buffer[0];
  uint32_t s_rh = (uint32_t)buffer[1] << 12 | (uint32_t)buffer[2] << 4 | buffer[3] >> 4;
//  s_rh >>= 4;
  d->relative_humidity = 100*((float)s_rh/POW_2_20);
  uint32_t s_t = (uint32_t)buffer[3] << 16 | (uint32_t)buffer[4] << 8 | (uint32_t)buffer[5];
  s_t &= 0x0FFFFF;
  d->temperature = ((float)s_t/POW_2_20)*200 -50;
  d->crc = buffer[6];
  return 0;
}

static bool Transfer(lpi2c_master_transfer_t t) {
  ClearGlobalFlags();
  status_t reVal;
  reVal = LPI2C_MasterTransferNonBlocking(LPI2C1, &lpi2c1_master_handle, &t);
  return reVal == kStatus_Success ? true : false;
}

static void TransitionState(aht21_t *aht, aht21_state_t new_state) {
    uint8_t *data;
  switch(new_state) {

    case GetStatus:
      PRINTF("Waiting State from AHT...\r\n");
      if (!Transfer(get_status))
        new_state = Transfer_Unsuccesful;
      break;

    case PowerupEnds:
      PRINTF("AHT Found, 10ms delay...\r\n");
      break;

    case MeasurementTriggered:
      GPIO_PinWrite(GPIO1, 11, 1);  // write 1
      data = (uint8_t *)measurement_trigger.data;
      data[0] = 0x33;
      data[1] = 0x00;
      aht->ms_period = g_msec;
      aht->last_measurement_retry = 0;
      if (!Transfer(measurement_trigger))
        new_state = Transfer_Unsuccesful;
      break;

    case MeasureDataRequested:
      if (!Transfer(measurement_request))
        new_state = Transfer_Unsuccesful;
      break;

    case InitializeAht:
      PRINTF("Sensor needs initialization (TBD)\r\n");
      break;

    case Transfer_Naked:
      PRINTF("Master naked by slave!");
      break;

    case Transfer_Timeout:
      PRINTF("Timeout!");
      break;

    case Transfer_Unsuccesful:
      PRINTF("Internal Driver Error!\r\n");
      break;

    default:
      break;
  }
  aht->ms = g_msec;
  aht->state = new_state;

}

int Aht21StateMachine(aht21_t *aht) {
  int new_measurement = 0;
  static uint64_t meas_delay = AHT21_TRIGGER_MEASURING_DELAY;
  static bool init_sm = false;

  switch (aht->state) {
    case Init:
      if(!init_sm) {
        aht->ms = g_msec;
        init_sm = true;
      }
      if(g_msec - aht->ms >= 100) {
        TransitionState(aht, GetStatus);
      }
      break;

    case GetStatus:
      // promote a function -{
      if (g_MasterCompletionFlag) {  // isr has completed transfer
        if ((lpi2c1_master_buffer[0] & 0x18) == 0x18) // status_word&0x18 == 0x18
          TransitionState(aht, PowerupEnds);
        else
          TransitionState(aht, InitializeAht);  // TBD
      } else if (g_MasterNackFlag) {
        TransitionState(aht, Transfer_Naked);
      } else if (g_msec - aht->ms >= TRANSFER_TIMEOUT) {
        TransitionState(aht, Transfer_Timeout);
      }
      // }
      break;

    case PowerupEnds:
      if (g_msec - aht->ms >= 10) {
        TransitionState(aht, MeasurementTriggered);
      }
      break;

    case MeasurementTriggered:
      if (g_msec - aht->ms >= meas_delay) {  // Complete Measurement
        TransitionState(aht,MeasureDataRequested);
        GPIO_PinWrite(GPIO1, 11, 0);  // write 0
      }
      break;

    case MeasureDataRequested:
      if (g_MasterCompletionFlag) {  // isr has completed transfer
        if ((lpi2c1_master_buffer[0] & 0x80) == 0) { // Status: measurement completed
          format_data(aht, lpi2c1_master_buffer);
          TransitionState(aht, MeasuringPeriod);
          new_measurement = 1;
          meas_delay = AHT21_TRIGGER_MEASURING_DELAY;
        }
        else {
          meas_delay = AHT21_RETRY_MEASURED_DATA_REQ_DELAY;
          TransitionState(aht,MeasurementTriggered);
        }
      } else if (g_MasterNackFlag) {
        TransitionState(aht, Transfer_Naked);
      } else if (g_msec - aht->ms >= 1000) {
        TransitionState(aht, Transfer_Timeout);
      }
      break;

    case MeasuringPeriod:
      if(g_msec - aht->ms_period >= MEASUREMENT_PERIOD)
        TransitionState(aht, MeasurementTriggered);
      break;

    case InitializeAht:
      if(g_msec - aht->ms_period >= 10000)
          TransitionState(aht, Init);
      break;

    case Transfer_Unsuccesful:
    case Transfer_Timeout:
    case Transfer_Naked:
      if (g_msec - aht->ms >= 5000)
        TransitionState(aht, Init);
      break;

    default:
      break;

  }
  return new_measurement;
}

