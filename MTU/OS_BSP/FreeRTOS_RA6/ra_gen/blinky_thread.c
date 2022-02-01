/* generated thread source file - do not edit */
#include "blinky_thread.h"

#if 0
                static StaticTask_t blinky_thread_memory;
                #if defined(__ARMCC_VERSION)           /* AC6 compiler */
                static uint8_t blinky_thread_stack[600] BSP_PLACE_IN_SECTION(BSP_UNINIT_SECTION_PREFIX ".stack.thread") BSP_ALIGN_VARIABLE(BSP_STACK_ALIGNMENT);
                #else
                static uint8_t blinky_thread_stack[600] BSP_PLACE_IN_SECTION(BSP_UNINIT_SECTION_PREFIX ".stack.blinky_thread") BSP_ALIGN_VARIABLE(BSP_STACK_ALIGNMENT);
                #endif
                #endif
                TaskHandle_t blinky_thread;
                void blinky_thread_create(void);
                static void blinky_thread_func(void * pvParameters);
                void rtos_startup_err_callback(void * p_instance, void * p_data);
                void rtos_startup_common_init(void);
icu_instance_ctrl_t g_sw1_irq1_ctrl;
const external_irq_cfg_t g_sw1_irq1_cfg =
{
    .channel             = 1,
    .trigger             = EXTERNAL_IRQ_TRIG_BOTH_EDGE,
    .filter_enable       = true,
    .pclk_div            = EXTERNAL_IRQ_PCLK_DIV_BY_64,
    .p_callback          = sw1_irq_Handler,
    /** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
    .p_context           = &NULL,
#endif
    .p_extend            = NULL,
    .ipl                 = (1),
#if defined(VECTOR_NUMBER_ICU_IRQ1)
    .irq                 = VECTOR_NUMBER_ICU_IRQ1,
#else
    .irq                 = FSP_INVALID_VECTOR,
#endif
};
/* Instance structure to use this module. */
const external_irq_instance_t g_sw1_irq1 =
{
    .p_ctrl        = &g_sw1_irq1_ctrl,
    .p_cfg         = &g_sw1_irq1_cfg,
    .p_api         = &g_external_irq_on_icu
};
extern uint32_t g_fsp_common_thread_count;

                const rm_freertos_port_parameters_t blinky_thread_parameters =
                {
                    .p_context = (void *) NULL,
                };

                void blinky_thread_create (void)
                {
                    /* Increment count so we will know the number of threads created in the RA Configuration editor. */
                    g_fsp_common_thread_count++;

                    /* Initialize each kernel object. */


                    #if 0
                    blinky_thread = xTaskCreateStatic(
                    #else
                    BaseType_t blinky_thread_create_err = xTaskCreate(
                    #endif
                        blinky_thread_func,
                        (const char *)"Blinky Thread",
                        600/4, // In words, not bytes
                        (void *) &blinky_thread_parameters, //pvParameters
                        1,
                        #if 0
                        (StackType_t *)&blinky_thread_stack,
                        (StaticTask_t *)&blinky_thread_memory
                        #else
                        & blinky_thread
                        #endif
                    );

                    #if 0
                    if (NULL == blinky_thread)
                    {
                        rtos_startup_err_callback(blinky_thread, 0);
                    }
                    #else
                    if (pdPASS != blinky_thread_create_err)
                    {
                        rtos_startup_err_callback(blinky_thread, 0);
                    }
                    #endif
                }
                static void blinky_thread_func (void * pvParameters)
                {
                    /* Initialize common components */
                    rtos_startup_common_init();

                    /* Initialize each module instance. */


                    #if (1 == BSP_TZ_NONSECURE_BUILD) && (1 == 0)
                    /* When FreeRTOS is used in a non-secure TrustZone application, portALLOCATE_SECURE_CONTEXT must be called prior
                     * to calling any non-secure callable function in a thread. The parameter is unused in the FSP implementation.
                     * If no slots are available then configASSERT() will be called from vPortSVCHandler_C(). If this occurs, the
                     * application will need to either increase the value of the "Process Stack Slots" Property in the rm_tz_context
                     * module in the secure project or decrease the number of threads in the non-secure project that are allocating
                     * a secure context. Users can control which threads allocate a secure context via the Properties tab when
                     * selecting each thread. Note that the idle thread in FreeRTOS requires a secure context so the application
                     * will need at least 1 secure context even if no user threads make secure calls. */
                     portALLOCATE_SECURE_CONTEXT(0);
                    #endif

                    /* Enter user code for this thread. Pass task handle. */
                    blinky_thread_entry(pvParameters);
                }
