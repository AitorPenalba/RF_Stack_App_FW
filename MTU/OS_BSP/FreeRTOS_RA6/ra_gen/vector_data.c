/* generated vector source file - do not edit */
        #include "bsp_api.h"

        extern void HardFault_Handler( void );      /* Aclara Added: add unused interrupt handler extern  */

        /* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
        #if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_MAX_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = sci_uart_rxi_isr, /* SCI2 RXI (Received data full) */
            [1] = sci_uart_txi_isr, /* SCI2 TXI (Transmit data empty) */
            [2] = sci_uart_tei_isr, /* SCI2 TEI (Transmit end) */
            [3] = sci_uart_eri_isr, /* SCI2 ERI (Receive error) */
            [4] = iic_master_rxi_isr, /* IIC0 RXI (Receive data full) */
            [5] = iic_master_txi_isr, /* IIC0 TXI (Transmit data empty) */
            [6] = iic_master_tei_isr, /* IIC0 TEI (Transmit end) */
            [7] = iic_master_eri_isr, /* IIC0 ERI (Transfer error) */
            [8] = r_icu_isr, /* ICU IRQ11 (External pin interrupt 11) */
            [9] = sci_uart_rxi_isr, /* SCI3 RXI (Received data full) */
            [10] = sci_uart_txi_isr, /* SCI3 TXI (Transmit data empty) */
            [11] = sci_uart_tei_isr, /* SCI3 TEI (Transmit end) */
            [12] = sci_uart_eri_isr, /* SCI3 ERI (Receive error) */
            [13] = sci_uart_rxi_isr, /* SCI4 RXI (Received data full) */
            [14] = sci_uart_txi_isr, /* SCI4 TXI (Transmit data empty) */
            [15] = sci_uart_tei_isr, /* SCI4 TEI (Transmit end) */
            [16] = sci_uart_eri_isr, /* SCI4 ERI (Receive error) */
            [17] = sci_uart_rxi_isr, /* SCI9 RXI (Received data full) */
            [18] = sci_uart_txi_isr, /* SCI9 TXI (Transmit data empty) */
            [19] = sci_uart_tei_isr, /* SCI9 TEI (Transmit end) */
            [20] = sci_uart_eri_isr, /* SCI9 ERI (Receive error) */
            [21] = rtc_alarm_periodic_isr, /* RTC ALARM (Alarm interrupt) */
            [22] = rtc_carry_isr, /* RTC CARRY (Carry interrupt) */
            [23] = gpt_counter_overflow_isr, /* GPT1 COUNTER OVERFLOW (Overflow) */
            [24] = gpt_capture_a_isr, /* GPT1 CAPTURE COMPARE A (Compare match A) */
            [25] = spi_rxi_isr, /* SPI1 RXI (Receive buffer full) */
            [26] = spi_txi_isr, /* SPI1 TXI (Transmit buffer empty) */
            [27] = spi_tei_isr, /* SPI1 TEI (Transmission complete event) */
            [28] = spi_eri_isr, /* SPI1 ERI (Error) */
            [29] = fcu_frdyi_isr, /* FCU FRDYI (Flash ready interrupt) */
            [30] = fcu_fiferr_isr, /* FCU FIFERR (Flash access error interrupt) */
            [31] = agt_int_isr, /* AGT0 INT (AGT interrupt) */
            [32] = r_icu_isr, /* ICU IRQ13 (External pin interrupt 13) */
            [33] = agt_int_isr, /* AGT1 INT (AGT interrupt) */
            [34] = r_icu_isr, /* ICU IRQ4 (External pin interrupt 4) */
            [35] = r_icu_isr, /* ICU IRQ14 (External pin interrupt 14) */
            [36] = gpt_counter_overflow_isr, /* GPT2 COUNTER OVERFLOW (Overflow) */
            [37] = gpt_capture_b_isr, /* GPT2 CAPTURE COMPARE B (Compare match B) */

            /* Aclara Added: add unused interrupt handler for interrupts 38-95 */
            [38] = HardFault_Handler,
            [39] = HardFault_Handler,
            [40] = HardFault_Handler,
            [41] = HardFault_Handler,
            [42] = HardFault_Handler,
            [43] = HardFault_Handler,
            [44] = HardFault_Handler,
            [45] = HardFault_Handler,
            [46] = HardFault_Handler,
            [47] = HardFault_Handler,
            [48] = HardFault_Handler,
            [49] = HardFault_Handler,
            [50] = HardFault_Handler,
            [51] = HardFault_Handler,
            [52] = HardFault_Handler,
            [53] = HardFault_Handler,
            [54] = HardFault_Handler,
            [55] = HardFault_Handler,
            [56] = HardFault_Handler,
            [57] = HardFault_Handler,
            [58] = HardFault_Handler,
            [59] = HardFault_Handler,
            [60] = HardFault_Handler,
            [61] = HardFault_Handler,
            [62] = HardFault_Handler,
            [63] = HardFault_Handler,
            [64] = HardFault_Handler,
            [65] = HardFault_Handler,
            [66] = HardFault_Handler,
            [67] = HardFault_Handler,
            [68] = HardFault_Handler,
            [69] = HardFault_Handler,
            [70] = HardFault_Handler,
            [71] = HardFault_Handler,
            [72] = HardFault_Handler,
            [73] = HardFault_Handler,
            [74] = HardFault_Handler,
            [75] = HardFault_Handler,
            [76] = HardFault_Handler,
            [77] = HardFault_Handler,
            [78] = HardFault_Handler,
            [79] = HardFault_Handler,
            [80] = HardFault_Handler,
            [81] = HardFault_Handler,
            [82] = HardFault_Handler,
            [83] = HardFault_Handler,
            [84] = HardFault_Handler,
            [85] = HardFault_Handler,
            [86] = HardFault_Handler,
            [87] = HardFault_Handler,
            [88] = HardFault_Handler,
            [89] = HardFault_Handler,
            [90] = HardFault_Handler,
            [91] = HardFault_Handler,
            [92] = HardFault_Handler,
            [93] = HardFault_Handler,
            [94] = HardFault_Handler,
            [95] = HardFault_Handler,
            /* Aclara Added - End */
        };
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_MAX_ENTRIES] =
        {
            [0] = BSP_PRV_IELS_ENUM(EVENT_SCI2_RXI), /* SCI2 RXI (Received data full) */
            [1] = BSP_PRV_IELS_ENUM(EVENT_SCI2_TXI), /* SCI2 TXI (Transmit data empty) */
            [2] = BSP_PRV_IELS_ENUM(EVENT_SCI2_TEI), /* SCI2 TEI (Transmit end) */
            [3] = BSP_PRV_IELS_ENUM(EVENT_SCI2_ERI), /* SCI2 ERI (Receive error) */
            [4] = BSP_PRV_IELS_ENUM(EVENT_IIC0_RXI), /* IIC0 RXI (Receive data full) */
            [5] = BSP_PRV_IELS_ENUM(EVENT_IIC0_TXI), /* IIC0 TXI (Transmit data empty) */
            [6] = BSP_PRV_IELS_ENUM(EVENT_IIC0_TEI), /* IIC0 TEI (Transmit end) */
            [7] = BSP_PRV_IELS_ENUM(EVENT_IIC0_ERI), /* IIC0 ERI (Transfer error) */
            [8] = BSP_PRV_IELS_ENUM(EVENT_ICU_IRQ11), /* ICU IRQ11 (External pin interrupt 11) */
            [9] = BSP_PRV_IELS_ENUM(EVENT_SCI3_RXI), /* SCI3 RXI (Received data full) */
            [10] = BSP_PRV_IELS_ENUM(EVENT_SCI3_TXI), /* SCI3 TXI (Transmit data empty) */
            [11] = BSP_PRV_IELS_ENUM(EVENT_SCI3_TEI), /* SCI3 TEI (Transmit end) */
            [12] = BSP_PRV_IELS_ENUM(EVENT_SCI3_ERI), /* SCI3 ERI (Receive error) */
            [13] = BSP_PRV_IELS_ENUM(EVENT_SCI4_RXI), /* SCI4 RXI (Received data full) */
            [14] = BSP_PRV_IELS_ENUM(EVENT_SCI4_TXI), /* SCI4 TXI (Transmit data empty) */
            [15] = BSP_PRV_IELS_ENUM(EVENT_SCI4_TEI), /* SCI4 TEI (Transmit end) */
            [16] = BSP_PRV_IELS_ENUM(EVENT_SCI4_ERI), /* SCI4 ERI (Receive error) */
            [17] = BSP_PRV_IELS_ENUM(EVENT_SCI9_RXI), /* SCI9 RXI (Received data full) */
            [18] = BSP_PRV_IELS_ENUM(EVENT_SCI9_TXI), /* SCI9 TXI (Transmit data empty) */
            [19] = BSP_PRV_IELS_ENUM(EVENT_SCI9_TEI), /* SCI9 TEI (Transmit end) */
            [20] = BSP_PRV_IELS_ENUM(EVENT_SCI9_ERI), /* SCI9 ERI (Receive error) */
            [21] = BSP_PRV_IELS_ENUM(EVENT_RTC_ALARM), /* RTC ALARM (Alarm interrupt) */
            [22] = BSP_PRV_IELS_ENUM(EVENT_RTC_CARRY), /* RTC CARRY (Carry interrupt) */
            [23] = BSP_PRV_IELS_ENUM(EVENT_GPT1_COUNTER_OVERFLOW), /* GPT1 COUNTER OVERFLOW (Overflow) */
            [24] = BSP_PRV_IELS_ENUM(EVENT_GPT1_CAPTURE_COMPARE_A), /* GPT1 CAPTURE COMPARE A (Compare match A) */
            [25] = BSP_PRV_IELS_ENUM(EVENT_SPI1_RXI), /* SPI1 RXI (Receive buffer full) */
            [26] = BSP_PRV_IELS_ENUM(EVENT_SPI1_TXI), /* SPI1 TXI (Transmit buffer empty) */
            [27] = BSP_PRV_IELS_ENUM(EVENT_SPI1_TEI), /* SPI1 TEI (Transmission complete event) */
            [28] = BSP_PRV_IELS_ENUM(EVENT_SPI1_ERI), /* SPI1 ERI (Error) */
            [29] = BSP_PRV_IELS_ENUM(EVENT_FCU_FRDYI), /* FCU FRDYI (Flash ready interrupt) */
            [30] = BSP_PRV_IELS_ENUM(EVENT_FCU_FIFERR), /* FCU FIFERR (Flash access error interrupt) */
            [31] = BSP_PRV_IELS_ENUM(EVENT_AGT0_INT), /* AGT0 INT (AGT interrupt) */
            [32] = BSP_PRV_IELS_ENUM(EVENT_ICU_IRQ13), /* ICU IRQ13 (External pin interrupt 13) */
            [33] = BSP_PRV_IELS_ENUM(EVENT_AGT1_INT), /* AGT1 INT (AGT interrupt) */
            [34] = BSP_PRV_IELS_ENUM(EVENT_ICU_IRQ4), /* ICU IRQ4 (External pin interrupt 4) */
            [35] = BSP_PRV_IELS_ENUM(EVENT_ICU_IRQ14), /* ICU IRQ14 (External pin interrupt 14) */
            [36] = BSP_PRV_IELS_ENUM(EVENT_GPT2_COUNTER_OVERFLOW), /* GPT2 COUNTER OVERFLOW (Overflow) */
            [37] = BSP_PRV_IELS_ENUM(EVENT_GPT2_CAPTURE_COMPARE_B), /* GPT2 CAPTURE COMPARE B (Compare match B) */
        };
        #endif
