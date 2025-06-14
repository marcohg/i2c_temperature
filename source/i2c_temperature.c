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
#include "i2c.h"

/* TODO: insert other definitions and declarations here. */

/*
 * @brief   Application entry point.
 */
#include "fsl_iomuxc.h" // refactor
int main(void) {

    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    // refactor: MUX and configuration of i2c pins
    IOMUXC_SetPinMux(IOMUXC_GPIO_01_LPI2C1_SDA, 1U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_02_LPI2C1_SCL, 1U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_01_LPI2C1_SDA, 0xD8A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_02_LPI2C1_SCL, 0xD8A0U);

    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    PRINTF("Hello World\r\n");
    i2c_test();

    /* Force the counter to be placed into memory. */
    volatile static int i = 0 ;
    /* Enter an infinite loop, just incrementing a counter. */
    while(1) {
        i++ ;
        /* 'Dummy' NOP to allow source level single stepping of
            tight while() loop */
        __asm volatile ("nop");
    }
    return 0 ;
}
