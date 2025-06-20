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

/* TODO: insert other definitions and declarations here. */


/*
 * @brief   Application entry point.
 */
#include "fsl_iomuxc.h" // refactor
int pcf8754_write(uint8_t addr_3bit, uint8_t wr);
int pcf8754_read(uint8_t add_3bit, uint8_t *rd);
int aht21_measurement(void);

bool gpt_tick = false;
uint64_t g_msec;  // msec since start application
/* GPT1_IRQn interrupt handler 1ms */
void GPT1_GPT_IRQHANDLER(void) {
  /*  Place your code here */
  gpt_tick = true;
  ++g_msec;
  /* Clear interrupt flag.*/
  GPT_ClearStatusFlags(GPT1_PERIPHERAL, kGPT_OutputCompare1Flag);

  /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F
     Store immediate overlapping exception return operation might vector to incorrect interrupt. */
  #if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
  #endif
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
  uint64_t wait = g_msec;
  uint16_t counter =0;
  while (g_msec - wait < 15000) {
    if (gpt_tick) {
      gpt_tick = false;
      if(++counter > 500) {
        counter =0;
        aht21_measurement();
        PRINTF("MEAS\r\n");
      }
    }
  }



//    b2b_test();
  uint8_t write_data = 0;
//  volatile uint8_t r;

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
