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
*   This file contains the source functions for functions required to
*   specifically initialize the card.
*
 *
*END************************************************************************/

#include "mqx_inc.h"
#include "bsp.h"
#include "bsp_prv.h"
#include "io_rev.h"
#include "bsp_rev.h"


extern uint8_t PWRLG_LastGasp(void);

#if BSP_ALARM_FREQUENCY == 0
#error Wrong definition of BSP_ALARM_FREQUENCY
#endif

HWTIMER systimer;                                   //System timer handle

void _bsp_systimer_callback(void *dummy);         //callback for system timer

const char      *_mqx_bsp_revision = REAL_NUM_TO_STR(BSP_REVISION);
const char      *_mqx_io_revision  = REAL_NUM_TO_STR(IO_REVISION);
_mem_pool_id _BSP_sram_pool;

/** Pre initialization - initializing requested modules for basic run of MQX.
 */
int _bsp_pre_init(void) {
    uint32_t result;
    KERNEL_DATA_STRUCT_PTR         kernel_data;

    /* Set the CPU type */
    _mqx_set_cpu_type(MQX_CPU);

#if MQX_EXIT_ENABLED
    /* Set the bsp exit handler, called by _mqx_exit */
    _mqx_set_exit_handler(_bsp_exit_handler);
#endif

    /* Memory splitter - prevent accessing both ram banks in one instruction */
#if !MQX_USE_TLSF_ALLOCATOR
    _mem_alloc_at(0, (void*)0x20000000);
#endif

    result = _psp_int_init(BSP_FIRST_INTERRUPT_VECTOR_USED, BSP_LAST_INTERRUPT_VECTOR_USED);
    if (result != MQX_OK) {
        return result;
    }

    /* set possible new interrupt vector table - if MQX_ROM_VECTORS = 0 switch to
    ram interrupt table which was initialized in _psp_int_init) */
    _int_set_vector_table(BSP_INTERRUPT_VECTOR_TABLE);


#if BSPCFG_HAS_SRAM_POOL
    /* When kernel data is placed outside of the SRAM memory create new _BSP_sram_pool in the SRAM,
       otherwise if kernel data points to SRAM, the _BSP_sram_pool points to system pool. */
    if ( (((uint32_t)__INTERNAL_SRAM_BASE) < (uint32_t)BSP_DEFAULT_START_OF_KERNEL_MEMORY) &&
            (((uint32_t)BSP_DEFAULT_START_OF_KERNEL_MEMORY) < ((uint32_t)__INTERNAL_SRAM_BASE + (uint32_t)__INTERNAL_SRAM_SIZE)))
    {
        _BSP_sram_pool  = _mem_get_system_pool_id();
    }
    else
    {
        _BSP_sram_pool = _mem_create_pool(__SRAM_POOL, (uint32_t)__INTERNAL_SRAM_BASE + (uint32_t)__INTERNAL_SRAM_SIZE - (uint32_t)__SRAM_POOL);
    }
#endif

    /** bsp low level internal initialization. ***/
    _bsp_low_level_init();

    /* Initialize , set and run system hwtimer */
    result = hwtimer_init(&systimer, &BSP_SYSTIMER_DEV, BSP_SYSTIMER_ID, BSP_SYSTIMER_ISR_PRIOR);
    if (MQX_OK != result) {
        return result;
    }
    result = hwtimer_set_freq(&systimer, BSP_SYSTIMER_SRC_CLK, BSP_ALARM_FREQUENCY);
    if (MQX_OK != result) {
        hwtimer_deinit(&systimer);
        return result;
    }
    result = hwtimer_callback_reg(&systimer,(HWTIMER_CALLBACK_FPTR)_bsp_systimer_callback, NULL);
    if (MQX_OK != result) {
        hwtimer_deinit(&systimer);
        return result;
    }
    result = hwtimer_start(&systimer);
    if (MQX_OK != result) {
        hwtimer_deinit(&systimer);
        return result;
    }

    /* MCG initialization and internal oscillators trimming */
    if (CM_ERR_OK != _bsp_set_clock_configuration(BSP_CLOCK_CONFIGURATION_AUTOTRIM))
    {
        return MQX_TIMER_ISR_INSTALL_FAIL;
    }

    if (CM_ERR_OK != _bsp_osc_autotrim())
    {
        return MQX_TIMER_ISR_INSTALL_FAIL;
    }

    /* Switch to startup clock configuration */
    if (CM_ERR_OK != _bsp_set_clock_configuration(BSP_CLOCK_CONFIGURATION_STARTUP))
    {
        return MQX_TIMER_ISR_INSTALL_FAIL;
    }

    /* Initialize the system ticks */
    _GET_KERNEL_DATA(kernel_data);
    kernel_data->TIMER_HW_REFERENCE = (BSP_SYSTEM_CLOCK / BSP_ALARM_FREQUENCY);
    _time_set_ticks_per_sec(BSP_ALARM_FREQUENCY);
    _time_set_hwticks_per_tick(hwtimer_get_modulo(&systimer));
    _time_set_hwtick_function(_bsp_get_hwticks, (void *)NULL);

    return 0;
}

/** Initialization - called from init task, usually for io initialization.
 */
int _bsp_init(void) {
    uint32_t result;

    /* Initialize DMA */
    result = dma_init(_bsp_dma_devif_list);
    if (result != MQX_OK) {
        return result;
    }

#if BSPCFG_ENABLE_IO_SUBSYSTEM
    /* Initialize the I/O Sub-system */
    result = _io_init();
    if (result != MQX_OK) {
        return result;
    }

    /* Initialize RTC and MQX time */
#if BSPCFG_ENABLE_RTCDEV
    if (MQX_OK == _bsp_rtc_io_init())   {
        _rtc_init (NULL);
    }
#endif

#if BSPCFG_ENABLE_SPI0
    _io_spi_install("spi0:", &_bsp_spi0_init);
#endif

#if BSPCFG_ENABLE_SPI1
    _io_spi_install("spi1:", &_bsp_spi1_init);
#endif

    /* Install the GPIO driver */
#if BSPCFG_ENABLE_GPIODEV
    _io_gpio_install("gpio:");
#endif

#if BSPCFG_ENABLE_FLASHX
    _io_flashx_install("flashx:", &_bsp_flashx_init);
#endif

    /* Install device drivers */
#if BSPCFG_ENABLE_TTYC
    _kuart_polled_install("ttyc:", &_bsp_sci2_init, _bsp_sci2_init.QUEUE_SIZE);
#endif

#if BSPCFG_ENABLE_ITTYC
    _kuart_int_install("ittyc:", &_bsp_sci2_init, _bsp_sci2_init.QUEUE_SIZE);
#endif
#if BSPCFG_ENABLE_ADC0
    _io_adc_install("adc0:", (void *) &_bsp_adc0_init);
#endif

#if BSPCFG_ENABLE_ADC1
    _io_adc_install("adc1:", (void *) &_bsp_adc1_init);
#endif

#if BSPCFG_ENABLE_LWADC0
    _lwadc_init(&lwadc0_init);
#endif

#if BSPCFG_ENABLE_LWADC1
    _lwadc_init(&lwadc1_init);
#endif

#if BSPCFG_ENABLE_I2C0
      _ki2c_polled_install("i2c0:", &_bsp_i2c0_init);
#endif
#if BSPCFG_ENABLE_II2C0
      _ki2c_int_install("ii2c0:", &_bsp_i2c0_init);
#endif

   /* Everything below this is only required if booting full system */
   if (!PWRLG_LastGasp())
   {

    /* Install device drivers */
#if BSPCFG_ENABLE_TTYA
      _kuart_polled_install("ttya:", &_bsp_sci0_init, _bsp_sci0_init.QUEUE_SIZE);
#endif

#if BSPCFG_ENABLE_ITTYA
      _kuart_int_install("ittya:", &_bsp_sci0_init, _bsp_sci0_init.QUEUE_SIZE);
#endif

#if BSPCFG_ENABLE_TTYB
      _kuart_polled_install("ttyb:", &_bsp_sci1_init, _bsp_sci1_init.QUEUE_SIZE);
#endif

#if BSPCFG_ENABLE_ITTYB
      _kuart_int_install("ittyb:", &_bsp_sci1_init, _bsp_sci1_init.QUEUE_SIZE);
#endif

#if BSPCFG_ENABLE_TTYD
    _kuart_polled_install("ttyd:", &_bsp_sci3_init, _bsp_sci3_init.QUEUE_SIZE);
#endif

#if BSPCFG_ENABLE_ITTYD
      _kuart_int_install("ittyd:", &_bsp_sci3_init, _bsp_sci3_init.QUEUE_SIZE);
#endif

#if BSPCFG_ENABLE_TTYE
      _kuart_polled_install("ttye:", &_bsp_sci4_init, _bsp_sci4_init.QUEUE_SIZE);
#endif

#if BSPCFG_ENABLE_ITTYE
      _kuart_int_install("ittye:", &_bsp_sci4_init, _bsp_sci4_init.QUEUE_SIZE);
#endif

#if BSPCFG_ENABLE_TTYF
      _kuart_polled_install("ttyf:", &_bsp_sci5_init, _bsp_sci5_init.QUEUE_SIZE);
#endif

#if BSPCFG_ENABLE_ITTYF
      _kuart_int_install("ittyf:", &_bsp_sci5_init, _bsp_sci5_init.QUEUE_SIZE);
#endif

#if BSPCFG_ENABLE_I2C1
      _ki2c_polled_install("i2c1:", &_bsp_i2c1_init);
#endif

#if BSPCFG_ENABLE_II2C1
      _ki2c_int_install("ii2c1:", &_bsp_i2c1_init);
#endif

#if BSPCFG_ENABLE_I2C2
      _ki2c_polled_install("i2c2:", &_bsp_i2c2_init);
#endif
#if BSPCFG_ENABLE_II2C2
      _ki2c_int_install("ii2c2:", &_bsp_i2c2_init);
#endif

#if BSPCFG_ENABLE_SPI2
      _io_spi_install("spi2:", &_bsp_spi2_init);
#endif

    /* Install the USB DCD driver */
#if BSPCFG_ENABLE_USBDCD
      _kusb_dcd_polled_install("usbdcd:", &_bsp_usb_dcd_init);
#endif

   /* Install the USB DCD driver */
#if BSPCFG_ENABLE_IUSBDCD
      _kusb_dcd_int_install("usbdcd:", &_bsp_usb_dcd_init);
#endif

#if BSPCFG_ENABLE_SAI
      result = _io_sai_dma_install(&_bsp_sai_init);
#endif
    }

    /* Initialize the default serial I/O */
    _io_serial_default_init();

#endif // BSPCFG_ENABLE_IO_SUBSYSTEM

    return 0;
}


/*FUNCTION*--------------------------------------------------------------------
 *
 * Function Name    : _bsp_exit_handler
 * Returned Value   : none
 * Comments         :
 *    This function is called when MQX exits
 *
 *END*-----------------------------------------------------------------------*/

void _bsp_exit_handler(void) {

}

/*FUNCTION*********************************************************************
 *
 * Function Name    : _bsp_systimer_callback
 * Returned Value   : void
 * Comments         :
 *    The system timer callback.
 *
 *END**********************************************************************/

void _bsp_systimer_callback(void *dummy) {
    _time_notify_kernel();
}


/*FUNCTION*--------------------------------------------------------------------
 *
 * Function Name    : _bsp_get_hwticks
 * Returned Value   : none
 * Comments         :
 *    This function returns the number of hw ticks that have elapsed
 * since the last interrupt
 *
 *END*-----------------------------------------------------------------------*/

uint32_t _bsp_get_hwticks(void *param) {
    HWTIMER_TIME_STRUCT time;      //struct for storing time
    hwtimer_get_time(&systimer, &time);
    return time.SUBTICKS;
}

/* EOF */
