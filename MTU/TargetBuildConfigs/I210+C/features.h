/******************************************************************************
 *
 * Filename: features.h
 *
 * Contents: Defines the options/features for the I-210+C
 *
 ******************************************************************************
 * A product of
 * Aclara Technologies LLC
 * Confidential and Proprietary
 * Copyright 2022 Aclara.  All Rights Reserved.
 *
 * PROPRIETARY NOTICE
 * The information contained in this document is private to Aclara Technologies LLC an Ohio limited liability company
 * (Aclara).  This information may not be published, reproduced, or otherwise disseminated without the express written
 * authorization of Aclara.  Any software or firmware described in this document is furnished under a license and may be
 * used or copied only in accordance with the terms of such license.
 *****************************************************************************************************************/

#ifndef features_H
#define features_H

/* ****************************************************************************************************************** */
/* INCLUDE FILES */
//#include "meter.h" /* TODO: RA6: Deprecated include */

/* ****************************************************************************************************************** */
/* MACRO DEFINITIONS */

#ifdef __BOOTLOADER
#undef __BOOTLOADER
#endif

#define HMC_I210_PLUS_C                   1  /* Target host meter is GE i210+c */
#define MAX_COIN_PER_DEMAND               2  /* Maximum number of coincident values per demand */
#define TOU_TIER_E_SUPPORT                0  /* Only Tiers A-D supported   */
#define DAILY_TOU_TIER_C_D_SUPPORT        0  /* Only Tiers A-B supported   */
#define ED_PROGRAMMER_NAME_LENTGH         10 /* Aclara Meters Manufacturing table field */

#define COMMERCIAL_METER                  2  /* Not a commercial meter, but can have more than one phase voltage */
#define CLOCK_IN_METER                    0  // TODO: RA6E1 Bob: this should probably be changed to 1 but we need to verify what happens with an I-210+c demand-only meter (e.g., no TOU)
#define LP_IN_METER                       1
#define DEMAND_IN_METER                   1
#define COINCIDENT_SUPPORT                0

#if ( LP_IN_METER == 0 )
#define ID_MAX_CHANNELS                   (6)
#else
#define ID_MAX_CHANNELS                   (8)
#endif

#define REMOTE_DISCONNECT                 0
#define HMC_SNAPSHOT                      0
#define ANSI_STANDARD_TABLES              1
#define ANSI_LOGON                        1
#define ANSI_SECURITY                     1
#define METER_TROUBLE_SIGNAL              1
#define END_DEVICE_NEGOTIATION            0
#define LOG_IN_METER                      1
#define OPTICAL_PASS_THROUGH              0
#define SAMPLE_METER_TEMPERATURE          0  /* Is sampling the meter's thermometer required, some meter's do this on their own */
#define LOAD_CONTROL_DEVICE               0  /* 1 -> supports load control and signed multicast messages.   */
#define END_DEVICE_PROGRAMMING_CONFIG     1  /* Supports configuration of the meter by programming/writing tables */
#define END_DEVICE_PROGRAMMING_FLASH      ED_PROG_FLASH_PATCH_ONLY  /* Supports programming of the meter Flash */
#define END_DEVICE_PROGRAMMING_DISPLAY    0  /* Supports the ability to write to the meter display */
#define VSWR_MEASUREMENT                  0  /* Supports VSWR measurements of antenna */
#define PHASE_DETECTION                   0  /* Supports Phase Detection */
#define LAST_GASP_SIMULATION              0  /* Supports Last Gasp Simulation Feature */
#define MAC_LINK_PARAMETERS               0  /* Supports Decoding MAC Link Parameters Messages */
#define MAC_CMD_RESP_TIME_DIVERSITY       0  /* Not an EP feature */
#define TX_THROTTLE_CONTROL               0  /* 0=No Tx Throttling, 1=Tx Throttling */
#define BM_USE_KERNEL_AWARE_DEBUGGING     1  // TODO: RA6E1 Bob: remove this before release
#define NOISEBAND_LOWEST_CAP_VOLTAGE      1  // TODO: RA6E1 Bob: remove this before release
#define GET_TEMPERATURE_FROM_RADIO        1  

/* TODO: RA6: Move the following to Appropriate module */
#define BARE_METAL                        0
#ifdef __mqx_h__
   #define MQX_RTOS                       1
#endif
#define FREE_RTOS                         2

#define INCLUDE_ECC                       0 /* TODO: RA6E1: Temporary conditional*/
#define RTOS_SELECTION                    FREE_RTOS /* 0 = Bare Metal, 1 = MQX , 2 = FreeRTOS */
#define DBG_TESTS                         1
#define SEGGER_RTT                        0
// TODO: RA6 [name_Balaji]: Move to Appropriate location
#define NXP_K24                           1
#define RA6E1                             2
#define MCU_SELECTED                      RA6E1
// TODO: RA6 [name_Balaji]: Move to appropriate place
#define DAC_CODE_CONFIG                   1  /* Supports DAC0 for TX Power Control */
#define LAST_GASP_RECONFIGURE_CLK         0  /* If enabled, the System Clock will be re-configured to use MOCO instead of Main Clock */
#endif

