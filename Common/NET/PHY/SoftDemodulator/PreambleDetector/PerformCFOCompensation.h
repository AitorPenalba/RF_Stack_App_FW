/******************************************************************************

   Filename: PerformCFOCompensation.h

   Global Designator:

   Contents:

 ******************************************************************************
   A product of
   Aclara Technologies LLC
   Confidential and Proprietary
   Copyright 2012-2019 Aclara.  All Rights Reserved.

   PROPRIETARY NOTICE
   The information contained in this document is private to Aclara Technologies LLC an Ohio limited liability company
   (Aclara).  This information may not be published, reproduced, or otherwise disseminated without the express written
   authorization of Aclara.  Any software or firmware described in this document is furnished under a license and may
   be used or copied only in accordance with the terms of such license.
 *****************************************************************************/

#ifndef PERFORMCFOCOMPENSATION_H
#define PERFORMCFOCOMPENSATION_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include "rtwtypes.h"

/* Function Declarations */
extern void PerformCFOCompensation(float b_Preamble_Buffer[1400],
                                   uint32_t samplesInPreambleBuffer,
                                   float estCFO,
                                   float *c_Last_Phase,
                                   float *c_Last_PhaseRamp);

#endif