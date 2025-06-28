#ifndef _AHT21_I2C_RT1010_I2C_
#define _AHT21_I2C_RT1010_I2C_
#include <stdint.h>
#include "fsl_lpi2c.h"
#include "i2c.h"

// AHT21 States
typedef enum aht_state_tag {
  Init = 0,
  GetStatus,
  InitializeAht,    	  //  AHT requires initialization (TBD)
  PowerupEnds, 				  // Short delay at end of init sequence
  MeasurementTriggered, // Wait before sending a meassure data request (80ms)
  MeasureDataRequested, // wait for the i2c data to complete
  RetryDataRequest,     // sensor was busy, retry
  MeasuringPeriod,  		// Waiting to comply measurement period
  // Some errors
  Transfer_Unsuccesful,
  Transfer_Timeout,
  Transfer_Naked

} aht21_state_t;

typedef struct aht21_tag {
  uint8_t internal_state;
  uint8_t crc;
  float temperature;
  float relative_humidity;
  aht21_state_t state;
  uint64_t ms;
  uint64_t ms_period;
  uint8_t last_measurement_retry;

//  int i2c_transfer;
} aht21_t;

#define AHT21_SLAVE_ADDR_7BIT 0x38U
#define AHT21_GET_STATUS 0x71
#define AHT21_TRIGGER_MEASUREMENT 0xAC
#define AHT21_TRIGGER_MEASURING_DELAY 80
#define AHT21_RETRY_MEASURED_DATA_REQ_DELAY 10


#define POW_2_20 (0x100000)	// 2^20
#define TRANSFER_TIMEOUT 1000   // ms
#define MEASUREMENT_PERIOD 500

//
//} i2c_transfer_result_t;
#define I2C_TRANSFER_TIMEOUT      -3
#define I2C_TRANSFER_NAK_BY_SLAVE -2
#define I2C_TRANSFER_UNSUCCESFUL  -1
#define I2C_TRANSFER_SUCCESFUL    0
#define I2C_TRANSFER_WAITING      1

extern volatile bool g_MasterCompletionFlag;
extern volatile bool g_MasterNackFlag;

int Aht21StateMachine(aht21_t *aht21);
#endif
