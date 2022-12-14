/***********************************************************************************************************************
 *
 * Filename: Task.c
 *
 * Global Designator: TASK_
 *
 * Contents:
 *
 ***********************************************************************************************************************
   A product of
   Aclara Technologies LLC
   Confidential and Proprietary
   Copyright 2012 - 2022 Aclara.  All Rights Reserved.

   PROPRIETARY NOTICE
   The information contained in this document is private to Aclara Technologies LLC an Ohio limited liability company
   (Aclara).  This information may not be published, reproduced, or otherwise disseminated without the express written
   authorization of Aclara.  Any software or firmware described in this document is furnished under a license and may be
   used or copied only in accordance with the terms of such license.
 **********************************************************************************************************************/

/* ****************************************************************************************************************** */
/* INCLUDE FILES */

#include "project.h"
#if ( RTOS_SELECTION == MQX_RTOS )
#include <mqx.h>
#include <fio.h>
#include <io_prv.h>
#include <charq.h>
#include <serinprv.h>
#include <bsp.h>
#include <mqx_prv.h>
#include "psp_cpudef.h"
#include "user_config.h"
#endif
//#include "sys_clock.h"
#include "DBG_SerialDebug.h"

#if ( ACLARA_LC != 1 ) && (ACLARA_DA != 1) /* meter specific code */
#if ENABLE_ALRM_TASKS
#include "EVL_event_log.h"
#include "ALRM_Handler.h"
#endif
#else
#include "EVL_event_log.h"
#endif
#include "EVL_event_log.h"
#include "DBG_CommandLine.h"
#include "MFG_Port.h"
#include "IDL_IdleProcess.h"

#if ENABLE_MFG_TASKS
#include "MFG_Port.h"
#endif

#include "timer_util.h"

#if ENABLE_TIME_SYS_TASKS
#include "time_sys.h"
#endif

#include "STRT_Startup.h"

#if ENABLE_HMC_TASKS
#include "hmc_app.h"
#endif

#if ENABLE_SRFN_ILC_TASKS
#include "ilc_dru_driver.h"
#include "ilc_tunneling_handler.h"
#include "ilc_srfn_reg.h"
#include "ilc_time_sync_handler.h"
#endif

#if (ENABLE_SRFN_DA_TASKS == 1)
#include "b2b.h"
#include "host_reset.h"
#endif

#include "demand.h"
#if ENABLE_ID_TASKS
#include "ID_intervalTask.h"
#endif

#if ENABLE_PAR_TASKS
#include "partitions.h"
#endif

#if ENABLE_PWR_TASKS
#include "pwr_task.h"
#include "pwr_last_gasp.h"
#include "pwr_restore.h"
#endif

#include "historyd.h"
#include "APP_MSG_Handler.h"
#if (USE_IPTUNNEL == 1)
#include "tunnel_msg_handler.h"
#endif
#if ENABLE_DFW_TASKS
#include "dfw_app.h"
#endif

#include "MAC_Protocol.h"
#include "MAC.h"

#include "STACK_Protocol.h"
#include "STACK.h"
#include "SM.h"
#include "PHY_Protocol.h"
#include "PHY.h"
#if ( RTOS_SELECTION == MQX_RTOS )
#include "stack_check.h"
#endif
#include "SoftDemodulator.h"
#include "SELF_test.h"
#include "dtls.h"

#if (SIGNAL_NW_STATUS == 1)
#include "nw_connected.h"
#endif

#if ( USE_MTLS == 1 )
#include "mtls.h"
#endif

#include "mode_config.h"
//#include "SoftDemodulator.h"

/*lint -esym(526,_kuart_polled_putc)  mqx putc routine for the current stdout stream  */
//extern void    _kuart_polled_putc(KUART_INFO_STRUCT_PTR, char);

/* ****************************************************************************************************************** */
/* #DEFINE DEFINITIONS */

/* ****************************************************************************************************************** */
/* MACRO DEFINITIONS */

#if ( RTOS_SELECTION == MQX_RTOS )
#define DEFAULT_ATTR       (MQX_FLOATING_POINT_TASK)                       /* All tasks save floating point on switch */
#define DEFAULT_ATTR_STRT  (AUTO_START_TASK|MQX_FLOATING_POINT_TASK)   /* Add the auto start attribute */
#elif (RTOS_SELECTION == FREE_RTOS)
#define DEFAULT_ATTR        (0)                                            /* All tasks save floating point on switch */
#define DEFAULT_ATTR_STRT   (AUTO_START_TASK)
#endif
#define TASK_CPULOAD_SIZE  10 // Keep track of the last 10 seconds
#define QUIET_MODE_ATTR    ((uint32_t)(1<<30))                             /* Task runs in quiet mode, also */
#define FAIL_INIT_MODE_ATTR ((uint32_t)(1<<29))                            /* Task runs even if init fails */
#define RFTEST_MODE_ATTR   ((uint32_t)(1<<28))                             /* Task runs in rfTest mode, also */


/* ****************************************************************************************************************** */
/* TYPE DEFINITIONS */

/*lint -esym(751,eOsTaskIndex_t) */
/* This enum list needs to match the MQX_template_list[] below */
/*lint --e{749}  Suppress enumerations that are not used for the next region. */
typedef enum
{
   eSELF_TSK_IDX = (uint32_t)0,  /* Unique value that can be used by a calling task for itself */
   eSTRT_TSK_IDX,                /* Auto start task */
   ePWR_TSK_IDX,
   ePWRLG_TSK_IDX,
   ePWROR_TSK_IDX,
   ePWRLG_IDL_TSK_IDX,
   eSD_PS_LISTENER_IDX,       // Soft Demodulator
   eSD_PREAM_DET_IDX,         //
   eSD_SYNC_PAYL_DEMOD1_IDX,  //
   eSD_SYNC_PAYL_DEMOD2_IDX,  //
   eSM_TSK_IDX,
   ePHY_TSK_IDX,
#if ( RTOS_SELECTION == FREE_RTOS )
   eTMR_SVC_IDX,              // Timer Service task that FreeRTOS creates by itself
#endif
   eTMR_TSK_IDX,
   eTIME_TSK_IDX,
   eMFGP_RECV_TSK_IDX,
#if ( ( OPTICAL_PASS_THROUGH != 0 ) && ( MQX_CPU == PSP_CPU_MK24F120M ) )
   eMFGP_OPTO_TSK_IDX,
#endif
   eHD_DS_TSK_IDX,
   eMAC_TSK_IDX,
   eDBG_TSK_IDX,
   eHMC_TSK_IDX,
   eDMD_TSK_IDX,
   eID_TSK_IDX,
   ePAR_TSK_IDX,
   eTEST_TSK_IDX,
   eDFW_TSK_IDX,
   eDBG_PRNT_TSK_IDX,
   eSTACK_TSK_IDX,
   eDTLS_TSK_IDX,
#if ( USE_MTLS == 1 )
   eMTLS_TSK_IDX,
#endif
   eAPP_TSK_IDX,
#if (USE_IPTUNNEL == 1)
   eTUN_TSK_IDX,
#endif
   eMFGP_CMD_TSK_IDX,
   eALRM_TSK_IDX,
   eBuALRM_TSK_IDX,
   eILC_DR_DR_TSK_IDX,
   eILC_SRFN_REG_TSK_IDX,
   eILC_TI_SY_TSK_IDX,
#if (ENABLE_SRFN_DA_TASKS == 1)
   eDA_SRFN_B2B_READ_TSK_IDX,
   eDA_HOST_RST_TSK_IDX,
#endif
#if (SIGNAL_NW_STATUS == 1)
   eNW_CON_TSK_IDX,
#endif
#if ( TM_CREATE_TWO_BLABBER_TASKS == 1 )
   eDBG_BLABBER1_IDX,
   eDBG_BLABBER2_IDX,
#endif
   eIDL_TSK_IDX,
#if ( RTOS_SELECTION == MQX_RTOS )
   eINT_TSK_IDX, // Keep that next to last. This is not really a task index. This is a place holder to keep track of CPU load for interrupt
#endif
   eLAST_TSK_IDX // Keep this last
}eOsTaskIndex_t;

static uint32_t   TASK_CPUload[eLAST_TSK_IDX][TASK_CPULOAD_SIZE];  // Keep track of the CPU load for each task for the last 10 seconds.
#if ( RTOS_SELECTION == MQX_RTOS )
static TD_STRUCT  *TASK_TD[eLAST_TSK_IDX];                         // Task descriptor list
#elif ( RTOS_SELECTION == FREE_RTOS )
static uint32_t   TASK_CPUtotal[TASK_CPULOAD_SIZE];                // Keep track of the CPU total load for the last 10 seconds
static uint32_t   Task_RunTimeCounters_[eLAST_TSK_IDX];            // Local Copy of Run Time Counters for every Task
static uint32_t   Task_PrevRunTimeCounters_[eLAST_TSK_IDX];        // Copy from the last pass
#endif
static uint32_t   cpuLoadIndex = 0;
static uint32_t   CPUTotal;

/* The following structure will be used to store task handle information for FreeRTOS. Each
   task handle will be tied to a specific task name as each task is created. When a task
   handle is needed, a table of these handles will be searched by matching a task name to
   find the corresponding handle. */
#if (RTOS_SELECTION == FREE_RTOS)
typedef struct
{
   char           *taskName;
   TaskHandle_t  taskHandle;
}taskHandleLookup_t;
#endif


/* ****************************************************************************************************************** */
/* CONSTANTS */

/* The Name fields below should have a MAX of 6 characters.  This list MUST match the extern list in OS_aclara.h. */

/*lint -esym(765,pTskName_Pwr,pTskName_PwrLastGasp,pTskName_PwrLastGaspIdls,pTskName_PwrRestore,pTskName_Strt) */
/*lint -esym(765,pTskName_Phy,pTskName_Tmr,pTskName_Time,pTskName_Mfg,pTskName_MfgUart,pTskName_Alrm,pTskName_BubLp) */
/*lint -esym(765,pTskName_HD_Ds,pTskName_Mac,pTskName_Dbg,pTskName_Hmc,pTskName_Par,pTskName_Test,pTskName_Id,pTskName_Dmd)*/
/*lint -esym(765,pTskName_Dfw,pTskName_Print,pTskName_ArmLB,pTskName_MacLB,pTskName_PhyMfg,pTskName_Nwk,pTskName_AppMsg) */
/*lint -esym(765,pTskName_Dtls,pTskName_Mtls,pTskName_Idle,pTskName_Strt,pTskName_LcDruDrv,pTskName_LcSrfnReg,pTskName_LcTimeSync) */
/*lint -esym(714,pTskName_Pwr,pTskName_PwrLastGasp,pTskName_PwrLastGaspIdls,pTskName_PwrRestore,pTskName_Strt) */
/*lint -esym(714,pTskName_Phy,pTskName_Tmr,pTskName_Time,pTskName_Mfg,pTskName_MfgUart,pTskName_Alrm,pTskName_BubLp) */
/*lint -esym(714,pTskName_HD_Ds,pTskName_Mac,pTskName_Dbg,pTskName_Hmc,pTskName_Par,pTskName_Test,pTskName_Id,pTskName_Dmd)*/
/*lint -esym(714,pTskName_Dfw,pTskName_Print,pTskName_ArmLB,pTskName_MacLB,pTskName_PhyMfg,pTskName_Nwk,pTskName_AppMsg) */
/*lint -esym(714,pTskName_Dtls,pTskName_Mtls,pTskName_Idle,pTskName_Strt,pTskName_LcDruDrv,pTskName_LcSrfnReg,pTskName_LcTimeSync) */
const char pTskName_Pwr[]           = "PWR";
const char pTskName_PwrLastGasp[]   = "PWRLG";
const char pTskName_PwrLastGaspIdls[] = "PWRLGI";
const char pTskName_PwrRestore[]    = "PWROR";
const char pTskName_Strt[]          = "STRT";
const char pTskName_Sm[]            = "SM";
const char pTskName_Phy[]           = "PHY";
const char pTskName_Tmr[]           = "TMR";
const char pTskName_Time[]          = "TIME";
const char pTskName_Mfg[]           = "MFG";
const char pTskName_MfgUartCmd[]    = "MFGC";
const char pTskName_MfgUartRecv[]   = "MFGR";
#if ( ( OPTICAL_PASS_THROUGH != 0 ) && ( MQX_CPU == PSP_CPU_MK24F120M ) )
const char pTskName_MfgUartOpto[]   = "MFGO";
#endif
#if ( ENABLE_METER_EVENT_LOGGING != 0 )
const char pTskName_Alrm[]          = "ALRM";
#endif
const char pTskName_BuAm[]          = "BUAM";
const char pTskName_BubLp[]         = "BUBLP";
const char pTskName_HD_Ds[]         = "HD_DS";
const char pTskName_Mac[]           = "MAC";
const char pTskName_Dbg[]           = "DBG";
const char pTskName_Hmc[]           = "HMC";
const char pTskName_Par[]           = "PAR";
const char pTskName_Test[]          = "TEST";
const char pTskName_Id[]            = "INTVL";
const char pTskName_Dmd[]           = "DMD";
const char pTskName_Dfw[]           = "DFW";
const char pTskName_Print[]         = "PRN";
const char pTskName_ArmLB[]         = "ARMLB";
const char pTskName_MacLB[]         = "MACLB";
const char pTskName_PhyMfg[]        = "PHYMFG";
const char pTskName_Nwk[]           = "NWK";
const char pTskName_AppMsg[]        = "APPMSG";
#if (USE_IPTUNNEL == 1)
const char pTskName_TunMsg[]        = "TUNMSG";
#endif
const char pTskName_Dtls[]          = "DTLS";
#if ( USE_MTLS == 1 )
const char pTskName_Mtls[]          = "MTLS";
#endif
#if ENABLE_FIO_TASKS
const char pTskName_Fio[]           = "FIO";
#endif
#if ENABLE_SRFN_ILC_TASKS
const char pTskName_LcDruDrv[]      = "LC_DD";
const char pTskName_LcSrfnReg[]     = "LC_SR";
const char pTskName_LcTimeSync[]    = "LC_TS";
#endif
#if (ENABLE_SRFN_DA_TASKS == 1)
const char pTskName_B2BRead[]       = "B2BRD";
const char pTskName_HostReset[]     = "HSTRST";
#endif
#if ( TM_CREATE_TWO_BLABBER_TASKS == 1 )
const char pTskName_DbgBalbber1[]    = "BLAB1";
const char pTskName_DbgBalbber2[]    = "BLAB2";
#endif
#if ( RTOS_SELECTION == MQX_RTOS )
const char pTskName_Idle[]          = "IDL";
#elif ( RTOS_SELECTION == FREE_RTOS )
const char pTskName_Idle[]          = "IDLE";
const char pTskName_TmrSvc[]        = "Tmr Svc";
#endif
#if (SIGNAL_NW_STATUS == 1)
const char pTskName_NwConn[]        = "NWCON";
#endif
static const char pTskName_SdPsListener[]        = "PSLSNR";
static const char pTskName_SdPreambleDetector[]  = "PREDET";
static const char pTskName_SdSyncPayloadDemod1[] = "DEMOD1";
static const char pTskName_SdSyncPayloadDemod2[] = "DEMOD2";
//static const char pTskName_TimeSys[]             = "TIMESYS"; // TODO: RA6E1 Bob: Not referenced.  Is this replaced by pTskName_Time[] = "TIME" ?

/* NOTE: The Highest Priority we should use is 9.  This excerpt was taken from AN3905.pdf on freesclale.com
   Task priority. Lower number is for higher priorities.  More than one task can have the same priority level.  Having
   no gaps between priority values improves performance */
/*lint -e{641}  Suppress the index conversion from enum to int for this section. */
const OS_TASK_Template_t  Task_template_list[] =
{
   /* Task Index,               Function,                    Stack, Pri, Name,                    Attributes,    Param, Time Slice */
   { eSTRT_TSK_IDX,             STRT_StartupTask,             1900,  13, (char *)pTskName_Strt,   DEFAULT_ATTR_STRT, 0, 0 },
#if ENABLE_PWR_TASKS
   { ePWR_TSK_IDX,              PWR_task,                     1000,  12, (char *)pTskName_Pwr,    DEFAULT_ATTR|QUIET_MODE_ATTR|RFTEST_MODE_ATTR, 0, 0 },
   { ePWROR_TSK_IDX,            PWROR_Task,                   1700,  12, (char *)pTskName_PwrRestore, DEFAULT_ATTR, 0, 0 },
#endif
#if ( RTOS_SELECTION == FREE_RTOS ) /* NOTE: This is a dummy entry for FreeRTOS used to print details of Timer Service task */
   { eTMR_SVC_IDX,              NULL,                            1,   0, (char *)pTskName_TmrSvc, 0, 0, 0 }, // Stack depth > 0 tells tasksummary to include
#endif
#if ENABLE_TMR_TASKS
   { eTMR_TSK_IDX,              TMR_HandlerTask,              1200,  14, (char *)pTskName_Tmr,    DEFAULT_ATTR|QUIET_MODE_ATTR|RFTEST_MODE_ATTR, 0, 0 },
#endif
   /* SELF_testTask lowers its priority to just above the IDLE task after its first iteration.  */
   { eTEST_TSK_IDX,             SELF_testTask,                1700,  14, (char *)pTskName_Test,   DEFAULT_ATTR, 0, 0 },
#if ENABLE_TIME_SYS_TASKS
   { eTIME_TSK_IDX,             TIME_SYS_HandlerTask,         1200,  15, (char *)pTskName_Time,   DEFAULT_ATTR|RFTEST_MODE_ATTR, 0, 0 },
#endif

#if ENABLE_MFG_TASKS
   { eMFGP_RECV_TSK_IDX,        MFGP_uartRecvTask,             700,  16, (char *)pTskName_MfgUartRecv, DEFAULT_ATTR|QUIET_MODE_ATTR|FAIL_INIT_MODE_ATTR|RFTEST_MODE_ATTR, 0, 0 },
#if ( ( OPTICAL_PASS_THROUGH != 0 ) && ( MQX_CPU == PSP_CPU_MK24F120M ) )
   { eMFGP_OPTO_TSK_IDX,        MFGP_optoTask,                 550,  16, (char *)pTskName_MfgUartOpto, DEFAULT_ATTR|QUIET_MODE_ATTR, 0, 0 },
#endif
#endif

#if 0 // TODO: RA6E1: Add when the OS Events issue has been resolved
   { eSD_PS_LISTENER_IDX,       SD_PhaseSamplesListenerTask,   600,  16, (char *)pTskName_SdPsListener,        DEFAULT_ATTR|RFTEST_MODE_ATTR, 0, 0 },
   { eSD_PREAM_DET_IDX,         SD_PreambleDetectorTask,      6300,  18, (char *)pTskName_SdPreambleDetector,  DEFAULT_ATTR|RFTEST_MODE_ATTR, 0, 0 }, // |  Parameters here are the ID's
   { eSD_SYNC_PAYL_DEMOD1_IDX,  SD_SyncPayloadDemodTask,     11500,  19, (char *)pTskName_SdSyncPayloadDemod1, DEFAULT_ATTR|RFTEST_MODE_ATTR, ( void * ) 1, 0 }, // |  ID's 0-2, run sequentially by the preprocessor
   { eSD_SYNC_PAYL_DEMOD2_IDX,  SD_SyncPayloadDemodTask,     11500,  19, (char *)pTskName_SdSyncPayloadDemod2, DEFAULT_ATTR|RFTEST_MODE_ATTR, ( void * ) 2, 0 }, // V  The preamble detector is always 0, regardles of what is passed
#endif

   { eSM_TSK_IDX,               SM_Task,                      1000,  20, (char *)pTskName_Sm,     DEFAULT_ATTR|RFTEST_MODE_ATTR, 0, 0 },
#if ( MCU_SELECTED == NXP_K24 )
   { ePHY_TSK_IDX,              PHY_Task,                     2100,  21, (char *)pTskName_Phy,    DEFAULT_ATTR|RFTEST_MODE_ATTR, 0, 0 },
#elif ( MCU_SELECTED == RA6E1 )
   { ePHY_TSK_IDX,              PHY_Task,                     3000,  21, (char *)pTskName_Phy,    DEFAULT_ATTR|RFTEST_MODE_ATTR, 0, 0 },
#endif
   { eMAC_TSK_IDX,              MAC_Task,                     1500,  22, (char *)pTskName_Mac,    DEFAULT_ATTR|RFTEST_MODE_ATTR, 0, 0 },
   { eSTACK_TSK_IDX,            NWK_Task,                     1500,  23, (char *)pTskName_Nwk,    DEFAULT_ATTR|RFTEST_MODE_ATTR, 0, 0 },

#if ENABLE_HMC_TASKS
   { eHMC_TSK_IDX,              HMC_APP_Task,                 1900,  24, (char *)pTskName_Hmc,    DEFAULT_ATTR, 0, 0 },
#endif
#if ENABLE_SRFN_ILC_TASKS
   { eILC_DR_DR_TSK_IDX,        ILC_DRU_DRIVER_Task,           900,  25, (char *)pTskName_LcDruDrv,     DEFAULT_ATTR, 0, 0 },
   { eILC_SRFN_REG_TSK_IDX,     ILC_SRFN_REG_Task,             650,  26, (char *)pTskName_LcSrfnReg,    DEFAULT_ATTR, 0, 0 },
   { eILC_TI_SY_TSK_IDX,        ILC_TIME_SYNC_Task,            650,  27, (char *)pTskName_LcTimeSync,   DEFAULT_ATTR, 0, 0 },
#endif
#if (ENABLE_SRFN_DA_TASKS == 1)
   { eDA_SRFN_B2B_READ_TSK_IDX, B2BRxTask,                    1100,  28, (char *)pTskName_B2BRead,      DEFAULT_ATTR, 0, 0 },
   { eDA_HOST_RST_TSK_IDX,      HostResetTask,                 600,  28, (char *)pTskName_HostReset,    DEFAULT_ATTR, 0, 0 },
#endif
#if ENABLE_MFG_TASKS
   { eMFGP_CMD_TSK_IDX,         MFGP_uartCmdTask,             2000,  29, (char *)pTskName_MfgUartCmd, DEFAULT_ATTR|QUIET_MODE_ATTR|FAIL_INIT_MODE_ATTR|RFTEST_MODE_ATTR, 0, 0 },
#endif

#if ( ENABLE_METER_EVENT_LOGGING != 0 )
#if ENABLE_ALRM_TASKS
#if ( MCU_SELECTED == NXP_K24 )
   { eALRM_TSK_IDX,             ALRM_RealTimeTask,            1100,  30, (char *)pTskName_Alrm,   DEFAULT_ATTR, 0, 0 },
#elif ( MCU_SELECTED == RA6E1 ) /* The RA6E1 does not have temperature in the chip so we must get it from the radio whihc uses more stack */
   { eALRM_TSK_IDX,             ALRM_RealTimeTask,            2000,  30, (char *)pTskName_Alrm,   DEFAULT_ATTR, 0, 0 },
#endif // MCU_SELECTED
#endif
#endif

#if ENABLE_ID_TASKS
   { eID_TSK_IDX,               ID_task,                      1900,  31, (char *)pTskName_Id,     DEFAULT_ATTR, 0, 0 },
#endif

#if ( ACLARA_LC != 1 ) && (ACLARA_DA != 1) /* meter specific code */
   { eDMD_TSK_IDX,              DEMAND_task,                  1200,  32, (char *)pTskName_Dmd,    DEFAULT_ATTR, 0, 0 },

   { eHD_DS_TSK_IDX,            HD_DailyShiftTask,            1600,  33, (char *)pTskName_HD_Ds,  DEFAULT_ATTR, 0, 0 },
#endif

#if ( TEST_QUIET_MODE == 0 )
   { eDBG_PRNT_TSK_IDX,         DBG_TxTask,                    680,  34, (char *)pTskName_Print,  DEFAULT_ATTR|FAIL_INIT_MODE_ATTR|RFTEST_MODE_ATTR, 0, 0 },
#else
   { eDBG_PRNT_TSK_IDX,         DBG_TxTask,                    680,  34, (char *)pTskName_Print,  DEFAULT_ATTR|QUIET_MODE_ATTR|FAIL_INIT_MODE_ATTR|RFTEST_MODE_ATTR, 0, 0 },
#endif
   { eDBG_TSK_IDX,              DBG_CommandLineTask,          2000,  35, (char *)pTskName_Dbg,    DEFAULT_ATTR|FAIL_INIT_MODE_ATTR|RFTEST_MODE_ATTR, 0, 0 },

#if ENABLE_PAR_TASKS
   { ePAR_TSK_IDX,              PAR_appTask,                   600,  36, (char *)pTskName_Par,    DEFAULT_ATTR, 0, 0 },
#endif
#if (SIGNAL_NW_STATUS == 1)
   { eNW_CON_TSK_IDX,           SM_NwState_Task,               400,  36, (char *)pTskName_NwConn,   DEFAULT_ATTR|RFTEST_MODE_ATTR, 0, 0 },
#endif
#if ENABLE_DFW_TASKS
   { eDFW_TSK_IDX,              DFWA_task,                    5400,  37, (char *)pTskName_Dfw,    DEFAULT_ATTR, 0, 0 },
#endif
#if ( USE_DTLS == 1 )
   { eDTLS_TSK_IDX,             DTLS_Task,                    5500,  38, (char *)pTskName_Dtls,   DEFAULT_ATTR|RFTEST_MODE_ATTR, 0, 0 },
#endif
#if ( USE_MTLS == 1 )
   { eMTLS_TSK_IDX,             MTLS_Task,                    1100,  38, (char *)pTskName_Mtls,   DEFAULT_ATTR|RFTEST_MODE_ATTR, 0, 0 },
#endif
   { eAPP_TSK_IDX,              APP_MSG_HandlerTask,          2400,  38, (char *)pTskName_AppMsg, DEFAULT_ATTR|RFTEST_MODE_ATTR, 0, 0 },
#if (USE_IPTUNNEL == 1)
   { eTUN_TSK_IDX,              TUNNEL_MSG_HandlerTask,        500,  38, (char *)pTskName_TunMsg, DEFAULT_ATTR, 0, 0 },
#endif
#if ENABLE_FIO_TASKS
   { eFIO_TSK_IDX,              FIO_Task,                     1000,  38, (char *)pTskName_Fio,    DEFAULT_ATTR, 0, 0 },
#endif
#if ( ENABLE_ALRM_TASKS == 1 )
   { eBuALRM_TSK_IDX,           EVL_AlarmHandlerTask,         1500,  38, (char *)pTskName_BuAm,   DEFAULT_ATTR, 0, 0 },
#endif
#if ( TM_CREATE_TWO_BLABBER_TASKS == 1 )
   { eDBG_BLABBER1_IDX,         DBG_BlabberTask1,             1000,  38, (char *)pTskName_DbgBalbber1, DEFAULT_ATTR, 0, 0 },
   { eDBG_BLABBER2_IDX,         DBG_BlabberTask2,             1000,  38, (char *)pTskName_DbgBalbber2, DEFAULT_ATTR, 0, 0 },
#endif
   /* should be the lowest priority tasks */
   // NOTE: MQX enforce a minimum stack size of 336 bytes even though less bytes are needed
#if ( RTOS_SELECTION == MQX_RTOS )
   { eIDL_TSK_IDX,              IDL_IdleTask,                  336,  39, (char *)pTskName_Idle,   DEFAULT_ATTR|QUIET_MODE_ATTR|FAIL_INIT_MODE_ATTR|RFTEST_MODE_ATTR, 0, 0 },
#elif ( RTOS_SELECTION == FREE_RTOS ) /* NOTE: This is a dummy entry for FreeRTOS used to print details of Idle Task conveniently */
   { eIDL_TSK_IDX,              NULL,                          0,    0, (char *)pTskName_Idle,   0, 0, 0 },
#endif
   { 0 }
};


/*lint -e{641}  Suppress the index conversion from enum to int for this section. */
const OS_TASK_Template_t  OS_template_list_last_gasp[] =
{
#if ENABLE_PWR_TASKS
   { ePWRLG_TSK_IDX,    PWRLG_Task,             1500,  10, (char*)pTskName_PwrLastGasp, DEFAULT_ATTR_STRT, 0, 0 },
#endif
#if ENABLE_TIME_SYS_TASKS
   { eTIME_TSK_IDX,     TIME_SYS_HandlerTask,   1200,  11, (char *)pTskName_Time,   DEFAULT_ATTR, 0, 0 },
#endif
   { eSM_TSK_IDX,       SM_Task,                1000,  12, (char *)pTskName_Sm,     DEFAULT_ATTR, 0, 0 },
   { ePHY_TSK_IDX,      PHY_Task,               1700,  13, (char *)pTskName_Phy,    DEFAULT_ATTR, 0, 0 },
   { eMAC_TSK_IDX,      MAC_Task,               1500,  14, (char *)pTskName_Mac,    DEFAULT_ATTR, 0, 0 },
   { eSTACK_TSK_IDX,    NWK_Task,               1500,  15, (char *)pTskName_Nwk,    DEFAULT_ATTR, 0, 0 },

#if ( USE_DTLS == 1 )
   { eDTLS_TSK_IDX,     DTLS_Task,              5400,  16, (char *)pTskName_Dtls,   DEFAULT_ATTR, 0, 0 },
#endif
   { eAPP_TSK_IDX,      APP_MSG_HandlerTask,    2400,  17, (char *)pTskName_AppMsg, DEFAULT_ATTR, 0, 0 },

   { eDBG_PRNT_TSK_IDX, DBG_TxTask,              680,  18, (char *)pTskName_Print,  DEFAULT_ATTR, 0, 0 },
#if ( RTOS_SELECTION == MQX_RTOS )
   // NOTE: MQX enforce a minimum stack size of 336 bytes even though less bytes are needed
   { ePWRLG_IDL_TSK_IDX,PWRLG_Idle_Task,         336,  19, (char *)pTskName_Idle,   DEFAULT_ATTR, 0, 0 },
#endif
   { 0 }
};

/* ****************************************************************************************************************** */
/* FILE VARIABLE DEFINITIONS */
#if (RTOS_SELECTION == FREE_RTOS)
static taskHandleLookup_t  taskHandleTable_[eLAST_TSK_IDX]; // table to store file handles matched to task name
static uint32_t            numberOfTasks = 0;                    /* Number of tasks from last update of taskHandle vector  */
//static OS_MUTEX_Obj        taskUsageMutex_;     //TODO: RA6E1 Bob: Remove before release  /* Access protection for the calculated values            */
//static bool                taskUsageMutexCreated_ = (bool)false; /* Flag saying whether the mutex was successfully created */
#endif

/* ****************************************************************************************************************** */
/* FUNCTION PROTOTYPES */
#if ( RTOS_SELECTION == MQX_RTOS )
void task_exception_handler( _mqx_uint para, void * stack_ptr );
#endif // ( RTOS_SELECTION == MQX_RTOS )
#if (RTOS_SELECTION == FREE_RTOS)
static TaskHandle_t * getFreeRtosTaskHandle( char const *pTaskName );
static uint32_t       setIdleTaskPriority ( uint32_t NewPriority );
#endif

/* ****************************************************************************************************************** */
/* FUNCTION DEFINITIONS */
#if ( RTOS_SELECTION == MQX_RTOS )
static void expt_frm_dump(void const * ext_frm_ptr)
{
   static const char * const expt_name[] =
   {
      "None",
      "Reset",
      "NMI",
      "HardFault",
      "MemManage",
      "BusFault",
      "UsageFault",
      "Rsvd",
      "Rsvd",
      "Rsvd",
      "Rsvd",
      "SVCall",
      "Debug Monitor",
      "Rsvd",
      "PendSV",
      "SysTick"
   };

   char                             pBuf[ 192 ];   /* Local buffer for printout  */
   uint16_t                         pOff;          /* offset into pBuf/length    */
   uint32_t                         i;             /* loop counter               */
   IO_SERIAL_INT_DEVICE_STRUCT_PTR  ioptr;         /* Needed to get proper UART interface */
   KUART_INFO_STRUCT_PTR            sci_info_ptr;  /* Needed to get proper UART interface */

   ioptr = (IO_SERIAL_INT_DEVICE_STRUCT_PTR)((void *)((FILE_DEVICE_STRUCT_PTR)stdout)->DEV_PTR->DRIVER_INIT_PTR);
   sci_info_ptr = ioptr->DEV_INFO_PTR;

   uint32_t excpt_num = __get_PSR() & 0x1FF;
   if(excpt_num < 16)
   {
      pOff =  (uint16_t)snprintf( pBuf,        (int32_t)sizeof( pBuf ), "\r\nExcpt [%s] in TASK 0x%x\r\n", expt_name[excpt_num] , _task_get_id() );
      pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "R0:  0x%08x\r\n", *( ( uint32_t * )ext_frm_ptr ) );
      pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "R1:  0x%08x\r\n", *( ( uint32_t * )ext_frm_ptr + 1 ) );
      pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "R2:  0x%08x\r\n", *( ( uint32_t * )ext_frm_ptr + 2 ) );
      pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "R3:  0x%08x\r\n", *( ( uint32_t * )ext_frm_ptr + 3 ) );
      pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "R12: 0x%08x\r\n", *( ( uint32_t * )ext_frm_ptr + 4 ) );
      pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "LR:  0x%08x\r\n", *( ( uint32_t * )ext_frm_ptr + 5 ) );
      pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "PC:  0x%08x\r\n", *( ( uint32_t * )ext_frm_ptr + 6 ) );
      pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "PSR: 0x%08x\r\n", *( ( uint32_t * )ext_frm_ptr + 7 ) );

      /* Additional debug registers (SHCSR, HFSR, CFSR, and BFAR) can be printed using the RA6E1 code below as an example.
      ** Use the K24 macros:  SCB_SHCSR, SCB_HFSR, SCB_CFSR, and SCB_BFAR to access the registers instead of SCB->register_name
      ** Note: 3 additional faults must be enabled at K24 startup (see RA6E1 hal_entry: R_BSP_WarmStart() for an example),
      ** e.g. add the line below:
      **    SCB_SHCSR |= SCB_SHCSR_USGFAULTENA_MASK | SCB_SHCSR_BUSFAULTENA_MASK | SCB_SHCSR_MEMFAULTENA_MASK;
      */
   }
   else
   {
      pOff = (uint16_t)snprintf( pBuf, (int32_t)sizeof( pBuf ), "External interrupt %u occured with no handler to serve it.\r\n", excpt_num );
   }
   for ( i = 0; i < pOff; i++ )
   {
      _kuart_polled_putc( sci_info_ptr, pBuf[ i ] );
   }
}
/*lint +esym(818, ext_frm_ptr)   */

/*lint -esym(818,stack_ptr)   could be pointer to const  */
void task_exception_handler(_mqx_uint para, void * stack_ptr)
{
    (void)para;
    (void)stack_ptr;

    void * expt_frm_ptr = (void *)__get_PSP();
    expt_frm_dump(expt_frm_ptr);
}
#elif ( RTOS_SELECTION == FREE_RTOS )
static void expt_frm_dump(void const * ext_frm_ptr)
{
   static const char * const expt_name[] =
   {
      "None",
      "Reset",
      "NMI",
      "HardFault",
      "MemManage",
      "BusFault",
      "UsageFault",
      "SecureFault",
      "Rsvd",
      "Rsvd",
      "Rsvd",
      "SVCall",
      "Debug Monitor",
      "Rsvd",
      "PendSV",
      "SysTick"
   };

   char        pBuf[ 192 ];                     /* Local buffer for printout  (must be < DEBUG_MSG_SIZE and < print size below) */
   uint16_t    pOff;                            /* offset into pBuf/length    */
   uint32_t    excpt_SHCSR = SCB->SHCSR;        /* capture SCB registers for printing later */
   uint32_t    excpt_HFSR  = SCB->HFSR;
   uint32_t    excpt_CFSR  = SCB->CFSR;
   uint32_t    excpt_BFAR  = SCB->BFAR;

   uint32_t excpt_num = __get_PSR() & 0x1FF;
   if(excpt_num < 16)
   {                                                                                                                                             //  chars -> running total
      pOff =  (uint16_t)snprintf( pBuf,     (int32_t)sizeof( pBuf ), "\nExcpt [%s] in TASK 0x%x \n", expt_name[excpt_num] , OS_TASK_GetId() );   // 23 + 13(max string) + 5(max task ID) -> 41
   }
   else
   {
      pOff =  (uint16_t)snprintf( pBuf,     (int32_t)sizeof( pBuf ), "\nUnused App Int [%d] in TASK 0x%x\n", excpt_num - 16 , OS_TASK_GetId() ); // 29 + 2(max int #) + 5(max task ID) = 36 < 41
   }
   pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "R0:  0x%08x\n", *( ( uint32_t * )ext_frm_ptr ) );                  // +17 -> 58
   pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "R1:  0x%08x\n", *( ( uint32_t * )ext_frm_ptr + 1 ) );              // +17 -> 75
   pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "R2:  0x%08x\n", *( ( uint32_t * )ext_frm_ptr + 2 ) );              // +17 -> 92
   pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "R3:  0x%08x\n", *( ( uint32_t * )ext_frm_ptr + 3 ) );              // +17 -> 109
   pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "R12: 0x%08x\n", *( ( uint32_t * )ext_frm_ptr + 4 ) );              // +17 -> 126
   pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "LR:  0x%08x\n", *( ( uint32_t * )ext_frm_ptr + 5 ) );              // +17 -> 143
   pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "PC:  0x%08x\n", *( ( uint32_t * )ext_frm_ptr + 6 ) );              // +17 -> 160
   pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "PSR: 0x%08x\n", *( ( uint32_t * )ext_frm_ptr + 7 ) );              // +17 -> 177
   printf( "%s", pBuf );

   /* print additional register debug data
    * System Handler Control and State Register:
    *    bit  Error
    *     0   MEMFAULTACTb (typically interested in this one)
    *     1   BUSFAULTACTb (typically interested in this one)
    *     3   USGFAULTACTb (typically interested in this one)
    *     7   SVCALLACTb
    *     8   MONITORACTb
    *    10   PENDSVACTb
    *    11   SYSTICKACTb
    *    12   USGFAULTPENDEDa
    *    13   MEMFAULTPENDEDa
    *    14   BUSFAULTPENDED
    *    15   SVCALLPENDEDa
    *    16   MEMFAULTENA (should be set)
    *    17   BUSFAULTENA (should be set)
    *    18   USGFAULTENA (should be set)
    * Hard Fault Status Register:
    *    bit  Error
    *     0
    *     1   VECTTBL  - Vector table read error
    *    30   FORCED   - Escalated exception priority to a Hard Fault (typically due to a SVCall when interrupts disabled)
    *    31   DEBUGEVT - Debug Event
    * Configurable Fault Status Register:
    *    bit  Error
    *     0   IACCVIOL        MemManage - Instruction Access Violation
    *     1   DACCVIOL        MemManage - Data Access Violation
    *     3   MUNSTKERR       MemManage - derived fault occurred on exception return
    *     4   MSTKERR         MemManage - derived fault occurred on exception entry
    *     5   MLSPERR         MemManage - fault occurred during FP lazy state preservation
    *     7   MMARVALID       MemManage - MMAR has valid contents
    *     8   IBUSERR         BusFault  - A bus fault on an instruction prefetch has occurred
    *     9   PRECISERR       BusFault  - Precise data access error has occurred
    *    10   IMPRECISERR     BusFault  - Imprecise data access error has occurred
    *    11   UNSTKERR        BusFault  - derived fault occurred on exception return
    *    12   STKERR          BusFault  - derived fault occurred on exception entry
    *    13   LSPERR          BusFault  - bus fault occurred during FP lazy state preservation
    *    15   BFARVALID       BusFault  - BFAR has valid contents
    *    16   UNDEFINSTR      UsageFault- Undefined instruction
    *    17   INVSTATE        UsageFault- Instruction executed with invalid EPSR.T or EPSR.IT field
    *    18   INVPC           UsageFault- Integrity check error on EXC_RETURN
    *    19   NOCP            UsageFault- CoProcessor access
    *    24   UNALIGNED       UsageFault- Unaligned access
    *    25   DIVBYZERO       UsageFault- Divide by zero
    * Bus Fault Address - ONLY valid if BFARVALID set
    */
                                                                                                                // chars -> running total
   pOff  = (uint16_t)snprintf( pBuf,        (int32_t)sizeof( pBuf ) - pOff, "SHCSR: 0x%08x\n", excpt_SHCSR );   //  +19 -> 19
   pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "HFSR:  0x%08x\n", excpt_HFSR );    //  +19 -> 38
   pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "CFSR:  0x%08x\n", excpt_CFSR );    //  +19 -> 57
   pOff += (uint16_t)snprintf( pBuf + pOff, (int32_t)sizeof( pBuf ) - pOff, "BFAR:  0x%08x\n", excpt_BFAR );    //  +19 -> 76

   // need Exclusion of RTOS to print
   printf( "%s", pBuf );

   /* Execute software reset */
   RESET();
}
/*lint +esym(818, ext_frm_ptr)   */

/*lint -esym(818,stack_ptr)   could be pointer to const  */
void HardFault_Handler( void )
{

   void * expt_frm_ptr = (void *)__get_PSP();
   expt_frm_dump(expt_frm_ptr);
}
#endif // RTOS_SELECTION

/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_Create_All
 *
 * Purpose: This function is used to start all of the tasks that were not auto started in the Task_template_list[] above
 *
* Arguments: bool initSuccess: true: All init functions successful, false otherwise
 *
 * Returns: None
 *
 * Side Effects:
 *
 * Reentrant Code:
 *
 * Notes:
 *
 **********************************************************************************************************************/
void OS_TASK_Create_All ( bool initSuccess )
{

   OS_TASK_Template_t const *pTaskList; /* Pointer to task list which contains all tasks in the system */
#if ( RTOS_SELECTION == MQX_RTOS )
   _task_id taskID;
#endif
   uint8_t  quiet;
   uint8_t  rfTest;

   quiet = MODECFG_get_quiet_mode();
   rfTest = MODECFG_get_rfTest_mode();

#if ( RTOS_SELECTION == MQX_RTOS )
   /* Install exception handler */
   (void)_int_install_exception_isr();
#elif ( RTOS_SELECTION == FREE_RTOS )
      // No equivalent operation for FreeRTOS.
#endif

   /*lint -e{641} converting enum to int  */
   for (pTaskList = &Task_template_list[0]; 0 != pTaskList->TASK_TEMPLATE_INDEX; pTaskList++)
   {  /* Create the task if the "Auto Start" attribute is NOT set */

      if (!(pTaskList->TASK_ATTRIBUTES & AUTO_START_TASK))
      {
         /* Create the task */
         if ( ( (quiet == 0) || ((pTaskList->TASK_ATTRIBUTES & QUIET_MODE_ATTR) != 0) ) &&
            ( (rfTest == 0) || ((pTaskList->TASK_ATTRIBUTES & RFTEST_MODE_ATTR) != 0) ) &&
            ( (initSuccess) || ((pTaskList->TASK_ATTRIBUTES & FAIL_INIT_MODE_ATTR) != 0) ) )
         {
#if ( RTOS_SELECTION == MQX_RTOS )
            if ( MQX_NULL_TASK_ID == (taskID = OS_TASK_Create(pTaskList) ) )
#elif (RTOS_SELECTION == FREE_RTOS)
            if( ( pTaskList->TASK_TEMPLATE_INDEX == eIDL_TSK_IDX ) ||
                ( pTaskList->TASK_TEMPLATE_INDEX == eTMR_SVC_IDX )    )
            {
               continue; // Skip the Idle Task and TmrSvr task. We are using the Idle Task created by FreeRTOS
            }
            if ( pdPASS != OS_TASK_Create(pTaskList) )
#endif
            {
               /* This condition should only show up in development.  This infinite loop should help someone figure out
                * that there is an issue creating a task.  Look at pTaskList->TASK_TEMPLATE_INDEX to figure out which task
                * was not created properly.  */
               while(true) /*lint !e716  */
#if ( RTOS_SELECTION == FREE_RTOS )
               {
                  OS_TASK_Sleep(500); /* If some task failed to start, blink tack-on LED at 1Hz forever or until IWDT gets us */
                  TEST_LED_TACKON_ON;
                  OS_TASK_Sleep(500);
                  TEST_LED_TACKON_OFF;
               }
#else
               {}  /* Todo:  We may wish to discuss the definition of LEDs.  Maybe we could add code here. */
#endif
            }
            // TODO: RA6: DG: Move these lines to OS_Task_Create

#if ( RTOS_SELECTION == MQX_RTOS )
            /* Set the exception handler of the task if still valid */
            if (MQX_NULL_TASK_ID != _task_get_id_from_name(pTaskList->TASK_NAME)) {
               (void)_task_set_exception_handler(taskID, task_exception_handler);
               stack_check_init(taskID);
            }
#elif (RTOS_SELECTION == FREE_RTOS)
            // RA6E1:Free RTOS not support the exception handling
#endif
         }
      }
   }
} /* end OS_TASK_Create_All () */


/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_Create
 *
 * Purpose: This function is used to create an individual task
 *
* Arguments: OS_TASK_Template_t const *pTaskList - pointer to item from the task template list
 *
 * Returns: taskCreateReturnValue_t - return value from RTOS create task call
 *
 * Side Effects:
 *
 * Reentrant Code:
 *
 * Notes:
 *
 **********************************************************************************************************************/
taskCreateReturnValue_t OS_TASK_Create ( OS_TASK_Template_t const *pTaskList )
{
#if (RTOS_SELECTION == MQX_RTOS)
   taskCreateReturnValue_t retVal = (taskCreateReturnValue_t)MQX_NULL_TASK_ID
#elif (RTOS_SELECTION == FREE_RTOS)
   taskCreateReturnValue_t retVal = (taskCreateReturnValue_t)pdFAIL;
#endif

   // ensure the task priority requested is an acceptable value
   ASSERT ( OS_MAX_TASK_PRIORITY >= pTaskList->uxPriority );
   ASSERT ( OS_MIN_TASK_PRIORITY <= pTaskList->uxPriority );

#if (RTOS_SELECTION == MQX_RTOS)
   retVal = _task_create( 0, pTaskList->TASK_TEMPLATE_INDEX, pTaskList->CREATION_PARAMETER );
#elif (RTOS_SELECTION == FREE_RTOS)
   retVal = xTaskCreate( pTaskList->pvTaskCode, pTaskList->pcName, pTaskList->usStackDepth/4, pTaskList->pvParameters,
                      FREE_RTOS_TASK_PRIORITY_CONVERT(pTaskList->uxPriority),
                      &taskHandleTable_[pTaskList->TASK_TEMPLATE_INDEX].taskHandle );

   if( pdPASS == retVal )
   {
      // update the task handle lookup table index location with the newly created task's name
      taskHandleTable_[pTaskList->TASK_TEMPLATE_INDEX].taskName = pTaskList->pcName;
   }
#endif

   return retVal;
}

/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_Get_Priority
 *
 * Purpose: This function will return the current Priority value for the requested Task
 *
 * Arguments: pTaskName - The name of the task (use extern values in OS_aclara.h "pTskName_")
 *
 * Returns: Priority - the current priority value for the requested task
 *
 * Side Effects:
 *
 * Reentrant Code:
 *
 * Notes: if the MQX function call fails, then Priority will be returned as 0
 *
 **********************************************************************************************************************/
uint32_t OS_TASK_Get_Priority ( char const *pTaskName )
{
   uint32_t Priority = 0;

#if (RTOS_SELECTION == MQX_RTOS)
   uint32_t RetStatus;

   if (NULL != pTaskName)
   {
      RetStatus = _task_get_priority ( _task_get_id_from_name((char *)pTaskName), &Priority );
   }
   else
   {
      RetStatus = _task_get_priority ( 0, &Priority );
   }
   if ( RetStatus != MQX_OK )
   {
      Priority = 0;
   } /* end if() */
#elif (RTOS_SELECTION == FREE_RTOS)
   if (NULL != pTaskName)
   {  // specific task requested
      TaskHandle_t *taskHandle = NULL;
      taskHandle = getFreeRtosTaskHandle( pTaskName );
      if( NULL != taskHandle )
      {  // found the task handle get the priority
         Priority = FREE_RTOS_TASK_PRIORITY_CONVERT(uxTaskPriorityGet(*taskHandle));
      }
      else
      {  // handle could not be found task template table, return 0
         Priority = (uint32_t)0;
      }
   }
   else
   {  // get info for current task
      Priority = FREE_RTOS_TASK_PRIORITY_CONVERT(uxTaskPriorityGet(NULL));
   }
#endif

   return ( (uint32_t)Priority );
} /* end OS_TASK_Get_Priority () */
/* ****************************************************************************************************************** */

/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_Set_Priority
 *
 * Purpose: This function will return the current Priority value for the requested Task
 *
 * Arguments: pTaskName   - The name of the task (use extern values in OS_aclara.h "pTskName_")
 *            NewPriority - new priority value for the task
 *
 * Returns: OldPriority - the current priority value for the requested task (before changing)
 *
 * Side Effects:
 *
 * Reentrant Code:
 *
 * Notes: if the MQX function call fails, then OldPriority will be returned as 0
 *
 **********************************************************************************************************************/
uint32_t OS_TASK_Set_Priority ( char const *pTaskName, uint32_t NewPriority )
{
   uint32_t OldPriority;

   // ensure priority value requested is valid
   ASSERT ( OS_MAX_TASK_PRIORITY >= pTaskList->uxPriority );
   ASSERT ( OS_MIN_TASK_PRIORITY <= pTaskList->uxPriority );

#if (RTOS_SELECTION == MQX_RTOS)
   uint32_t RetStatus;

   if (NULL != pTaskName)
   {
      RetStatus = _task_set_priority ( _task_get_id_from_name((char *)pTaskName), NewPriority, &OldPriority );
   }
   else
   {
      RetStatus = _task_set_priority ( 0, NewPriority, &OldPriority );
   }
   if ( RetStatus != MQX_OK )
   {
      OldPriority = 0;
   } /* end if() */
#elif (RTOS_SELECTION == FREE_RTOS)

   if(NULL != pTaskName)
   {  // specific task requested
      static TaskHandle_t *taskHandlePtr  = NULL;

      if( 0 == strcmp("IDLE", pTaskName ))  /* FreeRTOS creates the IDLE Task, hence its not in the list. Use different function */
      {
         OldPriority = setIdleTaskPriority( NewPriority );
      }
      else
      {
         taskHandlePtr = getFreeRtosTaskHandle( pTaskName );
         OldPriority = FREE_RTOS_TASK_PRIORITY_CONVERT( uxTaskPriorityGet(*taskHandlePtr) );
         if( NULL != taskHandlePtr )
         {
            vTaskPrioritySet( *taskHandlePtr, (UBaseType_t)FREE_RTOS_TASK_PRIORITY_CONVERT( NewPriority ) );
         }
         else
         {
            OldPriority = 0;
         }
      }
   }
   else
   {  // get info for current task
      OldPriority = FREE_RTOS_TASK_PRIORITY_CONVERT( uxTaskPriorityGet(NULL) );
      vTaskPrioritySet( NULL, (UBaseType_t)FREE_RTOS_TASK_PRIORITY_CONVERT( NewPriority ) );
   }
#endif /* MQX_RTOS */

   return ( (uint32_t)OldPriority );
} /* end OS_TASK_Set_Priority () */
/* ****************************************************************************************************************** */

#if (RTOS_SELECTION == FREE_RTOS)
/***********************************************************************************************************************
 *
 * Function Name: setIdleTaskPriority
 *
 * Purpose: To set the Idle Task Priority in FreeRTOS
 *
 * Arguments: NewPriority - new priority value for the task
 *
 * Returns: OldPriority - the current priority value for the requested task (before changing)
 *
 * Side Effects:
 *
 **********************************************************************************************************************/
static uint32_t setIdleTaskPriority ( uint32_t NewPriority )
{
   TaskHandle_t   IdleTaskHandle;
   uint32_t       OldIdlePriority;

   IdleTaskHandle = xTaskGetIdleTaskHandle();
   OldIdlePriority = FREE_RTOS_TASK_PRIORITY_CONVERT(uxTaskPriorityGet( IdleTaskHandle ));
   vTaskPrioritySet(IdleTaskHandle, (UBaseType_t)FREE_RTOS_TASK_PRIORITY_CONVERT( NewPriority ));

   return OldIdlePriority;

} /* end setIdleTaskPriority () */
/* ****************************************************************************************************************** */
#endif

/***********************************************************************************************************************
 *
 * Function Name: getFreeRtosTaskHandle
 *
 * Purpose: This function will return a pointer to the task handle located in the task template.  This handle is
 *          is used many calls into the Free RTOS task handling API.
 *
 * Arguments: pTaskName - string representing the name of a task
 *
 * Returns: TaskHandle_t * - pointer to the task handle reference from the task template
 *
 * Side Effects:
 *
 **********************************************************************************************************************/
#if (RTOS_SELECTION == FREE_RTOS)
static TaskHandle_t * getFreeRtosTaskHandle( char const *pTaskName )
{
   TaskHandle_t *retTaskHandlePtr = NULL; // return value, initialize to NULL and will get updated later

   for( uint8_t loopCtr = 0; ARRAY_IDX_CNT(taskHandleTable_); loopCtr++ )
   {  // loop through the task template list looking for a matching task name
      if( strcmp(taskHandleTable_[loopCtr].taskName, pTaskName) == 0)
      {  // we found a matching task name, assign the return value and break out of loop
         retTaskHandlePtr = &taskHandleTable_[loopCtr].taskHandle;
         break;
      }
   }
   return retTaskHandlePtr;
}
#endif

/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_Sleep
 *
 * Purpose: This function will pause the processing of the calling task by the passed in number of MilliSeconds
 *
 * Arguments: MSec - Time Delay in Milliseconds
 *
 * Returns: None
 *
 * Side Effects:
 *
 *
 **********************************************************************************************************************/
void OS_TASK_Sleep ( uint32_t MSec )
{
#if (RTOS_SELECTION == MQX_RTOS)
   _time_delay ( MSec );
#elif (RTOS_SELECTION == FREE_RTOS)
#if 1 // TODO: RA6E1 Bob: This is working properly.  Leaving #if in place in case we later find issues and need to revert.
   /* Increase the number of milliseconds by one tick (5msec) + 2msec for FreeRTOS. This results in delays being at
      least as long as requested by parameter MSec */
   uint32_t delayInTicks = pdMS_TO_TICKS( ( MSec + ( (uint32_t)1000 / (uint32_t)configTICK_RATE_HZ ) + 2U ) );
   vTaskDelay( delayInTicks );
#else
   vTaskDelay( pdMS_TO_TICKS(MSec) );
#endif // 1
#endif
}
/* ****************************************************************************************************************** */

/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_Exit
 *
 * Purpose: This function will kill/exit the calling task and free all memory associated with the task object
 *
 * Arguments: None
 *
 * Returns: None
 *
 * Side Effects: FREE_RTOS - Only memory allocated to a task by the kernel itself will be freed automatically
 * when the task is deleted. Any memory or other resource that the implementation of the task allocated must be
 * freed explicitly.
 *
 * Reentrant Code:
 *
 * Notes: Only the task calling this function can kill itself.  This does not allow a task to kill a different task.
 *
 **********************************************************************************************************************/
void OS_TASK_Exit ( void )
{
#if (RTOS_SELECTION == MQX_RTOS) /* MQX */
   (void)_task_destroy ( MQX_NULL_TASK_ID );
#elif (RTOS_SELECTION == FREE_RTOS)
   vTaskDelete( NULL ); /* NULL indicates the calling task */
#endif
}
/* ****************************************************************************************************************** */

/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_ExitId
 *
 * Purpose: This function will kill/exit the task with the task_id.
 *
 * Arguments: char *pTaskName - (use extern values in OS_aclara.h "pTskName_")
 *
 * Returns: None
 *
 * Side Effects: FREE_RTOS - Only memory allocated to a task by the kernel itself will be freed automatically
 * when the task is deleted. Any memory or other resource that the implementation of the task allocated must be
 * freed explicitly.
 *
 * Reentrant Code:
 *
 * Notes: If NULL is passed in the task calling this function can kill itself.
 *
 **********************************************************************************************************************/
void OS_TASK_ExitId ( char const *pTaskName )
{
#if (RTOS_SELECTION == MQX_RTOS) /* MQX */
   (void)_task_destroy ( _task_get_id_from_name((char *)pTaskName) );
#elif (RTOS_SELECTION == FREE_RTOS)
   TaskHandle_t *taskHandlePtr = NULL;
   taskHandlePtr = getFreeRtosTaskHandle( pTaskName );
   if(NULL != taskHandlePtr)
   {  // we found a task handle, delete the task
      vTaskDelete(*taskHandlePtr);
   }
#endif
} /* end OS_TASK_ExitId () */
/* ****************************************************************************************************************** */

/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_GetId
 *
 * Purpose: This function will get the ID of the calling task.
 *
 * Arguments: None
 *
 * Returns: OS_TASK_id
 *
 * Side Effects:
 *
 * Reentrant Code:
 *
 * Notes:
 *
 **********************************************************************************************************************/
OS_TASK_id OS_TASK_GetId (void)
{
#if (RTOS_SELECTION == MQX_RTOS) /* MQX */
   return(_task_get_id ());
#elif (RTOS_SELECTION == FREE_RTOS)
   TaskStatus_t taskStatusInfo;
   vTaskGetInfo(NULL, &taskStatusInfo, pdFALSE, eRunning );
   return taskStatusInfo.xTaskNumber;
#endif
} /* end OS_TASK_GetId () */

/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_GetID_fromName
 *
 * Purpose: This function will return the TaskID from the Name provided
 *
 * Arguments:
 *
 * Returns: OS_TASK_id TaskId
 *
 **********************************************************************************************************************/
OS_TASK_id OS_TASK_GetID_fromName ( const char *taskName )
{
#if (RTOS_SELECTION == MQX_RTOS) /* MQX */
   return ( _task_get_id_from_name( taskName ) );
#elif (RTOS_SELECTION == FREE_RTOS)
   TaskHandle_t  *pTaskHandle;
   TaskStatus_t  taskDetails;

   pTaskHandle = getFreeRtosTaskHandle( taskName );
   vTaskGetInfo( *pTaskHandle, &taskDetails, pdFALSE, eRunning );

   return ( taskDetails.xTaskNumber );
#endif
} /* end OS_TASK_GetID_fromName () */

/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_IsCurrentTask
 *
 * Purpose: This function will see if current task is the task name passed in.
 *
 * Arguments: char *pTaskName - (use extern values in OS_aclara.h "pTskName_")
 *
 * Returns: true if current task is the named task, otherwise returns false
 *
 * Side Effects:
 *
 * Reentrant Code:
 *
 * Notes:
 *
 **********************************************************************************************************************/
bool OS_TASK_IsCurrentTask ( char const *pTaskName )
{
   bool retVal = false;
#if (RTOS_SELECTION == MQX_RTOS) /* MQX */
   if ( OS_TASK_GetId() == _task_get_id_from_name( (char *)pTaskName ) )
   {
      retVal = true;
   }
#elif (RTOS_SELECTION == FREE_RTOS)
   TaskHandle_t taskHandle;
   taskHandle = xTaskGetCurrentTaskHandle();
   if( strcmp(pcTaskGetName(taskHandle) , pTaskName) == 0)
   {
      retVal = true;
   }
#endif
   return( retVal );
} /* end OS_TASK_IsCurrentTask () */

/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_GetTaskName
 *
 * Purpose: This function will return the name of the current Task
 *
 * Arguments:
 *
 * Returns: char *pTaskName
 *
 * Reentrant Code:
 *
 **********************************************************************************************************************/
char * OS_TASK_GetTaskName ( void )
{
#if (RTOS_SELECTION == MQX_RTOS) /* MQX */
   return ( _task_get_template_ptr( _task_get_id() )->TASK_NAME );
#elif (RTOS_SELECTION == FREE_RTOS)
   return ( pcTaskGetName( xTaskGetCurrentTaskHandle() ) );
#endif
} /* end OS_TASK_GetTaskName () */

#if ( RTOS_SELECTION == MQX_RTOS )
/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_UpdateCpuLoad
 *
 * Purpose: This function will update the CPU load counters for each task.
 *
 * Arguments: none
 *
 * Returns: Total CPU load for the last second in %
 *
 * Side Effects:
 *
 * Reentrant Code:
 *
 * Notes: This function should be call from the startup task every seconds for maximum accuracy
 *
 **********************************************************************************************************************/
uint32_t OS_TASK_UpdateCpuLoad ( void )
{
   OS_TASK_Template_t const *pTaskList; /* Pointer to task list which contains all tasks in the system */
   uint32_t                   CPULoad = 0;
   OS_TASK_id                 taskID;

   // Rebuild the TASK_TD array each time to account for tasks that could be dead or invalid.
   for ( pTaskList = Task_template_list; 0 != pTaskList->TASK_TEMPLATE_INDEX; pTaskList++ )
   {
      // Retrieve some task pointers
      // Make sure task ID is valid. Invalid number means task is dead.
      taskID = OS_TASK_GetID_fromName( pTaskList->pcName );
      if ( taskID ) {
         TASK_TD[pTaskList->TASK_TEMPLATE_INDEX] = _task_get_td( taskID );
      } else {
         TASK_TD[pTaskList->TASK_TEMPLATE_INDEX] = 0; // Mark task as bad
      }
   }

   CPUTotal = 0;
   cpuLoadIndex = (cpuLoadIndex+1)%TASK_CPULOAD_SIZE;

   TASK_CPUload[eINT_TSK_IDX][cpuLoadIndex] = 0; // Reset interrupt accumulator

   for ( pTaskList = Task_template_list; 0 != pTaskList->TASK_TEMPLATE_INDEX; pTaskList++ )
   {
      // Skip bad task
      if ( TASK_TD[pTaskList->TASK_TEMPLATE_INDEX] == 0 ) {
         continue;
      }

      // Save CPU load for this task without the time spent in the interrupt
      TASK_CPUload[pTaskList->TASK_TEMPLATE_INDEX][cpuLoadIndex] = TASK_TD[pTaskList->TASK_TEMPLATE_INDEX]->RUN_TIME - TASK_TD[pTaskList->TASK_TEMPLATE_INDEX]->INT_TIME;

      // Save time spent in interrupt
      TASK_CPUload[eINT_TSK_IDX][cpuLoadIndex] += TASK_TD[pTaskList->TASK_TEMPLATE_INDEX]->INT_TIME;

      // Only include the interrupt time for the IDLE task. The IDLE task run time is not counted toward the CPU load. Without doing this, the CPU load time would be %100
      if ( pTaskList->TASK_TEMPLATE_INDEX == (uint32_t)eIDL_TSK_IDX ) {
         CPULoad += TASK_TD[pTaskList->TASK_TEMPLATE_INDEX]->INT_TIME; // For idle task include only the interrupt time as part of the CPU load.
      } else {
         CPULoad += TASK_TD[pTaskList->TASK_TEMPLATE_INDEX]->RUN_TIME; // Compute total CPU load including intterupt time
      }
      CPUTotal += TASK_TD[pTaskList->TASK_TEMPLATE_INDEX]->RUN_TIME;// This is the task run time including any time spent in the interrupt handler
      TASK_TD[pTaskList->TASK_TEMPLATE_INDEX]->RUN_TIME = 0;        // Reset task run time
      TASK_TD[pTaskList->TASK_TEMPLATE_INDEX]->INT_TIME = 0;        // Reset time spent in interrupt
   }

   // Sanity check
   if ( CPULoad > CPUTotal ) {
      CPULoad = CPUTotal;
   }

   // Make sure we don't divide by 0
   if ( CPUTotal != 0 ) {
      CPULoad = (uint32_t)((((float)CPULoad*100)/CPUTotal)+0.5);
   }

   return CPULoad;

} /* end OS_TASK_UpdateCpuLoad () */

#elif ( RTOS_SELECTION == FREE_RTOS )
/***********************************************************************************************************************
                     FreeRTOS Version of OS_TASK_UpdateCpuLoad
***********************************************************************************************************************/
uint32_t OS_TASK_UpdateCpuLoad ( void )
{
#if ( GENERATE_RUN_TIME_STATS == 0 )
#warning "Function will NOT provide ulRunTimeCounter values without this feature"
#endif
   OS_TASK_Template_t const *pTaskList; /* Pointer to task list which contains all tasks in the system */
   uint32_t                 CPULoad = 0;
   TaskStatus_t             taskStatusInfo;

   // Rebuild the list of task handles only when number of tasks changes since this is an expensive operation in FreeRTOS
   uint32_t taskCount = uxTaskGetNumberOfTasks();
   if ( taskCount != numberOfTasks )
   {
      for ( pTaskList = Task_template_list; 0 != pTaskList->TASK_TEMPLATE_INDEX; pTaskList++ )
      {
         if( pTaskList->TASK_TEMPLATE_INDEX == eIDL_TSK_IDX ) /* Is this the IDLE task? */
         {
            /* Get the task Handle for FreeRTOS Idle task */
            taskHandleTable_[pTaskList->TASK_TEMPLATE_INDEX].taskHandle = xTaskGetIdleTaskHandle();
         }
         else if( pTaskList->TASK_TEMPLATE_INDEX == eTMR_SVC_IDX )
         {
            /* Save the task handle for this task in our list into a static vector for faster access */
            taskHandleTable_[pTaskList->TASK_TEMPLATE_INDEX].taskHandle = xTaskGetHandle( ( char* )pTaskList->pcName );
         }
#if 0 // TODO: DG: This section is not required because the TaskHandleTable_ is updated when the tasks are created. In addtion we can remove the for loop as well.
         else
         {
            /* Save the task handle for this task in our list into a static vector for faster access */
            taskHandleTable_[pTaskList->TASK_TEMPLATE_INDEX].taskHandle = xTaskGetHandle( ( char* )pTaskList->pcName );
         }
#endif
      }
      numberOfTasks = taskCount; /* Update the number of running tasks so we don't do the above every time */
   }

   for ( pTaskList = Task_template_list; 0 != pTaskList->TASK_TEMPLATE_INDEX; pTaskList++ )
   {
      Task_RunTimeCounters_[pTaskList->TASK_TEMPLATE_INDEX] = 0;
      if ( taskHandleTable_[pTaskList->TASK_TEMPLATE_INDEX].taskHandle != NULL )
      {
         vTaskGetInfo( taskHandleTable_[pTaskList->TASK_TEMPLATE_INDEX].taskHandle, &taskStatusInfo, pdFALSE, eInvalid );  // No need to get the HighWaterMark and Task Stat, which are time consuming
         if( 0 == strcmp( pTaskList->pcName, taskStatusInfo.pcTaskName ) )
         {
            Task_RunTimeCounters_[pTaskList->TASK_TEMPLATE_INDEX] = taskStatusInfo.ulRunTimeCounter;
         }
      }
   }

//   /* Lock the data table and CPUTotal to prevent a tasksummary command from getting inconsistent data */
//   if ( taskUsageMutexCreated_ == (bool)false )
//   {
//      taskUsageMutexCreated_ = OS_MUTEX_Create( &taskUsageMutex_ );
//   }
//   if ( taskUsageMutexCreated_ ) OS_MUTEX_Lock( &taskUsageMutex_ );

   CPUTotal = 0;
   cpuLoadIndex = (cpuLoadIndex+1)%TASK_CPULOAD_SIZE;

   for ( pTaskList = Task_template_list; 0 != pTaskList->TASK_TEMPLATE_INDEX; pTaskList++ )
   {
      // Save CPU load for this task without the time spent in the interrupt
      uint32_t deltaRunTimeCounter = ( Task_RunTimeCounters_    [pTaskList->TASK_TEMPLATE_INDEX] -
                                       Task_PrevRunTimeCounters_[pTaskList->TASK_TEMPLATE_INDEX]   );
      /* Check for the possibility of overflow and invert the wrapped around value if that occurred */
      if ( deltaRunTimeCounter > 10 * STRT_BACKGROUND_CYCLE_SECONDS * R_BSP_SourceClockHzGet(FSP_PRIV_CLOCK_SUBCLOCK))
      {
         deltaRunTimeCounter = (uint32_t)0xFFFFFFFF - deltaRunTimeCounter;
      }
      Task_PrevRunTimeCounters_[pTaskList->TASK_TEMPLATE_INDEX] = Task_RunTimeCounters_[pTaskList->TASK_TEMPLATE_INDEX];
      TASK_CPUload[pTaskList->TASK_TEMPLATE_INDEX][cpuLoadIndex] = deltaRunTimeCounter;

      // The IDLE task run time is not counted toward the CPU load. Without doing this, the CPU load time would be %100
      if( pTaskList->TASK_TEMPLATE_INDEX != eIDL_TSK_IDX )
      {
         CPULoad  += deltaRunTimeCounter; // Compute total CPU load
      }
      CPUTotal += deltaRunTimeCounter;                                      // This is the task run time including any time spent in the interrupt handler
   }
   TASK_CPUtotal[cpuLoadIndex] = CPUTotal;                                  // Save the CPU total value for subsequent calculations
//   if ( taskUsageMutexCreated_ ) OS_MUTEX_Unlock( &taskUsageMutex_ );       // TASK_CPUload, CPUTotal and cpuLoadIndex have all been updated and are consistent
   // Sanity check
   if ( CPULoad > CPUTotal ) {
      CPULoad = CPUTotal;
   }

   // Make sure we don't divide by 0
   if ( CPUTotal != 0 ) {
      CPULoad = (uint32_t)((((float)CPULoad*100)/CPUTotal)+0.5);
   }

   return CPULoad;
} /* end OS_TASK_UpdateCpuLoad () */
#endif

/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_GetCpuLoad
 *
 * Purpose: This function will retrieve the CPU loads for a given task.
 *
 * Arguments: taskIdx - Task Index
 *            CPULoad - Array of CPU load per task
 *
 * Returns: CPU loads
 *
 * Side Effects:
 *
 * Reentrant Code:
 *
 * Notes:
 *
 **********************************************************************************************************************/
#if ( RTOS_SELECTION == MQX_RTOS )
void OS_TASK_GetCpuLoad ( OS_TASK_id taskIdx, uint32_t * CPULoad )
{
   uint32_t i; // Loop counter

   // Check if this is to retrieve the CPU load interrupt
   if ( taskIdx == 0xFFFFFFFF) {
      taskIdx = (uint32_t)eINT_TSK_IDX;
   }
   for (i=0 ; i<TASK_CPULOAD_SIZE; i++) {
     // Retrieve CPU load
     // Make sure we don't divide by 0
     // TODO: K24 Bob: the calculation below uses CPUTotal for all historical entries.  This is mathematically incorrect.  See below for RA6E1 fix.
     if ( CPUTotal != 0 ) {
        CPULoad[i] = (uint32_t)((((float)TASK_CPUload[taskIdx][(cpuLoadIndex+TASK_CPULOAD_SIZE-i)%TASK_CPULOAD_SIZE]*1000)/CPUTotal)+0.5);
     } else {
        CPULoad[i] = 0;
     }
   }
}
#elif ( RTOS_SELECTION == FREE_RTOS )
/***********************************************************************************************************************
                     FreeRTOS Version of OS_TASK_GetCpuLoad
***********************************************************************************************************************/
void OS_TASK_GetCpuLoad ( OS_TASK_id taskIdx, uint32_t * CPULoad )
{
   uint32_t cpuTotal, i;
   /* Loop through the historical run time counters for this task.  The entry at [taskIdx][cpuLoadIndex] is the most recent one */
   for ( i = 0; i < TASK_CPULOAD_SIZE; i++ )
   {
      /* Get the total CPU run time counter summation for each 1 second period */
      cpuTotal = TASK_CPUtotal[ (cpuLoadIndex+TASK_CPULOAD_SIZE-i) % TASK_CPULOAD_SIZE ];
      /* Calculate the percent-times-10X for this task for each historical entry as the ratio of run time counter for this task to the total CPU */
      if ( cpuTotal > 0 )
      {
         CPULoad[i] = (uint32_t)((((float)TASK_CPUload[taskIdx][(cpuLoadIndex+TASK_CPULOAD_SIZE-i)%TASK_CPULOAD_SIZE]*1000)/cpuTotal)+0.5);
      } else {
         CPULoad[i] = 0; /* Handle the possibility that cpuTotal is zero, returning zero to avoid divide by zero */
      }
   }
}
#endif // RTOS_SELECTION

#if ( RTOS_SELECTION == MQX_RTOS )
/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_Summary
 *
 * Purpose: This function will print the state of tasks.
 *
 * Arguments: bool safePrint - True will use the debug logger.
 *
 * Returns:
 *
 * Side Effects:
 *
 * Reentrant Code:
 *
 * Notes:
 *
 **********************************************************************************************************************/
void OS_TASK_Summary ( bool safePrint )
{
   char                       str[40];
   char                       str2[150];
   TD_STRUCT                  *task_td;
   TASK_TEMPLATE_STRUCT const *pTaskList; /* Pointer to task list which contains all tasks in the system */
   char                       *state;
   uint32_t                   taskID;
   uint32_t                   CPULoad[TASK_CPULOAD_SIZE];
   uint32_t                   *pStack; // Stack pointer
   uint32_t                   percent=0;  //Percentage of the task/interrupt stack used
#if MQX_MONITOR_STACK && MQX_TD_HAS_STACK_LIMIT
   uint32_t                   *stackUsed; //Used to find high water mark in stack
#endif

   (void)snprintf( str2, (int32_t)sizeof(str2), "Task Name  Task ID Prio State                         Error Code LR        %%%%Stack  CPU load (last 10 seconds)              (Oldest)\n" );
   if (safePrint) {
      str2[strlen(str2)-1] = 0; // remove \n. It will be added back when printed
      DBG_printf( str2 );
   } else {
      DBG_LW_printf( str2 );
   }
   for ( pTaskList = MQX_template_list; 0 != pTaskList->TASK_TEMPLATE_INDEX; pTaskList++ )
   {
      taskID = OS_TASK_GetID_fromName( pTaskList->TASK_NAME );
      if ( taskID == 0 ) {
         continue; // Skip bad task
      }
      task_td = _task_get_td( taskID );
      // Find active task
      if ( OS_TASK_GetId() == taskID )
      {
         ( void )strcpy( str, "Active" );
      }
      else
      {
         switch ( task_td->STATE & STATE_MASK )
         {
            case ( READY & STATE_MASK ):
               state = "Ready";
               break;
            case ( BLOCKED & STATE_MASK ):
               state = "Blocked";
               break;
            case ( RCV_SPECIFIC_BLOCKED & STATE_MASK ):
               state = "Receive Specific Blocked";
               break;
            case ( RCV_ANY_BLOCKED & STATE_MASK ):
               state = "Receive Any Blocked";
               break;
            case ( DYING & STATE_MASK ):
               state = "Dying";
               break;
            case ( UNHANDLED_INT_BLOCKED & STATE_MASK ):
               state = "Unhandled Interrupt Blocked";
               break;
            case ( SEND_BLOCKED & STATE_MASK ):
               state = "Send Blocked";
               break;
            case ( BREAKPOINT_BLOCKED & STATE_MASK ):
               state = "Breakpoint Blocked";
               break;
            case ( IO_BLOCKED & STATE_MASK ):
               state = "IO Blocked";
               break;
            case ( SEM_BLOCKED & STATE_MASK ):
               state = "Semaphore Blocked";
               break;
            case ( MUTEX_BLOCKED & STATE_MASK ):
               state = "Mutex Blocked";
               break;
            case ( EVENT_BLOCKED & STATE_MASK ):
               state = "Event Blocked";
               break;
            case ( TASK_QUEUE_BLOCKED & STATE_MASK ):
               state = "Task Queue Blocked";
               break;
            case ( LWSEM_BLOCKED & STATE_MASK ):
               state = "LW Semaphore Blocked";
               break;
            case ( LWEVENT_BLOCKED & STATE_MASK ):
               state = "LW Event Blocked";
               break;
            default:
               state = "Unknown Task State";
               break;
         }

         // If task is ready but in timeout, che the print statements
         if ( task_td->STATE == ( READY | IS_ON_TIMEOUT_Q ) )
         {
            ( void )strcpy( str, "Time delay blocked" );
         }
         else
         {
            ( void )sprintf( str, "%s%s", state, task_td->STATE & IS_ON_TIMEOUT_Q ? ", timeout" : "" );
         }
      }

      pStack = task_td->STACK_PTR;

#if MQX_MONITOR_STACK && MQX_TD_HAS_STACK_LIMIT
      stackUsed = task_td->STACK_LIMIT;
      stackUsed++;
      while ( *stackUsed == MQX_STACK_MONITOR_VALUE )
      {
         stackUsed++;
      }
      if ( ( stackUsed == task_td->STACK_LIMIT ) || ( stackUsed == (uint32_t *)task_td->STACK_LIMIT + 1 ) )
      {
         percent = 100;
      }
      else
      {
         percent = ( (uint32_t)((uint32_t *)task_td->STACK_BASE - stackUsed ) ) * 100U / ( (uint32_t)((uint32_t *)task_td->STACK_BASE - (uint32_t *)task_td->STACK_LIMIT ) );
      }
#endif

      OS_TASK_GetCpuLoad( pTaskList->TASK_TEMPLATE_INDEX, CPULoad );
      (void)snprintf( str2, (int32_t)sizeof(str2), "%-10s 0x%05X %2u   %-29s 0x%05X    %08X  %3u%%%%   %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u\n",
                                     pTaskList->TASK_NAME,
                                     taskID,
                                     pTaskList->TASK_PRIORITY,
                                     str,
                                     task_td->TASK_ERROR_CODE,
                                     pStack[17],                  // Retrieve the Link Register of the task. This is where the task was preempted.
                                     percent,                      //Percentage of stack used
                                     CPULoad[0]/10, CPULoad[0]%10,
                                     CPULoad[1]/10, CPULoad[1]%10,
                                     CPULoad[2]/10, CPULoad[2]%10,
                                     CPULoad[3]/10, CPULoad[3]%10,
                                     CPULoad[4]/10, CPULoad[4]%10,
                                     CPULoad[5]/10, CPULoad[5]%10,
                                     CPULoad[6]/10, CPULoad[6]%10,
                                     CPULoad[7]/10, CPULoad[7]%10,
                                     CPULoad[8]/10, CPULoad[8]%10,
                                     CPULoad[9]/10, CPULoad[9]%10);
      if (safePrint) {
         str2[strlen(str2)-1] = 0; // remove \n. It will be added back when printed
         DBG_printf( str2 );
         // Throttle printing process because we don't have enough buffers
         OS_TASK_Sleep(TEN_MSEC); // About how long it takes to print one line
      } else {
         DBG_LW_printf( str2 );
      }
   }

#if MQX_MONITOR_STACK && MQX_TD_HAS_STACK_LIMIT
   KERNEL_DATA_STRUCT_PTR   kernel_data_ptr = _mqx_get_kernel_data ();
   uint32_t                 *stackLimit;

   stackUsed = stackLimit = (uint32_t *)( ( (uint32_t)( kernel_data_ptr->INTERRUPT_STACK_PTR ) ) - kernel_data_ptr->INIT.INTERRUPT_STACK_SIZE );  //Not in interrupt so PTR better be at base
   stackUsed++;
   while ( *stackUsed == MQX_STACK_MONITOR_VALUE )
   {
      stackUsed++;
   }
   if ( (stackUsed == stackLimit) || (stackUsed == stackLimit + 1) )
   {
      percent = 100;
   }
   else
   {
      percent = ( ( (uint32_t)kernel_data_ptr->INTERRUPT_STACK_PTR - (uint32_t)stackUsed ) * 100 ) / kernel_data_ptr->INIT.INTERRUPT_STACK_SIZE;
   }
#endif

   // Retrieve CPU interrupt load
   OS_TASK_GetCpuLoad( 0xFFFFFFFF, CPULoad );
   (void)snprintf( str2, (int32_t)sizeof(str2), "%-10s 0x%05x %2u   %-29s 0x%05X    %08X  %3u%%%%   %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u\n",
                                  "Interrupts",
                                  0,
                                  0,
                                  "Time spent in interrupts",
                                  0,
                                  0,                  // Retrieve the Link Register of the task. This is where the task was preempted.
                                  percent,
                                  CPULoad[0]/10, CPULoad[0]%10,
                                  CPULoad[1]/10, CPULoad[1]%10,
                                  CPULoad[2]/10, CPULoad[2]%10,
                                  CPULoad[3]/10, CPULoad[3]%10,
                                  CPULoad[4]/10, CPULoad[4]%10,
                                  CPULoad[5]/10, CPULoad[5]%10,
                                  CPULoad[6]/10, CPULoad[6]%10,
                                  CPULoad[7]/10, CPULoad[7]%10,
                                  CPULoad[8]/10, CPULoad[8]%10,
                                  CPULoad[9]/10, CPULoad[9]%10);
   if (safePrint) {
      str2[strlen(str2)-1] = 0; // remove \n. It will be added back when printed
      DBG_printf( str2 );
   } else {
      DBG_LW_printf( str2 );
   }
}
#endif // #if ( RTOS_SELECTION == MQX_RTOS )

#if ( RTOS_SELECTION == FREE_RTOS ) /* FREE_RTOS */
/***********************************************************************************************************************

   Function Name: OS_TASK_Summary

   Purpose: This function will print the Task Info for FreeRTOS

   Arguments: bool safePrint

   Returns: None

**********************************************************************************************************************/
void OS_TASK_Summary ( bool safePrint )
{
   int8_t                     index = 0;
   TaskStatus_t               taskStatusInfo;
   TaskHandle_t               taskHandle;
   OS_TASK_Template_t const   *pTaskList;
   uint32_t                   CPULoad[TASK_CPULOAD_SIZE];
   char taskState[6][10] = { "Running", "Ready", "Blocked", "Suspended", "Deleted", "Invalid" };
   char taskInformation[8][25] = { "TaskName", "TaskNumber", "TaskCurrentPriority", "TaskState", "TaskRunTimeCounter", "TaskStackDepth",
                                    "TaskStackHighWaterMark", "TaskStackBase" };
   char taskInfo[10][25] = { "TName", "TNumber", "TCP", "TState", "TRTC", "TSD", "TSHWM", "TSB", "CPU load (last 10 secs)","(Oldest)"};
   char buffer[150];
   static uint32_t passCounter = 0;

   if ( ( passCounter == 0 ) && ( safePrint ) ) /* Only print the dictionary of headings every 100 time and only if we are safe */
   {
      while( index < ((sizeof( taskInfo)/sizeof(taskInfo[0])) - 2) )  // Excluding the CPU Load Strings
      {
         if (safePrint)
         {
            DBG_printf( "%20s : %s" ,taskInfo[ index ], taskInformation[ index ] );
         }
         else
         {
            DBG_LW_printf( "%20s : %s" ,taskInfo[ index ], taskInformation[ index ] );
         }
         index++;
      }
      passCounter++;
      if ( passCounter >= 100) passCounter = 0;
   }
   snprintf( buffer, sizeof(buffer), "%-8s %4s %5s %8s %10s %5s %5s %10s %30s %15s\n", taskInfo[0], taskInfo[1], taskInfo[2], taskInfo[3], taskInfo[4],
                                      taskInfo[5], taskInfo[6], taskInfo[7], taskInfo[8], taskInfo[9] );
   if (safePrint)
   {
      buffer[strlen(buffer)-1] = 0; // remove \n. It will be added back when printed
      DBG_printf( buffer );
   }
   else
   {
      DBG_LW_printf( buffer );
   }
//   if ( safePrint ) /* Only use the mutex if this is a normal task summary printout rather than one occurring in a high duty cycle situation */
//   {
//      /* Lock the data table to prevent a tasksummary command from getting inconsistent data */
//      if ( taskUsageMutexCreated_ == (bool)false )
//      {
//         taskUsageMutexCreated_ = OS_MUTEX_Create( &taskUsageMutex_ );
//      }
//      if ( taskUsageMutexCreated_ ) OS_MUTEX_Lock( &taskUsageMutex_ ); /* Lock the CPULoad table and several other variables */
//   }
//   /* While under mutex protection, make a quick pass through the CPU load table to calculate values for each task */
//   for ( pTaskList = &Task_template_list[ 0 ]; 0 != pTaskList->TASK_TEMPLATE_INDEX; pTaskList++ )
//   {
//      OS_TASK_GetCpuLoad( pTaskList->TASK_TEMPLATE_INDEX, CPULoad ); /* Get last 10 CPU loads in percent-times-10 */
//   }
//   if ( safePrint )
//   {
//      if ( taskUsageMutexCreated_ ) OS_MUTEX_Unlock( &taskUsageMutex_ ); /* Release the lock on CPULoad table */
//   }
   /* Now we can "take our time" formatting the data for printing without holding the mutex.  However, with the current task priorities, this really  *
    * doesn't matter because the DBG task is higher priority than the STRT task.  So we really don't need the mutex in this function.                 */
   for ( pTaskList = &Task_template_list[ 0 ]; 0 != pTaskList->TASK_TEMPLATE_INDEX; pTaskList++ )
   {
      if( ( pTaskList->pvTaskCode != NULL )  || ( pTaskList->usStackDepth != 0 ) ) /* Only the IDLE task has both of these as 0 / NULL */
      {
         taskHandle = taskHandleTable_[pTaskList->TASK_TEMPLATE_INDEX].taskHandle;
      }
      else
      {
         taskHandle = xTaskGetIdleTaskHandle();
      }
      if ( taskHandle != NULL )
      {
         vTaskGetInfo( taskHandle, &taskStatusInfo, pdTRUE, eInvalid ); /* Collect all interesting information about this task */
         OS_TASK_GetCpuLoad( pTaskList->TASK_TEMPLATE_INDEX, CPULoad ); /* Get last 10 CPU loads in percent-times-10 */
         snprintf( buffer, sizeof(buffer), "%-8s %4d %8u %8s %10lu %5u %5u   0x%08X %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u %2u.%1u\n",
                  ( char* )pTaskList->pcName, taskStatusInfo.xTaskNumber, taskStatusInfo.uxCurrentPriority, ( char* )taskState[ taskStatusInfo.eCurrentState ],
                  taskStatusInfo.ulRunTimeCounter, pTaskList->usStackDepth,
                  ( taskStatusInfo.usStackHighWaterMark )*4, // Stack depth is divided by four while creating the task. So multiply StackHighWaterMark by four while print in debuglog
                  taskStatusInfo.pxStackBase,
                                  CPULoad[0]/10, CPULoad[0]%10,
                                  CPULoad[1]/10, CPULoad[1]%10,
                                  CPULoad[2]/10, CPULoad[2]%10,
                                  CPULoad[3]/10, CPULoad[3]%10,
                                  CPULoad[4]/10, CPULoad[4]%10,
                                  CPULoad[5]/10, CPULoad[5]%10,
                                  CPULoad[6]/10, CPULoad[6]%10,
                                  CPULoad[7]/10, CPULoad[7]%10,
                                  CPULoad[8]/10, CPULoad[8]%10,
                                  CPULoad[9]/10, CPULoad[9]%10);

         if (safePrint)
         {
            buffer[strlen(buffer)-1] = 0; // remove \n. It will be added back when printed
            DBG_printf( buffer );
            // Throttle printing process because we don't have enough buffers
            OS_TASK_Sleep(TEN_MSEC); // About how long it takes to print one line
         }
         else
         {
            DBG_LW_printf( buffer );
         }
      }
   }
}
#endif

#if ( RTOS_SELECTION == FREE_RTOS ) /* FREE_RTOS */
/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_Create_STRT
 *
 * Purpose: This function will Create the Startup Task in FreeRTOS
 *
 * Arguments: None
 *
 * Returns: None
 *
 * Notes:
 *
 **********************************************************************************************************************/
void OS_TASK_Create_STRT( void )
{
   // initialize the task handle lookup table, this table will be updated as we create each task
   (void)memset( (uint8_t *)&taskHandleTable_, 0, sizeof(taskHandleTable_) );
   if ( pdPASS != OS_TASK_Create( &Task_template_list[0] ) )
   {
      printf("Unable to create STRT");
   }
}

/***********************************************************************************************************************
 *
 * Function Name: OS_TASK_Create_PWRLG
 *
 * Purpose: This function will Create the PWRLG_Task in FreeRTOS
 *
 * Arguments: None
 *
 * Returns: None
 *
 * Notes:
 *
 **********************************************************************************************************************/
void OS_TASK_Create_PWRLG( void )
{
   // initialize the task handle lookup table, this table will be updated as we create each task
   (void)memset( (uint8_t *)&taskHandleTable_, 0, sizeof(taskHandleTable_) );

   if ( pdPASS != OS_TASK_Create( &OS_template_list_last_gasp[0] ) )
   {
      printf("Unable to create PWRLG_Task");
   }
}

/***********************************************************************************************************************
 *
 * Function Name: vApplicationStackOverflowHook
 *
 * Purpose: Application Hook for StackOverflow
 *
 * Arguments: TaskHandle_t xTask
 *            char * pcTaskName
 *
 * Returns: None
 *
 **********************************************************************************************************************/
void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
{
   printf("!!STACK OVERFLOW!! Task: %s ", pcTaskName );
}

#endif /* FREE_RTOS */