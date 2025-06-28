/*
 * Copyright 2016-2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    i2c_temperature.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
/* TODO: insert other include files here. */
#include "b2b_i2c.h"
#include "aht21_i2c.h"

/* TODO: insert other definitions and declarations here. */


/*
 * @brief   Application entry point.
 */
#include "fsl_iomuxc.h" // refactor
int pcf8754_write(uint8_t addr_3bit, uint8_t wr);
int pcf8754_read(uint8_t add_3bit, uint8_t *rd);

bool gpt_tick = false;
volatile uint64_t g_msec;  // msec since start application

/* GPT1_IRQn interrupt handler 1ms */
void GPT1_GPT_IRQHANDLER(void) {
  /*  Place your code here */
  gpt_tick = true;
  ++g_msec;
  /* Clear interrupt flag.*/
  GPT_ClearStatusFlags(GPT1_PERIPHERAL, kGPT_OutputCompare1Flag);
  __DSB();  // Needed in RT1010 Cortex-M7
}


uint8_t read_data[4];
int main(void) {

  /* Init board hardware. */
  BOARD_ConfigMPU();
  BOARD_InitBootPins();
  BOARD_InitBootClocks();
  BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
  /* Init FSL debug console. */
  BOARD_InitDebugConsole();
#endif

  PRINTF("Hello World\r\n");
  aht21_t aht = { .state = Init };

  while (1) {
      if(Aht21StateMachine(&aht)) {
        float C = aht.temperature;
        PRINTF("R: %5.2f, T:%5.1fC,%5.1fF\r\n", aht.relative_humidity, C, C*9/5+32);
      }
    }

//    b2b_test();
  uint8_t write_data = 0;

  /* Force the counter to be placed into memory. */
  volatile static int i = 0;
  /* Enter an infinite loop, just incrementing a counter. */

  const uint8_t io_expander_addr = 0;
  while (1) {
    i++;
    // We use isr but we block for about ~1.25ms, make tick 10ms
    if (gpt_tick) {
      gpt_tick = false;
      static uint32_t ticks = 0;
      static secs = 0;
      if(ticks % 100 == 0) {
        ++secs;
      }
      if (++ticks % 10 == 0) {
        int wr_status = pcf8754_write(0, write_data);
        int rd_status = pcf8754_read(0, read_data);
        ++write_data;
        if (!rd_status && !wr_status) {
          PRINTF("%5u Write %x, Read %x\r\n", secs, write_data & 0x0ff,
                 read_data[0] & 0x0ff);
        } else {
          PRINTF("Failed! r:%d, w:%d\r\n", rd_status, wr_status);
        }
      }
    }
  }
  return 0;
}
