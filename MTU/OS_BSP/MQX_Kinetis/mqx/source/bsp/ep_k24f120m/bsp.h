/*HEADER**********************************************************************
*
* Copyright 2014 Freescale Semiconductor, Inc.
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale MQX RTOS License
* distributed with this Material.
* See the MQX_RTOS_LICENSE file distributed for more details.
*
* Brief License Summary:
* This software is provided in source form for you to use free of charge,
* but it is not open source software. You are allowed to use this software
* but you cannot redistribute it or derivative works of it in source form.
* The software may be used only in connection with a product containing
* a Freescale microprocessor, microcontroller, or digital signal processor.
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   This file includes all include files specific to this target system.
*
*
*END************************************************************************/

#ifndef __bsp_h__
#define __bsp_h__   1

#include <psp.h>

/* Processor Expert files */
#include <PE_LDD.h>
#ifdef PE_LDD_VERSION
#include <Events.h>
#endif

#include <bsp_rev.h>
#include <ep_k24f120m.h>

/* Generic IO drivers */
#include <fio.h>
#include <io.h>
#include <io_mem.h>
#include <io_null.h>
/* Clock manager */
#include <cm_kinetis.h>
#include <bsp_cm.h>
#include <cm.h>
/* Low power manager*/
#include <init_lpm.h>
#include <lpm_kinetis.h>
#include <lpm.h>

/* Hardware timer */
#include <hwtimer.h>
#include <hwtimer_pit.h>
#include <hwtimer_lpt.h>
#include <hwtimer_systick.h>
/* Memories and Memory Interfaces */
#include <flashx.h>
#include <flash_ftfe.h>
#include <flash_mk24.h>
/* Analog drivers */
#include <adc_mk24.h>
#include <adc.h>
#include <adc_kadc.h>
#include <lwadc_kadc.h>
/* Timers */
#include <krtc.h>
#include <rtc.h>
/* Communication Interfaces */
#include <serial.h>
#include <serl_kuart.h>
#include <i2c.h>
#include <i2c_ki2c.h>
#include <spi.h>
#include <spi_dspi.h>
#include <spi_dspi_dma.h>
/* Human-Machine Interfaces */
#include <lwgpio_kgpio.h>
#include <lwgpio.h>
#include <io_gpio.h>
#include <io_gpio_kgpio.h>

/* Other drivers */
#include <timer_qpit.h>
#include <dma.h>
#include <rnga.h>

#ifdef __cplusplus
extern "C" {
#endif

_mqx_int _bsp_adc_io_init(_mqx_uint adc_num);
_mqx_int _bsp_adc_channel_io_init(uint16_t source);
_mqx_int _bsp_dspi_io_init(uint32_t dev_num);

_mqx_int _bsp_i2c_io_init( uint32_t );
_mqx_int _bsp_gpio_io_init( void );

_mqx_int _bsp_rtc_io_init( void );
_mqx_int _bsp_serial_io_init(uint8_t dev_num,  uint8_t flags);
_mqx_int _bsp_serial_rts_init( uint32_t );
_mqx_int _bsp_rnga_io_init(void);
_mqx_int _bsp_ftfx_io_init(_mqx_uint);
_mqx_int _bsp_serial_irda_tx_init(uint32_t, bool);
_mqx_int _bsp_serial_irda_rx_init(uint32_t, bool);

#if BSP_ENET_WIFI_ENABLED
bool  _bsp_get_mac_address(uint32_t,uint32_t,_enet_address);
#endif

extern const LPM_CPU_OPERATION_MODE LPM_CPU_OPERATION_MODES[];

extern const LWADC_INIT_STRUCT lwadc0_init;
extern const LWADC_INIT_STRUCT lwadc1_init;

#define _bsp_int_init(num, prior, subprior, enable)     _nvic_int_init(num, prior, enable)
#define _bsp_int_enable(num)                            _nvic_int_enable(num)
#define _bsp_int_disable(num)                           _nvic_int_disable(num)

#ifdef __cplusplus
}
#endif

#endif  /* _bsp_h_ */
/* EOF */
