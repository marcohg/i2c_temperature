#ifndef _AHT21_I2C_RT1010_I2C_
#define _AHT21_I2C_RT1010_I2C_
#include <stdint.h>
#include "fsl_lpi2c.h"
#include "i2c.h"

// AHT21 States
typedef enum {
  Init,
  Powerup,          //  100ms delay
  WaitStateReply,   //  Host request AHT state, if ready or needs Initialization
  InitializeAht,    //  AHT requires initialization (TBD)
  PauseBeforeMeasuring, // After detecting AHD and State replied, 10ms pause

  MeasuringState,  // Wait 80ms for completion of the last measurement triggered
  MeasurementReply,// Wait for the 7 bytes to arrive
  RetryLastMeasuring,  // retry last measurement
  MeasuringPeriod,  // Wait for the period to elapse
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
#define POW_2_20 (0x100000)	// 2^20
#define TRANSFER_TIMEOUT 1000   // ms
#define MEASUREMENT_PERIOD 1000

//
//} i2c_transfer_result_t;
#define I2C_TRANSFER_TIMEOUT      -3
#define I2C_TRANSFER_NAK_BY_SLAVE -2
#define I2C_TRANSFER_UNSUCCESFUL  -1
#define I2C_TRANSFER_SUCCESFUL    0
#define I2C_TRANSFER_WAITING      1



//extern lpi2c_master_handle_t g_m_handle;
extern volatile bool g_MasterCompletionFlag;
extern volatile bool g_MasterNackFlag;

// lpi2c_master_transfer_callback_t
//void lpi2c_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle, status_t completionStatus, void *userData);
int Aht21StateMachine(aht21_t *aht21);
#endif
