/******************************************************************************

   Filename: meter.h

   Contents: Defines the host meter target for the project

 ******************************************************************************
   A product of
   Aclara Technologies LLC
   Confidential and Proprietary
   Copyright 2016-2020 Aclara.  All Rights Reserved.

   PROPRIETARY NOTICE
   The information contained in this document is private to Aclara Technologies LLC an Ohio limited liability company
   (Aclara).  This information may not be published, reproduced, or otherwise disseminated without the express written
   authorization of Aclara.  Any software or firmware described in this document is furnished under a license and may be
   used or copied only in accordance with the terms of such license.
 ******************************************************************************

   $Log$ kad Created 20160324

 *****************************************************************************/

#ifndef meter_H
#define meter_H

/* ****************************************************************************************************************** */
/* INCLUDE FILES */

/* ****************************************************************************************************************** */
/* MACRO DEFINITIONS */
#ifdef __BOOTLOADER
#undef __BOOTLOADER
#endif

#define ACLARA_LC                         1  /* Target is Aclara LC */
#define MAX_COIN_PER_DEMAND               0  /* Maximum number of coincident values per demand */
#define TOU_TIER_E_SUPPORT                0  /* Only Tiers A-D supported   */
#define DAILY_TOU_TIER_C_D_SUPPORT        0  /* Only Tiers A-B supported   */

#define COMMERCIAL_METER                  0
#define CLOCK_IN_METER                    0
#define LP_IN_METER                       0
#define DEMAND_IN_METER                   0
#define ID_MAX_CHANNELS                   0
#define REMOTE_DISCONNECT                 0
#define HMC_SNAPSHOT                      0
#define ANSI_STANDARD_TABLES              0
#define ANSI_LOGON                        0
#define ANSI_SECURITY                     0
#define METER_TROUBLE_SIGNAL              0
#define END_DEVICE_NEGOTIATION            0
#define LOG_IN_METER                      0
#define OPTICAL_PASS_THROUGH              0
#define SAMPLE_METER_TEMPERATURE          0  /* Is sampling the meter's thermometer required, some meter's do this on their own */
#define LOAD_CONTROL_DEVICE               1  /* 1 -> supports load control and signed multicast messages.   */
#define END_DEVICE_PROGRAMMING_CONFIG     0  /* Supports configuration of the meter by programming/writing tables */
#define END_DEVICE_PROGRAMMING_FLASH      ED_PROG_FLASH_NOT_SUPPORTED  /* Supports programming of the meter Flash */
#define END_DEVICE_PROGRAMMING_DISPLAY    0  /* Supports the ability to write to the meter display */
#define VSWR_MEASUREMENT                  0  /* Supports VSWR measurements of antenna  */
#define PHASE_DETECTION                   0  /* Supports Phase Detection */
#define LAST_GASP_SIMULATION              0  /* Supports Last Gasp Simulation Feature */
#define MAC_LINK_PARAMETERS               0  /* Supports Decoding MAC Link Parameters Messages */
#define MAC_CMD_RESP_TIME_DIVERSITY       0  /* Not an EP feature */
#define TX_THROTTLE_CONTROL               0  /* 0=No Tx Throttling, 1=Tx Throttling */
#endif
