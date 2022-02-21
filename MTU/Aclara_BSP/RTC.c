/******************************************************************************
 *
 * Filename: RTC.c
 *
 * Global Designator:
 *
 * Contents:
 *
 ******************************************************************************
 * Copyright (c) 2022 ACLARA.  All rights reserved.
 * This program may not be reproduced, in whole or in part, in any form or by
 * any means whatsoever without the written permission of:
 *    ACLARA, ST. LOUIS, MISSOURI USA
 *****************************************************************************/

/* INCLUDE FILES */
#include <stdint.h>
#include <stdbool.h>
#if ( RTOS_SELECTION == 1 ) //( RTOS_SELECTION == MQX_RTOS )
  #include <mqx.h>
  #include <bsp.h>
  #include <rtc.h>
#elif ( RTOS_SELECTION == FREE_RTOS )
  #include "hal_data.h"
#endif
#include "BSP_aclara.h"
#include "vbat_reg.h"

/* #DEFINE DEFINITIONS */ 

/* MACRO DEFINITIONS */
/* Flags used for Alarm Interrupts */
#define SET_FLAG                             1
#define RESET_FLAG                           0
/* TYPE DEFINITIONS */

/* CONSTANTS */

/* FILE VARIABLE DEFINITIONS */
#if ( MCU_SELECTED == RA6E1 )
volatile uint32_t g_alarm_irq_flag = RESET_FLAG;       //flag to check occurrence of alarm interrupt
#endif
/* FUNCTION PROTOTYPES */
#if ( TM_RTC_UNIT_TEST == 1 )
bool RTC_UnitTest(void);
#endif
/* FUNCTION DEFINITIONS */
/*******************************************************************************

  Function name: accessBackupRegister

  Purpose: This function will Enable the write access to VBTBKR register

  Arguments: void

  Returns: void

*******************************************************************************/
static void accessBackupRegister( void )
{
   /*When accessed after Register Protection Enable, the entire BackUp registers are set to FF 
   so accessing it outside Register Protection has no affects on this register*/
   VBTBER_RTC_ACCESS = 1; 
}/* end accessBackupRegister () */
/*******************************************************************************

  Function name: RTC_Init

  Purpose: This function will Initialise the Real Time Clock peripheral

  Arguments: void

  Returns: void

*******************************************************************************/
returnStatus_t RTC_init( void )
{
   returnStatus_t retVal = eSUCCESS;
   fsp_err_t err = FSP_SUCCESS;
   err = R_RTC_Open( &g_rtc0_ctrl, &g_rtc0_cfg );
   if ( err!= FSP_SUCCESS )
   {
    retVal = eFAILURE;
    (void)printf ( "ERROR - RTC failed to init\n" ); /*Change to APP_Print or Debug Print*/
   }
   return( retVal ); 
} /* end RTC_init () */

/*******************************************************************************

  Function name: RTC_GetDateTime

  Purpose: This function will return the current Real Time Clock value

  Arguments: RT_Clock - structure that contains the Real Time Clock values

  Returns:

  Notes: There are two clock sources in this project - Real Time Clock, and an
         MQX timer/clock.  We are only modifying the RTC time with this module

*******************************************************************************/
void RTC_GetDateTime ( sysTime_dateFormat_t *RT_Clock )
{
#if ( MCU_SELECTED == NXP_K24 )
   DATE_STRUCT RTC_Time;
   TIME_STRUCT currentTime = { 0 };

   RTC_GetTimeAtRes (&currentTime, 1);
   (void)_time_to_date( &currentTime, &RTC_Time );

   RT_Clock->month = (uint8_t)RTC_Time.MONTH;
   RT_Clock->day   = (uint8_t)RTC_Time.DAY;
   RT_Clock->year  = (uint16_t)RTC_Time.YEAR;
   RT_Clock->hour  = (uint8_t)RTC_Time.HOUR;
   /*lint -e{123} min element same name as macro elsewhere is OK */
   RT_Clock->min   = (uint8_t)RTC_Time.MINUTE;
   RT_Clock->sec   = (uint8_t)RTC_Time.SECOND;
   RTC_Time.MILLISEC += (int16_t)SYS_TIME_TICK_IN_mS/2;  //Round up
   RT_Clock->msec  = (uint16_t)( ((uint16_t)RTC_Time.MILLISEC / SYS_TIME_TICK_IN_mS) * SYS_TIME_TICK_IN_mS ); //Normalize
#elif ( MCU_SELECTED == RA6E1 )
   fsp_err_t err = FSP_SUCCESS;
   rtc_time_t pTime;
   uint32_t sec = 0;
   uint32_t microSec = 0;
   err = R_RTC_CalendarTimeGet( &g_rtc0_ctrl, &pTime );
   if ( err!= FSP_SUCCESS )
   {
    (void)printf ( "ERROR - RTC failed to Get Time\n" );
   }
   RTC_GetTimeInSecMicroSec ( &sec , &microSec);
   RT_Clock->month = (uint8_t)pTime.tm_mon;
   RT_Clock->day   = (uint8_t)pTime.tm_mday;
   RT_Clock->year  = (uint16_t)pTime.tm_year;
   RT_Clock->hour  = (uint8_t)pTime.tm_hour;
   RT_Clock->min   = (uint8_t)pTime.tm_min;
   RT_Clock->sec   = (uint8_t)pTime.tm_sec;
   RT_Clock->msec  = (uint16_t)(microSec/1000); //Converting Microsecond to Millisecond
#endif

} /* end RTC_GetDateTime () */

/*******************************************************************************

  Function name: RTC_SetDateTime

  Purpose: This function will set the Real Time Clock to the passed in values

  Arguments: RT_Clock - structure that contains the Real Time Clock values

  Returns: FuncStatus - true if everything was successful, false if something failed

  Notes: There are two clock sources in this project - Real Time Clock, and an
         MQX timer/clock.  We are only modifying the RTC time with this module

*******************************************************************************/
bool RTC_SetDateTime ( const sysTime_dateFormat_t *RT_Clock )
{
   bool FuncStatus = true;
#if ( MCU_SELECTED == NXP_K24 )
   DATE_STRUCT mqxDate;    /* Used to convert requested date/time to seconds/milliseconds*/
   TIME_STRUCT mqxTime;    /* Converted date/time sent to set RTC time func.  */

   /* Validate the input date/time  */
   if ( (RT_Clock->month >= MIN_RTC_MONTH)
     && (RT_Clock->month <= MAX_RTC_MONTH)
     && (RT_Clock->day   >= MIN_RTC_DAY)
     && (RT_Clock->day   <= MAX_RTC_DAY)
     && (RT_Clock->year  >= MIN_RTC_YEAR)
     && (RT_Clock->year  <= MAX_RTC_YEAR)
     && (RT_Clock->hour  <= MAX_RTC_HOUR)
     /*lint -e{123} min element same name as macro elsewhere is OK */
     && (RT_Clock->min   <= MAX_RTC_MINUTE)
     && (RT_Clock->sec   <= MAX_RTC_SECOND) )
   {
      mqxDate.MONTH    = RT_Clock->month;
      mqxDate.DAY      = RT_Clock->day;
      mqxDate.YEAR     = (int16_t)RT_Clock->year;
      mqxDate.HOUR     = RT_Clock->hour;
      /*lint -e{123} min element same name as macro elsewhere is OK */
      mqxDate.MINUTE   = RT_Clock->min;
      mqxDate.SECOND   = RT_Clock->sec;
      mqxDate.MILLISEC = 0;

      (void)_time_from_date ( &mqxDate, &mqxTime );   /* Convert to seconds/milliseconds */
      (void)_rtc_set_time ( mqxTime.SECONDS );        /* Set the RTC time */
      _time_set( &mqxTime );

      if (0 == mqxTime.SECONDS)
      {  
         VBATREG_RTC_VALID = 0;
      }
      else
      {
         VBATREG_RTC_VALID = 1;
      }
   } /* end if() */
   else
   {
      /* Invalid Parameter passed in */
      (void)printf ( "ERROR - RTC invalid RT_Clock value passed into Set function\n" );
      FuncStatus = false;
   } /* end else() */

#elif ( MCU_SELECTED == RA6E1 )
   fsp_err_t err = FSP_SUCCESS;
   rtc_time_t getsec;
   rtc_time_t pTime;
   pTime.tm_sec = RT_Clock->sec;
   pTime.tm_mon = RT_Clock->month;
   pTime.tm_mday = RT_Clock->day;
   pTime.tm_year = (int16_t)RT_Clock->year;
   pTime.tm_hour = RT_Clock->hour;
   pTime.tm_min   = RT_Clock->min;
   /* No Requirement for millisecond */
   err = R_RTC_CalendarTimeSet( &g_rtc0_ctrl, &pTime );
   if ( err!= FSP_SUCCESS )
   {
    FuncStatus = false;
    (void)printf ( "ERROR - RTC failed to Set Time\n" );
   }
   getsec = pTime;
   if( 0 == getsec.tm_sec)
   {
    accessBackupRegister(); 
    /* Disable write protection on battery backup function. */
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_OM_LPC_BATT);
    VBATREG_RTC_VALID = 0; /*TODO Writing zero to VBTBER Register can happen in LastGasp considering Deep Software Standby Mode*/
    /* Enable write protection on battery backup function. */
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_OM_LPC_BATT);
   }/* end if() */
   else
   {
     accessBackupRegister();
    /* Disable write protection on battery backup function. */
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_OM_LPC_BATT);
    VBATREG_RTC_VALID = 1; /*TODO Writing zero to VBTBER Register can happen in LastGasp considering Deep Software Standby Mode*/
    /* Enable write protection on battery backup function. */
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_OM_LPC_BATT);
   }/* end else() */
#endif
   return ( FuncStatus );
} /* end RTC_SetDateTime () */

/*******************************************************************************

  Function name: RTC_Valid

  Purpose: To determine if the RTC is valid.

  Arguments: None

  Returns: RTCValid - True if the RTC is valid, otherwsie false.

  Notes: This is deterimed from two pieces of information.  From the
         TIF (Time Invalid Flag) in the RTC_SR (RTC Status Register)
         and from knowing whether or not the RTC has been set since
         the last time that TIF was true.

*******************************************************************************/
bool RTC_Valid(void)
{
   bool bRTCValid = false;

   if (1 == VBATREG_RTC_VALID)/*There is no RTC_SR in RA6E1*/
   {
      bRTCValid = true;
   }

   return (bRTCValid);
} /* end RTC_Valid () */

/*******************************************************************************

  Function name: RTC_GetTimeAtRes

  Purpose: This function will return the current Real Time Clock registers

  Arguments: ptime    - structure for the Real Time Clock values
             fractRes - fractional time resolution in milliseconds

  Returns:

  Notes: DO NOT try to debug through the loop reading the RTC registers. The RTC
         keeps running while dubugger stopped.

*******************************************************************************/
void RTC_GetTimeAtRes ( TIME_STRUCT *ptime, uint16_t fractRes )
{
#if ( MCU_SELECTED == NXP_K24 )
   uint32_t       seconds;
   uint16_t       fractSeconds;
   RTC_MemMapPtr  rtc;

   if ( ( fractRes != 0 ) && ( 1000 >= fractRes ) )
   {
      rtc          = RTC_BASE_PTR;
      seconds      = (uint32_t)rtc->TSR;
      fractSeconds = (uint16_t)rtc->TPR;
      /* Need to make sure no rollover between reads */
      /* NOTE: If SRFN Debug Macros not used, DO NOT try to debug through this loop. The RTC will keep running when dubugger stopped */
      while ( ( seconds != (uint32_t)rtc->TSR ) || ( fractSeconds != (uint32_t)rtc->TPR ) )
      {
         seconds      = (uint32_t)rtc->TSR;
         fractSeconds = (uint16_t)rtc->TPR;
      }
      ptime->SECONDS      = seconds;
      ptime->MILLISECONDS = ( ( (uint32_t)fractSeconds * (uint32_t)1000 / (uint32_t)fractRes ) / 32768 ) * fractRes;
   }
   return;  
#endif
   
} /* end RTC_GetDateTime () */


/*******************************************************************************

  Function name: RTC_GetTimeInSecMicroSec

  Purpose: Return the current Time in seconds and microseconds

  Arguments: uint32_t *sec - Seconds
             uint32_t *microSec - Micro-seconds

  Returns: None

  Notes: DO NOT try to debug through the loop reading the RTC registers. The RTC
         keeps running while dubugger stopped.

*******************************************************************************/
void RTC_GetTimeInSecMicroSec ( uint32_t *sec, uint32_t *microSec )
{
#if ( MCU_SELECTED == NXP_K24 )
   uint32_t       seconds;
   uint64_t       fractSeconds;
   RTC_MemMapPtr  rtc;

   rtc          = RTC_BASE_PTR;
   seconds      = (uint32_t)rtc->TSR;
   fractSeconds = (uint64_t)rtc->TPR;
   /* Need to make sure no rollover between reads */
   /* NOTE: If SRFN Debug Macros not used, DO NOT try to debug through this loop. The RTC will keep running when dubugger stopped */
   while ( ( seconds != (uint32_t)rtc->TSR ) || ( fractSeconds != (uint64_t)rtc->TPR ) )
   {
      seconds      = (uint32_t)rtc->TSR;
      fractSeconds = (uint64_t)rtc->TPR;
   }
   *sec = seconds;
   *microSec = (uint32_t)((fractSeconds * 1000000) / 32768);

   return;
#elif ( MCU_SELECTED == RA6E1 )
   fsp_err_t err = FSP_SUCCESS;
   rtc_time_t getSecTime;
   err = R_RTC_CalendarTimeGet( &g_rtc0_ctrl, &getSecTime );
   if ( err!= FSP_SUCCESS )
   {
    (void)printf ( "ERROR - RTC failed to Get Time in RTC_GetTimeInSecMicroSec function\n" );
   }
   *sec = getSecTime.tm_sec;
   rtc_instance_ctrl_t * p_instance_ctrl = ( rtc_instance_ctrl_t * ) &g_rtc0_ctrl;
   uint8_t milliSecGet;
   //Disable the NVIC carry interrupt request
   NVIC_DisableIRQ( p_instance_ctrl->p_cfg->carry_irq );
   //Enable the RTC carry interrupt request
   R_BSP_IrqEnable( p_instance_ctrl->p_cfg->carry_irq );

   /* If a carry occurs while the 64-Hz counter and time are being read, the correct time is not obtained,
    * therefore they must be read again. 23.3.5 "Reading 64-Hz Counter and Time" of the RA6E1 manual R01UH0930EJ0100)*/
   do
   {
       p_instance_ctrl->carry_isr_triggered = false; /** This flag will be set to 'true' in the carry ISR */
       milliSecGet = (int8_t) R_RTC->R64CNT;
       *microSec = ( (milliSecGet)*1000000 )/128;
   } while ( p_instance_ctrl->carry_isr_triggered );
   //Enable the NVIC carry interrupt request
   NVIC_EnableIRQ( p_instance_ctrl->p_cfg->carry_irq );
#endif
   
}/* end RTC_GetTimeInSecMicroSec () */

/*******************************************************************************

  Function name: RTC_SetAlarmTime

  Purpose: Sets the Alarm Time

  Arguments: pAlarm - structure that contains the Real Time Clock Alarm 
             configuration

  Returns: None

*******************************************************************************/
bool RTC_SetAlarmTime ( rtc_alarm_time_t * const pAlarm )
{
   bool FuncStatus = true;
   fsp_err_t err = FSP_SUCCESS;
   err = R_RTC_CalendarAlarmSet( &g_rtc0_ctrl, pAlarm );
   if ( err!= FSP_SUCCESS )
   {
    FuncStatus = false;
    (void)printf ( "ERROR - RTC failed to Set Alarm\n" );
   }
   return ( FuncStatus );
}/* end RTC_SetAlarmTime () */

/*******************************************************************************

  Function name: RTC_GetAlarmTime

  Purpose: Gets the current Alarm Time

  Arguments: pAlarm - structure that contains the Real Time Clock Alarm 
             configuration

  Returns: None

*******************************************************************************/
void RTC_GetAlarmTime ( rtc_alarm_time_t * const pAlarm )
{
   fsp_err_t err = FSP_SUCCESS;
   err = R_RTC_CalendarAlarmGet( &g_rtc0_ctrl, pAlarm );
   if ( err!= FSP_SUCCESS )
   {
    (void)printf ( "ERROR - RTC failed to Get Alarm\n" );
   }
}/* end RTC_GetAlarmTime () */

/*******************************************************************************

  Function name: RTC_ErrorAdjustmentSet

  Purpose: RTC Error adjustment

  Arguments: erradjcfg - structure that contains the Real Time Clock error 
             adjustment configuration

  Returns: None

*******************************************************************************/
void RTC_ErrorAdjustmentSet( rtc_error_adjustment_cfg_t const * const erradjcfg )
{
   fsp_err_t err = FSP_SUCCESS;
   err = R_RTC_ErrorAdjustmentSet( &g_rtc0_ctrl, erradjcfg );
   if ( err!= FSP_SUCCESS )
   {
    (void)printf ( "ERROR - RTC failed to Set Error Adjustment\n" );
   }
}/* end RTC_ErrorAdjustmentSet () */


/*******************************************************************************

  Function name: rtc_callback

  Purpose: Interrupt Handler for RTC Module

  Arguments: rtc_callback_args_t *p_args

  Returns: None

  Notes: Used for Alarm Interrupt.

*******************************************************************************/
void rtc_callback( rtc_callback_args_t *p_args )
{
   if( RTC_EVENT_ALARM_IRQ == p_args->event )
   {
       g_alarm_irq_flag = SET_FLAG; //Alarm Interrupt occured
   }/* end if */
}/* end rtc_callback () */


   #if ( TM_RTC_UNIT_TEST == 1 )
/*******************************************************************************

  Function name: RTC_UnitTest

  Purpose: This function will test RTC

  Arguments: None

  Returns: bool - 0 if everything was successful, 1 if something failed

*******************************************************************************/
// TODO: RA6 [name_Balaji]: Move to SelfTest Task
bool RTC_UnitTest(void)
{
   uint16_t retVal = 0;
   uint32_t Sec = 0;
   uint32_t MicroSec = 0;
   sysTime_dateFormat_t set_time =
   {
   .sec  = 55,
   .min  = 59,
   .hour = 23,       //24-HOUR mode       
   .day = 31,        //RDAYCNT
   .month  = 11,     //RMONCNT 0-Jan 11 Dec
   .year = 121       //RYRCNT //Year SINCE 1900 (2021 = 2021-1900 = 121)
   };
   sysTime_dateFormat_t get_time;
   rtc_alarm_time_t alarm_set_time =
   {
        .sec_match        =  true,
        .min_match        =  true,
        .hour_match       =  false,
        .mday_match       =  false,
        .mon_match        =  false,
        .year_match       =  false,
        .dayofweek_match  =  false
   };
   bool isTimeSetSuccess;
   bool isAlarmSetSuccess;
   bool isRTCValid;
   isTimeSetSuccess = RTC_SetDateTime (&set_time);
   if(isTimeSetSuccess == 0)
   {
     retVal = 1;
   }
   isRTCValid = RTC_Valid();
   if(isRTCValid == false)
   {
     retVal = 1;
   }
   RTC_GetDateTime (&get_time);
   if(get_time.min != 59)
   {
     retVal = 1;
   }
   RTC_GetTimeInSecMicroSec ( &Sec , &MicroSec);
   if(Sec == 0)
   {
     retVal = 1;
   }
   if(MicroSec == 0)
   {
     retVal = 1;
   }
   alarm_set_time.time.tm_sec = 10;
   alarm_set_time.time.tm_min = 59;
   isAlarmSetSuccess = RTC_SetAlarmTime (&alarm_set_time);
   if(isAlarmSetSuccess == 0)
   {
     retVal = 1;
   }
   return retVal;
}    
#endif